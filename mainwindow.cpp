#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "billon.h"
#include "dicomreader.h"
#include "slicehistogram.h"

#include <iostream>

#include <QFileDialog>
#include <QMouseEvent>

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

	_groupSliceView.addButton(ui->_originalSlice,CURRENT_SLICE);
	_groupSliceView.addButton(ui->_averageSlice,AVERAGE_SLICE);
	_groupSliceView.addButton(ui->_medianSlice,MEDIAN_SLICE);
	_groupSliceView.setExclusive(true);
	ui->_originalSlice->setChecked(true);

	// Raccourcis des actions du menu
	ui->_actionOpenDicom->setShortcut(Qt::CTRL + Qt::Key_O);
	ui->_actionCloseImage->setShortcut(Qt::CTRL + Qt::Key_W);
	ui->_actionQuit->setShortcut(Qt::CTRL + Qt::Key_Q);

	/**** Mise en place de la communication MVC ****/

	// Évènements déclenchés par le slider
	QObject::connect(ui->_imageSlider, SIGNAL(valueChanged(int)), _sliceView, SLOT(drawSlice(int)));
	QObject::connect(ui->_imageSlider, SIGNAL(valueChanged(int)), ui->_sliceNumber, SLOT(setNum(int)));

	// Évènements déclenchés par les boutons de sélection de la vue
	QObject::connect(&_groupSliceView, SIGNAL(buttonClicked(int)), _sliceView, SLOT(setTypeOfView(int)));

	// Évènements déclenchés par le seuillage
	QObject::connect(ui->_lowThreshold, SIGNAL(valueChanged(int)), _sliceView, SLOT(setLowThreshold(int)));
	QObject::connect(ui->_highThreshold, SIGNAL(valueChanged(int)), _sliceView, SLOT(setHighThreshold(int)));

	// Évènements reçus de la vue en coupe
	QObject::connect(_sliceView, SIGNAL(updated(QPixmap)), ui->_labelImage, SLOT(setPixmap(QPixmap)));
	QObject::connect(_sliceView, SIGNAL(typeOfViewChanged(SliceType)), this, SLOT(adaptToSliceType(SliceType)));

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

void MainWindow::adaptToSliceType(const SliceType &type) {
	switch (type) {
		case AVERAGE_SLICE :
		case MEDIAN_SLICE :
			ui->_imageSlider->setEnabled(false);
			ui->_highThreshold->setEnabled(false);
			ui->_lowThreshold->setEnabled(false);
			break;
		case CURRENT_SLICE :
		default :
			ui->_imageSlider->setEnabled(true);
			ui->_highThreshold->setEnabled(true);
			ui->_lowThreshold->setEnabled(true);
	}
}

void MainWindow::updateBillon() {
	if ( _billon != 0 ) {
		ui->_lowThreshold->setMinimum(_billon->minValue());
		ui->_lowThreshold->setMaximum(_billon->maxValue());
		std::cout << "ui->_lowThreshold->setValue(_billon->minValue());" << std::endl;
		ui->_lowThreshold->setValue(_billon->minValue());
		ui->_highThreshold->setMinimum(_billon->minValue());
		ui->_highThreshold->setMaximum(_billon->maxValue());
		std::cout << "ui->_maxThreshold->setValue(_billon->maxValue());" << std::endl;
		ui->_highThreshold->setValue(_billon->maxValue());
	}

	std::cout << "ui->_imageSlider->setValue(0)" << std::endl;
	ui->_imageSlider->setValue(0);
	ui->_imageSlider->setRange(0,_billon!=0?_billon->n_slices-1:0);

	if ( _billon != 0 )	ui->_sliceNumber->setNum(0);
	else ui->_sliceNumber->setText("Aucune coupe présente.");

	_sliceView->setModel(_billon);
	_sliceHistogram->setModel(_billon);
	_sliceView->drawSlice();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
	if (obj == ui->_labelImage) {
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
