#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "billon.h"
#include "dicomreader.h"
#include "slicehistogram.h"
#include "sliceview.h"

#include <iostream>

#include <QPicture>
#include <QPainter>
#include <QLabel>
#include <QPainter>
#include <QLineEdit>
#include <QGridLayout>
#include <QFileDialog>
#include <QMouseEvent>

#define ORIGINAL_SLICE_VIEW_ID 0
#define AVERAGE_SLICE_VIEW_ID 1
#define MEDIAN_SLICE_VIEW_ID 2


MainWindow::MainWindow( QWidget *parent ) : QMainWindow(parent), ui(new Ui::MainWindow), _billon(0), _sliceView(0), _sliceHistogram(0), _currentSlice(0) {
	ui->setupUi(this);

	// Initialisation des vues
	_sliceHistogram = new SliceHistogram(ui->_histogramContainer);
	_sliceView = new SliceView();

	// Paramétrisation des composant graphiques
	ui->_labelImage->installEventFilter(this);

	ui->_imageSlider->setSingleStep(1);
	ui->_imageSlider->setPageStep(10);
	ui->_imageSlider->setTickInterval(1);
	ui->_imageSlider->setTickPosition(QSlider::TicksBelow);

	_groupSliceView.addButton(ui->_originalSlice,ORIGINAL_SLICE_VIEW_ID);
	_groupSliceView.addButton(ui->_averageSlice,AVERAGE_SLICE_VIEW_ID);
	_groupSliceView.addButton(ui->_medianSlice,MEDIAN_SLICE_VIEW_ID);
	_groupSliceView.setExclusive(true);
	ui->_originalSlice->setChecked(true);

	// Raccourcis des actions du menu
	ui->_actionOpenDicom->setShortcut(Qt::CTRL + Qt::Key_O);
	ui->_actionCloseImage->setShortcut(Qt::CTRL + Qt::Key_W);
	ui->_actionQuit->setShortcut(Qt::CTRL + Qt::Key_Q);

	/**** Mise en place de la communication MVC ****/

	// Évènements déclenchés par le slider
	QObject::connect(ui->_imageSlider, SIGNAL(valueChanged(int)), this, SLOT(drawSlice(int)));
	QObject::connect(ui->_imageSlider, SIGNAL(valueChanged(int)), ui->_sliceNumber, SLOT(setNum(int)));

	// Évènements déclenchés par les boutons de sélection de la vue
	QObject::connect(&_groupSliceView, SIGNAL(buttonClicked(int)), this, SLOT(drawSliceType(int)));

	// Évènements déclenchés par le seuillage
	QObject::connect(ui->_lowThreshold, SIGNAL(valueChanged(int)), _sliceView, SLOT(setLowThreshold(int)));
	QObject::connect(ui->_highThreshold, SIGNAL(valueChanged(int)), _sliceView, SLOT(setHighThreshold(int)));

	// Évènements reçus de la vue en coupe
	QObject::connect(_sliceView, SIGNAL(updated(QPixmap)), ui->_labelImage, SLOT(setPixmap(QPixmap)));
	QObject::connect(_sliceView, SIGNAL(thresholdUpdated()), this, SLOT(redrawSlice()));

	// Évènements déclenchés par les actions du menu
	QObject::connect(ui->_actionOpenDicom, SIGNAL(triggered()), this, SLOT(openDicom()));
	QObject::connect(ui->_actionCloseImage, SIGNAL(triggered()), this, SLOT(closeImage()));
	QObject::connect(ui->_actionQuit, SIGNAL(triggered()), this, SLOT(close()));
}

MainWindow::~MainWindow() {
	if ( _sliceHistogram != 0 ) delete _sliceHistogram;
	if ( _sliceView != 0 ) delete _sliceView;
	if ( _billon != 0 ) delete _billon;
	if ( ui != 0 ) delete ui;
}

void MainWindow::openDicom() {
	QString folderName = QFileDialog::getExistingDirectory(0,"Sélection du répertoire DICOM",QDir::homePath(),QFileDialog::ShowDirsOnly);
	if ( !folderName.isEmpty() ) {
		// Lecture des fichiers DICOM
		if ( _billon != 0 ) {
			delete _billon;
			_billon = 0;
		}
		_billon = DicomReader::read(folderName);

		updateBillon();
	}
}

void MainWindow::closeImage() {
	if ( _billon != 0 ) {
		delete _billon;
		_billon = 0;
	}

	updateBillon();
}

void MainWindow::updateBillon() {
	_sliceView->setModel(_billon);
	_sliceHistogram->setModel(_billon);
	_currentSlice = 0;

	ui->_imageSlider->setRange(0,_billon!=0?_billon->n_slices-1:0);
	ui->_imageSlider->setSliderPosition(0);

	if ( _billon != 0 )	ui->_sliceNumber->setNum(0);
	else ui->_sliceNumber->setText("Aucune coupe présente.");

	if ( _billon != 0 ) {
		ui->_lowThreshold->setMinimum(_billon->minValue());
		ui->_lowThreshold->setMaximum(_billon->maxValue());
		ui->_lowThreshold->setValue(_billon->minValue());
		ui->_highThreshold->setMinimum(_billon->minValue());
		ui->_highThreshold->setMaximum(_billon->maxValue());
		ui->_highThreshold->setValue(_billon->maxValue());
	}

	drawSlice(_currentSlice);
}

void MainWindow::redrawSlice() {
	_sliceView->drawSlice(_currentSlice);
}

void MainWindow::drawSlice( const int &imageIndex ) {
	_currentSlice = imageIndex;
	_sliceView->drawSlice(imageIndex);
}

void MainWindow::drawSliceType( const int &buttonId ) {
	switch (buttonId) {
		case AVERAGE_SLICE_VIEW_ID :
			ui->_imageSlider->setEnabled(false);
			ui->_lowThreshold->setEnabled(false);
			ui->_highThreshold->setEnabled(false);
			_sliceView->drawAverageSlice();
			break;
		case MEDIAN_SLICE_VIEW_ID :
			ui->_imageSlider->setEnabled(false);
			ui->_lowThreshold->setEnabled(false);
			ui->_highThreshold->setEnabled(false);
			_sliceView->drawMedianSlice();
			break;
		default :
			ui->_imageSlider->setEnabled(true);
			ui->_lowThreshold->setEnabled(true);
			ui->_highThreshold->setEnabled(true);
			_sliceView->drawSlice(_currentSlice);
	}
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
	if (obj == ui->_labelImage) {
		if ( _billon != 0 && event->type() == QEvent::MouseButtonPress ) {
			QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
			std::cout << obj->objectName().toStdString() << " : ( " << mouseEvent->x() << ", " << mouseEvent->y() << ", " << _currentSlice << " )" << std::endl;
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
