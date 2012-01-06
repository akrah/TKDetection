#include "inc/mainwindow.h"

#include "ui_mainwindow.h"
#include "inc/billon.h"
#include "inc/sliceview.h"
#include "inc/dicomreader.h"
#include "inc/slicehistogram.h"
#include "inc/marrowextractor.h"

#include <iostream>

#include <QFileDialog>
#include <QMouseEvent>
#include <QPainter>


MainWindow::MainWindow( QWidget *parent ) : QMainWindow(parent), _ui(new Ui::MainWindow), _billon(0), _sliceView(0), _sliceHistogram(0), _marrow(0) {
	_ui->setupUi(this);

	// Initialisation des vues
	_sliceView = new SliceView();
	_sliceHistogram = new SliceHistogram(_ui->_plotSliceHistogram);

	// Paramétrisation des composant graphiques
	_ui->_labelSliceView->installEventFilter(this);

	_ui->_sliderSelectSlice->setSingleStep(1);
	_ui->_sliderSelectSlice->setPageStep(10);
	_ui->_sliderSelectSlice->setTickInterval(1);
	_ui->_sliderSelectSlice->setTickPosition(QSlider::TicksAbove);

	_ui->_plotSliceHistogram->enableAxis(QwtPlot::yLeft,false);

	_histogramCursor.attach(_ui->_plotSliceHistogram);
	_histogramCursor.setBrush(Qt::red);
	_histogramCursor.setPen(QPen(Qt::red));

	_groupSliceView.addButton(_ui->_radioOriginalSlice,SliceType::CURRENT);
	_groupSliceView.addButton(_ui->_radioAverageSlice,SliceType::AVERAGE);
	_groupSliceView.addButton(_ui->_radioMedianSlice,SliceType::MEDIAN);
	_groupSliceView.setExclusive(true);

	// Raccourcis des actions du menu
	_ui->_actionOpenDicom->setShortcut(Qt::CTRL + Qt::Key_O);
	_ui->_actionCloseImage->setShortcut(Qt::CTRL + Qt::Key_W);
	_ui->_actionQuit->setShortcut(Qt::CTRL + Qt::Key_Q);

	/**** Mise en place de la communication MVC ****/

	// Évènements déclenchés par le slider de n° de coupe
	QObject::connect(_ui->_sliderSelectSlice, SIGNAL(valueChanged(int)), _sliceView, SLOT(drawSlice(int)));
	QObject::connect(_ui->_sliderSelectSlice, SIGNAL(valueChanged(int)), _ui->_labelSliceNumber, SLOT(setNum(int)));
	QObject::connect(_ui->_sliderSelectSlice, SIGNAL(valueChanged(int)), this, SLOT(highlightHistogramSlice(int)));

	// Évènements déclenchés par les boutons de sélection de la vue
	QObject::connect(&_groupSliceView, SIGNAL(buttonClicked(int)), _sliceView, SLOT(setTypeOfView(int)));

	// Évènements déclenchés par le slider de seuillage
	QObject::connect(_ui->_spansliderSliceThreshold, SIGNAL(lowerPositionChanged(int)), _sliceView, SLOT(setLowThreshold(int)));
	QObject::connect(_ui->_spansliderSliceThreshold, SIGNAL(lowerPositionChanged(int)), _sliceHistogram, SLOT(setLowThreshold(int)));
	QObject::connect(_ui->_spansliderSliceThreshold, SIGNAL(lowerValueChanged(int)), _ui->_labelMinThreshold, SLOT(setNum(int)));
	QObject::connect(_ui->_spansliderSliceThreshold, SIGNAL(upperPositionChanged(int)), _sliceView, SLOT(setHighThreshold(int)));
	QObject::connect(_ui->_spansliderSliceThreshold, SIGNAL(upperPositionChanged(int)), _sliceHistogram, SLOT(setHighThreshold(int)));
	QObject::connect(_ui->_spansliderSliceThreshold, SIGNAL(upperValueChanged(int)), _ui->_labelMaxThreshold, SLOT(setNum(int)));

	// Évènements déclenchés par le bouton de mise à jour de l'histogramme
	QObject::connect(_ui->_buttonUpdateHistogram, SIGNAL(clicked()), _sliceHistogram, SLOT(constructHistogram()));

	// Évènements déclenchés par les bouton associès à la moelle
	QObject::connect(_ui->_buttonComputeMarrow, SIGNAL(clicked()), this, SLOT(computeNewMarrow()));
	QObject::connect(_ui->_checkDrawMarrow, SIGNAL(toggled(bool)), _sliceView, SLOT(drawMarrow(bool)));

	// Évènements reçus de la vue en coupe
	QObject::connect(_sliceView, SIGNAL(updated(QPixmap)), _ui->_labelSliceView, SLOT(setPixmap(QPixmap)));
	QObject::connect(_sliceView, SIGNAL(typeOfViewChanged(SliceType::SliceType)), this, SLOT(adaptGraphicsComponentsToSliceType(SliceType::SliceType)));

	// Évènements reçus de la vue histogramme
	QObject::connect(_sliceHistogram, SIGNAL(histogramUpdated()), this, SLOT(redrawHistogram()));

	// Évènements déclenchés par les actions du menu
	QObject::connect(_ui->_actionOpenDicom, SIGNAL(triggered()), this, SLOT(openDicom()));
	QObject::connect(_ui->_actionCloseImage, SIGNAL(triggered()), this, SLOT(closeImage()));
	QObject::connect(_ui->_actionQuit, SIGNAL(triggered()), this, SLOT(close()));

	_ui->_radioOriginalSlice->click();
	//updateGraphicsComponentsValues();
}

MainWindow::~MainWindow() {
	if ( _marrow != 0 ) delete _marrow;
	if ( _sliceHistogram != 0 ) delete _sliceHistogram;
	if ( _sliceView != 0 ) delete _sliceView;
	if ( _billon != 0 ) delete _billon;
	if ( _ui != 0 ) delete _ui;
}

/****************************************
 * Public
 ****************************************/

#include "inc/piechart.h"

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
	if (obj == _ui->_labelSliceView) {
		if ( _billon != 0 && event->type() == QEvent::MouseButtonPress ) {
			QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
			const int x = mouseEvent->x();
			const int y = mouseEvent->y();
			std::cout << obj->objectName().toStdString() << " : ( " << x << ", " << y << ", " << _sliceView->currentSliceIndex() << " )" << std::endl;

			QPixmap pix(*(_ui->_labelSliceView->pixmap()));
			const int b1 = y-x;
			const int b2 = y+x;
			const int width = pix.width();
			const int height = pix.height();

			PiePart part;
			part._orientation = 3.*PI/8.;
			part._angle = PI/4.;
			const double a_l = - part.leftFactor();
			const double b_l = y - (a_l*x);
			const double a_r = - part.rightFactor();
			const double b_r = y - (a_r*x);

			QPainter painter(&pix);
//				painter.drawEllipse(x-10,y-10,20,20);
//				painter.drawLine(x,0,x,height);
//				painter.drawLine(0,y,width,y);
//				painter.drawLine(0,b1,width,width+b1);
//				painter.drawLine(0,b2,width,-width+b2);
				painter.drawLine(0,b_l,width,a_l*width+b_l);
				painter.drawLine(0,b_r,width,a_r*width+b_r);

				std::cout << " y = a_l * x + b_l ==> " << y << " = " << a_l << " * " << x << " + " << b_l << " = " << a_l*x+b_l << std::endl;
				std::cout << " y = a_r * x + b_r ==> " << y << " = " << a_r << " * " << x << " + " << b_r << " = " << a_r*x+b_r << std::endl;

			_ui->_labelSliceView->setPixmap(pix);

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

/****************************************
 * Private slots
 ****************************************/

void MainWindow::openDicom() {
	QString folderName = QFileDialog::getExistingDirectory(0,tr("Sélection du répertoire DICOM"),QDir::homePath(),QFileDialog::ShowDirsOnly);
	if ( !folderName.isEmpty() ) {
		// Lecture des fichiers DICOM
		openNewBillon(folderName);
		updateGraphicsComponentsValues();
	}
}

void MainWindow::closeImage() {
	openNewBillon();
	computeNewMarrow();
	updateGraphicsComponentsValues();
}

void MainWindow::adaptGraphicsComponentsToSliceType(const SliceType::SliceType &type) {
	switch (type) {
		case SliceType::AVERAGE :
		case SliceType::MEDIAN :
			_ui->_sliderSelectSlice->setEnabled(false);
			_ui->_spansliderSliceThreshold->setEnabled(false);
			_ui->_buttonComputeMarrow->setEnabled(false);
			_ui->_buttonUpdateHistogram->setEnabled(false);
			_ui->_checkDrawMarrow->setEnabled(false);
			break;
		case SliceType::CURRENT :
		default :
			const bool existBillon = _billon != 0;
			_ui->_sliderSelectSlice->setEnabled(existBillon);
			_ui->_spansliderSliceThreshold->setEnabled(existBillon);
			_ui->_buttonComputeMarrow->setEnabled(existBillon);
			_ui->_buttonUpdateHistogram->setEnabled(existBillon);
			_ui->_checkDrawMarrow->setEnabled(existBillon);
	}
}

void MainWindow::computeNewMarrow() {
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

void MainWindow::redrawHistogram() {
	_ui->_plotSliceHistogram->setAxisScale(QwtPlot::xBottom,0,(_billon != 0)?_billon->n_slices:0);
	_ui->_plotSliceHistogram->replot();
	highlightHistogramSlice(_ui->_sliderSelectSlice->value());
}


void MainWindow::highlightHistogramSlice( const int &slicePosition ) {
	double height = _sliceHistogram->sample(slicePosition).value;
	double x[4] = {slicePosition,slicePosition, slicePosition+1,slicePosition+1};
	double y[4] = {0,height,height,0};
	_histogramCursor.setSamples(x,y,4);
	_ui->_plotSliceHistogram->replot();
}

/****************************************
 * Private
 ****************************************/

void MainWindow::updateGraphicsComponentsValues() {
	uint minValue, maxValue, nbSlices;
	if ( _billon != 0 ) {
		minValue = _billon->minValue();
		maxValue = _billon->maxValue();
		nbSlices = _billon->n_slices;
		_ui->_buttonComputeMarrow->setEnabled(true);
	}
	else {
		_ui->_buttonComputeMarrow->setEnabled(false);
		minValue = maxValue = 0;
		nbSlices = 1;
	}

	_ui->_spansliderSliceThreshold->setMinimum(minValue);
	_ui->_spansliderSliceThreshold->setLowerValue(minValue);
	_ui->_spansliderSliceThreshold->setMaximum(maxValue);
	_ui->_spansliderSliceThreshold->setUpperValue(maxValue);

	_ui->_sliderSelectSlice->setValue(0);
	_ui->_sliderSelectSlice->setRange(0,nbSlices-1);

	if ( _billon != 0 )	_ui->_labelSliceNumber->setNum(0);
	else _ui->_labelSliceNumber->setText(tr("Aucune coupe présente."));

	adaptGraphicsComponentsToSliceType(_sliceView->sliceType());
}

void MainWindow::openNewBillon( const QString &folderName ) {
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
