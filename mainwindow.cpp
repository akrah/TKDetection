#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "billon.h"
#include "sliceview.h"
#include "dicomreader.h"
#include "slicehistogram.h"
#include "marrowextractor.h"

#include <iostream>

#include <QFileDialog>
#include <QMouseEvent>

MainWindow::MainWindow( QWidget *parent ) : QMainWindow(parent), ui(new Ui::MainWindow), _billon(0), _sliceView(0), _sliceHistogram(0), _marrow(0), _currentSlice(0) {
	ui->setupUi(this);

	// Initialisation des vues
	_sliceView = new SliceView();
	_sliceHistogram = new SliceHistogram(ui->_plotSliceHistogram);

	// Paramétrisation des composant graphiques
	ui->_labelSliceView->installEventFilter(this);

	ui->_sliderSelectSlice->setSingleStep(1);
	ui->_sliderSelectSlice->setPageStep(10);
	ui->_sliderSelectSlice->setTickInterval(1);
	ui->_sliderSelectSlice->setTickPosition(QSlider::TicksAbove);

	ui->_plotSliceHistogram->enableAxis(QwtPlot::yLeft,false);

	_groupSliceView.addButton(ui->_radioOriginalSlice,SliceType::CURRENT);
	_groupSliceView.addButton(ui->_radioAverageSlice,SliceType::AVERAGE);
	_groupSliceView.addButton(ui->_radioMedianSlice,SliceType::MEDIAN);
	_groupSliceView.setExclusive(true);
	ui->_radioOriginalSlice->setChecked(true);

	// Raccourcis des actions du menu
	ui->_actionOpenDicom->setShortcut(Qt::CTRL + Qt::Key_O);
	ui->_actionCloseImage->setShortcut(Qt::CTRL + Qt::Key_W);
	ui->_actionQuit->setShortcut(Qt::CTRL + Qt::Key_Q);

	/**** Mise en place de la communication MVC ****/

	// Évènements déclenchés par le slider de n° de coupe
	QObject::connect(ui->_sliderSelectSlice, SIGNAL(valueChanged(int)), _sliceView, SLOT(drawSlice(int)));
	QObject::connect(ui->_sliderSelectSlice, SIGNAL(valueChanged(int)), ui->_labelSliceNumber, SLOT(setNum(int)));

	// Évènements déclenchés par les boutons de sélection de la vue
	QObject::connect(&_groupSliceView, SIGNAL(buttonClicked(int)), _sliceView, SLOT(setTypeOfView(int)));

	// Évènements déclenchés par le slider de seuillage
	QObject::connect(ui->_spansliderSliceThreshold, SIGNAL(lowerPositionChanged(int)), _sliceView, SLOT(setLowThreshold(int)));
	QObject::connect(ui->_spansliderSliceThreshold, SIGNAL(lowerPositionChanged(int)), _sliceHistogram, SLOT(setLowThreshold(int)));
	QObject::connect(ui->_spansliderSliceThreshold, SIGNAL(lowerValueChanged(int)), ui->_labelMinThreshold, SLOT(setNum(int)));
	QObject::connect(ui->_spansliderSliceThreshold, SIGNAL(upperPositionChanged(int)), _sliceView, SLOT(setHighThreshold(int)));
	QObject::connect(ui->_spansliderSliceThreshold, SIGNAL(upperPositionChanged(int)), _sliceHistogram, SLOT(setHighThreshold(int)));
	QObject::connect(ui->_spansliderSliceThreshold, SIGNAL(upperValueChanged(int)), ui->_labelMaxThreshold, SLOT(setNum(int)));

	// Évènements déclenchés par le bouton de mise à jour de l'histogramme
	QObject::connect(ui->_buttonUpdateHistogram, SIGNAL(clicked()), _sliceHistogram, SLOT(constructHistogram()));

	// Évènements déclenchés par le bouton de calcul de la moelle
	QObject::connect(ui->_buttonComputeMarrow, SIGNAL(clicked()), this, SLOT(newMarrow()));
	QObject::connect(ui->_checkDrawMarrow, SIGNAL(toggled(bool)), _sliceView, SLOT(drawMarrow(bool)));

	// Évènements reçus de la vue en coupe
	QObject::connect(_sliceView, SIGNAL(updated(QPixmap)), ui->_labelSliceView, SLOT(setPixmap(QPixmap)));
	QObject::connect(_sliceView, SIGNAL(typeOfViewChanged(SliceType::SliceType)), this, SLOT(adaptToSliceType(SliceType::SliceType)));

	// Évènements reçus de la vue histogramme
	QObject::connect(_sliceHistogram, SIGNAL(histogramUpdated()), this, SLOT(updateHistogram()));

	// Évènements déclenchés par les actions du menu
	QObject::connect(ui->_actionOpenDicom, SIGNAL(triggered()), this, SLOT(openDicom()));
	QObject::connect(ui->_actionCloseImage, SIGNAL(triggered()), this, SLOT(closeImage()));
	QObject::connect(ui->_actionQuit, SIGNAL(triggered()), this, SLOT(close()));
}

MainWindow::~MainWindow() {
	if ( _marrow != 0 ) delete _marrow;
	if ( _sliceHistogram != 0 ) delete _sliceHistogram;
	if ( _sliceView != 0 ) delete _sliceView;
	if ( _billon != 0 ) delete _billon;
	if ( ui != 0 ) delete ui;
}

void MainWindow::openDicom() {
	QString folderName = QFileDialog::getExistingDirectory(0,tr("Sélection du répertoire DICOM"),QDir::homePath(),QFileDialog::ShowDirsOnly);
	if ( !folderName.isEmpty() ) {
		// Lecture des fichiers DICOM
		newBillon(folderName);
		updateComponentsState();
	}
}

void MainWindow::closeImage() {
	newBillon();
	newMarrow();
	updateComponentsState();
}

void MainWindow::adaptToSliceType(const SliceType::SliceType &type) {
	switch (type) {
		case SliceType::AVERAGE :
		case SliceType::MEDIAN :
			ui->_sliderSelectSlice->setEnabled(false);
			ui->_spansliderSliceThreshold->setEnabled(false);
			ui->_buttonComputeMarrow->setEnabled(false);
			ui->_buttonUpdateHistogram->setEnabled(false);
			ui->_checkDrawMarrow->setEnabled(false);
			break;
		case SliceType::CURRENT :
		default :
			ui->_sliderSelectSlice->setEnabled(true);
			ui->_spansliderSliceThreshold->setEnabled(true);
			ui->_buttonComputeMarrow->setEnabled(true);
			ui->_buttonUpdateHistogram->setEnabled(true);
			ui->_checkDrawMarrow->setEnabled(true);
	}
}

void MainWindow::newMarrow() {
	if ( _marrow != 0 ) {
		delete _marrow;
		_marrow = 0;
	}
	if ( _billon != 0 ) {
		MarrowExtractor extractor;
		_marrow = extractor.process(*_billon,0,_billon->n_slices-1);
	}
	_sliceView->setModel(_marrow);
}

void MainWindow::updateHistogram() {
	ui->_plotSliceHistogram->setAxisScale(QwtPlot::xBottom,0,(_billon != 0)?_billon->n_slices:0);
	ui->_plotSliceHistogram->replot();
}

void MainWindow::updateComponentsState() {
	uint minValue, maxValue, nbSlices;
	if ( _billon != 0 ) {
		minValue = _billon->minValue();
		maxValue = _billon->maxValue();
		nbSlices = _billon->n_slices;
	}
	else {
		minValue = maxValue = 0;
		nbSlices = 1;
	}

	ui->_spansliderSliceThreshold->setMinimum(minValue);
	ui->_spansliderSliceThreshold->setLowerValue(minValue);
	ui->_spansliderSliceThreshold->setMaximum(maxValue);
	ui->_spansliderSliceThreshold->setUpperValue(maxValue);

	ui->_sliderSelectSlice->setValue(0);
	ui->_sliderSelectSlice->setRange(0,nbSlices-1);

	if ( _billon != 0 )	ui->_labelSliceNumber->setNum(0);
	else ui->_labelSliceNumber->setText(tr("Aucune coupe présente."));
}

void MainWindow::newBillon( const QString &folderName ) {
	if ( _billon != 0 ) {
		delete _billon;
		_billon = 0;
	}
	if ( !folderName.isEmpty() ) {
		_billon = DicomReader::read(folderName);
	}
	_sliceView->setModel(_billon);
	_sliceHistogram->setModel(_billon);
}


bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
	if (obj == ui->_labelSliceView) {
		if ( _billon != 0 && event->type() == QEvent::MouseButtonPress ) {
			QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
			std::cout << obj->objectName().toStdString() << " : ( " << mouseEvent->x() << ", " << mouseEvent->y() << ", " << _sliceView->currentSlice() << " )" << std::endl;
			return true;
		}
		else {
			return false;
		}
	 }
	else {
		 return QMainWindow::eventFilter(obj, event);
	 }
}
