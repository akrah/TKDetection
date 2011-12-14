#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "billon.h"
#include "dicomreader.h"
#include "slicehistogram.h"

#include <iostream>

#include <QPicture>
#include <QPainter>
#include <QLabel>
#include <QPainter>
#include <QLineEdit>
#include <QGridLayout>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), _cube(0), _image(0), _currentSlice(0), _sliceHistogram(0) {
	ui->setupUi(this);

	_sliceHistogram = new SliceHistogram(ui->_histogramContainer);

	ui->_imageSlider->setSingleStep(1);
	ui->_imageSlider->setPageStep(10);
	ui->_imageSlider->setTickInterval(1);
	ui->_imageSlider->setTickPosition(QSlider::TicksBelow);

	QObject::connect(ui->_imageSlider, SIGNAL(valueChanged(int)), this, SLOT(setImage(int)));

	ui->_actionOpenDicom->setShortcut(Qt::CTRL + Qt::Key_O);
	QObject::connect(ui->_actionOpenDicom, SIGNAL(triggered()), this, SLOT(openDicom()));

	ui->_actionCloseImage->setShortcut(Qt::CTRL + Qt::Key_W);
	QObject::connect(ui->_actionCloseImage, SIGNAL(triggered()), this, SLOT(closeImage()));

	ui->_actionQuit->setShortcut(Qt::CTRL + Qt::Key_Q);
	QObject::connect(ui->_actionQuit, SIGNAL(triggered()), this, SLOT(close()));

	QObject::connect(ui->_averageSlice, SIGNAL(toggled(bool)), this, SLOT(changeAverageSliceVisibility(bool)));

	// A supprimer pour exploitation
//	openDicom("/home/adrien/These/DICOM_test");
//	openDicom("/home/adrien/These/DICOM/17_Poutres_sapin_pin_epicea/Jeu2/epicea_1_b2/");
//	if ( _cube != 0 && _image != 0 ) {
//		ui->_imageSlider->setRange(0,_cube->n_slices-1);
//		ui->_imageSlider->setSliderPosition(0);
//		enableImageModifiers(true);
//	}
}

MainWindow::~MainWindow() {
	closeImage();
	if (_sliceHistogram != 0) delete _sliceHistogram;
	if (ui != 0) delete ui;
}

void MainWindow::openDicom(const QString &folderName) {
	// Lecture des fichiers DICOM
	if ( _cube != 0 ) {
		delete _cube;
	}
	_cube = DicomReader::read(folderName);

	// Réinitialisation de l'image
	if ( _image != 0 ) {
		delete _image;
	}
	_image = new QImage(_cube->n_cols,_cube->n_rows,QImage::Format_ARGB32);
	_currentSlice = 0;
	updateImage();

	ui->_histogramContainer->setFixedSize(_cube->n_cols,HISTOGRAM_HEIGHT);
	_sliceHistogram->update(*_cube);
}

void MainWindow::drawAverageSlice() {
	if ( _cube != 0 && _image != 0 ) {
		const uint width = _cube->n_cols;
		const uint height = _cube->n_rows;
		const uint depth = _cube->n_slices;
		const float minValue = _cube->minValue();
		const float maxValue = _cube->maxValue();
		const float fact = (255.0/(maxValue-minValue));

		float c;
		QRgb * line;
		for (unsigned int j=0 ; j<height ; j++) {
			line = (QRgb *) _image->scanLine(j);
			for (unsigned int i=0 ; i<width ; i++) {
				c = depth*-minValue;
				for (unsigned int k=0 ; k<depth ; k++) {
					c += _cube->slice(k)(j,i);
				}
				c = c*fact/(float)depth;
				line[i] = qRgb(c,c,c);
			}
		}

		QPixmap pix = QPixmap::fromImage(*_image);
		ui->_labelImage->setPixmap(pix);
	}
	else {
		QPixmap pix;
		ui->_labelImage->setPixmap(pix);
	}
}

void MainWindow::enableImageModifiers(bool enable) {
	ui->_imageSlider->setEnabled(enable);
	ui->_averageSlice->setEnabled(enable);
}

void MainWindow::openDicom() {
	QString folderName = QFileDialog::getExistingDirectory(0,"Sélection du répertoire DICOM",QDir::homePath(),QFileDialog::ShowDirsOnly);
	if ( !folderName.isEmpty() ) {
		openDicom(folderName);
		if ( _cube != 0 && _image != 0 ) {
			ui->_imageSlider->setRange(0,_cube->n_slices-1);
			ui->_imageSlider->setSliderPosition(0);
			enableImageModifiers(true);
		}
	}
}

void MainWindow::closeImage() {
	if ( _image != 0 ) {
		delete _image;
		_image = 0;
	}
	if ( _cube != 0 ) {
		delete _cube;
		_cube = 0;
	}
	updateImage();
	enableImageModifiers(false);
}

void MainWindow::updateImage() {
	if ( _cube != 0 && _image != 0 ) {
		const uint width = _cube->n_cols;
		const uint height = _cube->n_rows;

		const float minValue = _cube->minValue();
		const float maxValue = _cube->maxValue();
		const float fact = (255.0/(maxValue-minValue));

		float c;
		QRgb * line;
		for (unsigned int j=0 ; j<height ; j++) {
			line = (QRgb *) _image->scanLine(j);
			for (unsigned int i=0 ; i<width ; i++) {
				c = _cube->slice(_currentSlice)(j,i);
				c = (c-minValue)*fact;
				line[i] = qRgb(c,c,c);
			}
		}
		QPixmap pix = QPixmap::fromImage(*_image);
		ui->_labelImage->setPixmap(pix);
	}
	else {
		QPixmap pix;
		ui->_labelImage->setPixmap(pix);
	}
}

void MainWindow::setImage(const int imageIndex) {
	if ( imageIndex >= 0 || static_cast<uint>(imageIndex) < _cube->n_slices ) {
		_currentSlice = imageIndex;
		updateImage();
	}
}

void MainWindow::changeAverageSliceVisibility(bool isVisible) {
	ui->_imageSlider->setEnabled(!isVisible);
	if ( isVisible ) {
		drawAverageSlice();
	}
	else {
		updateImage();
	}
}
