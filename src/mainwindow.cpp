#include "inc/mainwindow.h"

#include "ui_mainwindow.h"
#include "inc/billon.h"
#include "inc/global.h"
#include "inc/marrow.h"
#include "inc/sliceview.h"
#include "inc/dicomreader.h"
#include "inc/slicehistogram.h"
#include "inc/marrowextractor.h"
#include "inc/piepart.h"
#include "inc/piechart.h"
#include "inc/piechartdiagrams.h"

#include <QFileDialog>
#include <QMouseEvent>
#include <QPainter>

MainWindow::MainWindow( QWidget *parent ) : QMainWindow(parent), _ui(new Ui::MainWindow), _billon(0), _marrow(0), _sliceView(new SliceView()), _sliceHistogram(0), _pieChart(new PieChart(0,1)), _pieChartDiagrams(new PieChartDiagrams()) {
	_ui->setupUi(this);

	// Initialisation des vues
	_sliceHistogram = new SliceHistogram(_ui->_plotSliceHistogram);
	_pieChartDiagrams->setModel(_pieChart);

	// Paramétrisation des composant graphiques
	_ui->_labelSliceView->installEventFilter(this);

	_ui->_plotSliceHistogram->enableAxis(QwtPlot::yLeft,false);

	_histogramCursor.attach(_ui->_plotSliceHistogram);
	_histogramCursor.setBrush(Qt::red);
	_histogramCursor.setPen(QPen(Qt::red));

	_groupSliceView.addButton(_ui->_radioCurrentSlice,SliceType::CURRENT);
	_groupSliceView.addButton(_ui->_radioAverageSlice,SliceType::AVERAGE);
	_groupSliceView.addButton(_ui->_radioMedianSlice,SliceType::MEDIAN);
	_groupSliceView.setExclusive(true);

	/**** Mise en place de la communication MVC ****/

	// Évènements déclenchés par le slider de n° de coupe
	QObject::connect(_ui->_sliderSelectSlice, SIGNAL(valueChanged(int)), this, SLOT(drawSlice(int)));

	// Évènements déclenchés par les boutons de sélection de la vue
	QObject::connect(&_groupSliceView, SIGNAL(buttonClicked(int)), this, SLOT(setTypeOfView(int)));

	// Évènements déclenchés par le slider de seuillage
	QObject::connect(_ui->_spansliderSliceThreshold, SIGNAL(lowerValueChanged(int)), this, SLOT(setLowThreshold(int)));
	QObject::connect(_ui->_spinMinThreshold, SIGNAL(valueChanged(int)), this, SLOT(setLowThreshold(int)));
	QObject::connect(_ui->_spansliderSliceThreshold, SIGNAL(upperValueChanged(int)), this, SLOT(setHighThreshold(int)));
	QObject::connect(_ui->_spinMaxThreshold, SIGNAL(valueChanged(int)), this, SLOT(setHighThreshold(int)));

	// Évènements déclenchés par le bouton de mise à jour de l'histogramme
	QObject::connect(_ui->_buttonUpdateSliceHistogram, SIGNAL(clicked()), this, SLOT(updateSliceHistogram()));

	// Évènements déclenchés par les bouton associès à la moelle
	QObject::connect(_ui->_buttonComputeMarrow, SIGNAL(clicked()), this, SLOT(updateMarrow()));

	// Évènements déclenchés par les bouton associès aux histogrammes de secteurs
	QObject::connect(_ui->_buttonUpdateSectors, SIGNAL(clicked()), this, SLOT(updateSectorsHistograms()));
	QObject::connect(_ui->_comboSelectSector, SIGNAL(currentIndexChanged(int)), this, SLOT(selectSectorHistogram(int)));
	QObject::connect(_ui->_spinMinSectorSlice, SIGNAL(valueChanged(int)), this, SLOT(updateMaximumSectorsIntervalExtremum(int)));
	QObject::connect(_ui->_buttonSliceMinimalSector, SIGNAL(clicked()), this, SLOT(setMinimumSectorsIntervalToCurrentSlice()));
	QObject::connect(_ui->_spinMaxSectorSlice, SIGNAL(valueChanged(int)), this, SLOT(updateMinimumSectorsIntervalExtremum(int)));
	QObject::connect(_ui->_buttonSliceMaximalSector, SIGNAL(clicked()), this, SLOT(setMaximumSectorsIntervalToCurrentSlice()));
	QObject::connect(_ui->_spinMinimumIntensityForSum, SIGNAL(valueChanged(int)), this, SLOT(setMinimalDifferenceForSectors(int)));

	// Raccourcis des actions du menu
	_ui->_actionOpenDicom->setShortcut(Qt::CTRL + Qt::Key_O);
	QObject::connect(_ui->_actionOpenDicom, SIGNAL(triggered()), this, SLOT(openDicom()));
	_ui->_actionCloseImage->setShortcut(Qt::CTRL + Qt::Key_W);
	QObject::connect(_ui->_actionCloseImage, SIGNAL(triggered()), this, SLOT(closeImage()));
	_ui->_actionQuit->setShortcut(Qt::CTRL + Qt::Key_Q);
	QObject::connect(_ui->_actionQuit, SIGNAL(triggered()), this, SLOT(close()));

	_ui->_radioCurrentSlice->click();
}

MainWindow::~MainWindow() {
	while ( !_pieChartPlots.isEmpty() ) {
		_pieChartPlots.removeLast();
	}
	if ( _pieChartDiagrams != 0 ) delete _pieChartDiagrams;
	if ( _pieChart != 0 ) delete _pieChart;
	if ( _marrow != 0 ) delete _marrow;
	if ( _sliceHistogram != 0 ) delete _sliceHistogram;
	if ( _sliceView != 0 ) delete _sliceView;
	if ( _billon != 0 ) delete _billon;
	if ( _ui != 0 ) delete _ui;
}

/*******************************
 * Public fonctions
 *******************************/

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
	bool res = true;
	if (obj == _ui->_labelSliceView) {
		if ( _billon != 0 && _pieChartDiagrams != 0 && event->type() == QEvent::MouseButtonPress ) {
			QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
			const int x = mouseEvent->x();
			const int y = mouseEvent->y();
			const int centerX = _marrow!=0 ? _marrow->at(_ui->_sliderSelectSlice->value()).x:_billon->n_cols/2;
			const int centerY = _marrow!=0 ? _marrow->at(_ui->_sliderSelectSlice->value()).y:_billon->n_rows/2;

			const int sector = _pieChart->partOfAngle( TWO_PI-ANGLE(centerX,centerY,x,y) );

			if ( sector > -1 && sector < _ui->_comboSelectSector->count()) {
				_ui->_comboSelectSector->setCurrentIndex(sector);
			}
		}
		else {
			res = false;
		}
	 }
	else {
		 res = QMainWindow::eventFilter(obj, event);
	 }
	return res;
}

/*******************************
 * Private slots
 *******************************/

void MainWindow::openDicom() {
	QString folderName = QFileDialog::getExistingDirectory(0,tr("Sélection du répertoire DICOM"),QDir::homePath(),QFileDialog::ShowDirsOnly);
	if ( !folderName.isEmpty() ) {
		// Lecture des fichiers DICOM
		closeImage();
		openNewBillon(folderName);
		updateSliceHistogram();
		updateComponentsValues();
		drawSlice();
	}
}

void MainWindow::closeImage() {
	openNewBillon();
	updateMarrow();
	updateSliceHistogram();
	updateSectorsHistograms();
	updateComponentsValues();
	drawSlice();
}

void MainWindow::drawSlice( const int &sliceNumber ) {
	if ( _billon != 0 ) {
		_ui->_labelSliceNumber->setNum(sliceNumber);
		_pix = QPixmap::fromImage(QImage(_billon->n_cols,_billon->n_rows,QImage::Format_ARGB32));
		QPainter painter(&_pix);
		if ( _sliceView != 0 ) _sliceView->drawSlice(painter,sliceNumber);
		if ( _sliceHistogram != 0 ) highlightSliceHistogram(sliceNumber);
		if ( _marrow != 0 ) _marrow->draw(painter,sliceNumber);
		if ( _pieChart != 0 && !_pieChartPlots.isEmpty() && sliceNumber >= _ui->_spinMinSectorSlice->value() && sliceNumber <= _ui->_spinMaxSectorSlice->value()) {
			Coord2D center(_pix.width()/2,_pix.height()/2);
			if ( _marrow != 0 && sliceNumber >= _marrow->beginSlice() && sliceNumber <= _marrow->endSlice()) {
				center = _marrow->at(sliceNumber-_marrow->beginSlice());
			}
			_pieChart->draw(painter,_ui->_comboSelectSector->currentIndex(), center);
		}
	}
	else {
		_ui->_labelSliceNumber->setText(tr("Aucune"));
		_pix = QPixmap::fromImage(QImage(0,0));
	}
	_ui->_labelSliceView->setPixmap(_pix);
}

void MainWindow::setTypeOfView( const int &type ) {
	enabledComponents();
	if ( _sliceView != 0 ) _sliceView->setTypeOfView( static_cast<const SliceType::SliceType>(type) );
	drawSlice();
}

void MainWindow::setLowThreshold( const int &threshold ) {
	if ( _sliceView != 0 ) _sliceView->setLowThreshold(threshold);
	if ( _sliceHistogram ) _sliceHistogram->setLowThreshold(threshold);
	if ( _pieChartDiagrams != 0 ) _pieChartDiagrams->setLowThreshold(threshold);

	_ui->_spansliderSliceThreshold->blockSignals(true);
		_ui->_spansliderSliceThreshold->setLowerValue(threshold);
	_ui->_spansliderSliceThreshold->blockSignals(false);

	_ui->_spinMinThreshold->blockSignals(true);
		_ui->_spinMinThreshold->setValue(threshold);
		_ui->_spinMaxThreshold->setMinimum(threshold);
	_ui->_spinMinThreshold->blockSignals(false);

	drawSlice();
}

void MainWindow::setHighThreshold( const int &threshold ) {
	if ( _sliceView != 0 ) _sliceView->setHighThreshold(threshold);
	if ( _sliceHistogram ) _sliceHistogram->setHighThreshold(threshold);
	if ( _pieChartDiagrams != 0 ) _pieChartDiagrams->setHighThreshold(threshold);

	_ui->_spansliderSliceThreshold->blockSignals(true);
		_ui->_spansliderSliceThreshold->setUpperValue(threshold);
	_ui->_spansliderSliceThreshold->blockSignals(false);

	_ui->_spinMaxThreshold->blockSignals(true);
		_ui->_spinMaxThreshold->setValue(threshold);
		_ui->_spinMinThreshold->setMaximum(threshold);
	_ui->_spinMaxThreshold->blockSignals(false);

	drawSlice();
}

void MainWindow::updateSliceHistogram() {
	if ( _sliceHistogram ) _sliceHistogram->constructHistogram();
	_ui->_plotSliceHistogram->setAxisScale(QwtPlot::xBottom,0,(_billon != 0)?_billon->n_slices:0);
	highlightSliceHistogram(_ui->_sliderSelectSlice->value());
}


void MainWindow::highlightSliceHistogram( const int &slicePosition ) {
	qreal height = _sliceHistogram->sample(slicePosition).value;
	qreal x[4] = {slicePosition,slicePosition, slicePosition+1,slicePosition+1};
	qreal y[4] = {0,height,height,0};
	_histogramCursor.setSamples(x,y,4);
	_ui->_plotSliceHistogram->replot();
}

void MainWindow::updateMarrow() {
	if ( _marrow != 0 ) {
		delete _marrow;
		_marrow = 0;
	}
	if ( _billon != 0 ) {
		MarrowExtractor extractor;
		_marrow = extractor.process(*_billon,0,_billon->n_slices-1);
	}
	_pieChartDiagrams->setModel(_marrow);
	drawSlice();
}

void MainWindow::updateSectorsHistograms() {
	if ( _pieChart != 0 ) {
		_pieChart->setOrientation(_ui->_spinSectorsOrientation->value()*DEG_TO_RAD_FACT);
		_pieChart->setSectorsNumber(_ui->_spinSectorsNumber->value());
	}

	while ( !_pieChartPlots.isEmpty() ) {
		_ui->_stackedSectorsHistograms->removeWidget(_pieChartPlots.last());
		_pieChartPlots.removeLast();
	}

	_ui->_comboSelectSector->clear();
	_ui->_polarTest->detachItems(QwtPolarItem::Rtti_PolarCurve,false);
	_ui->_polarTest->replot();

	if ( _pieChartDiagrams != 0 ) {
		_pieChartDiagrams->setBillonInterval(_ui->_spinMinSectorSlice->value(),_ui->_spinMaxSectorSlice->value());
		_pieChartDiagrams->compute();

		const int nbHistograms = _pieChartDiagrams->count();
		if ( nbHistograms != 0 ) {

			_pieChartPlots.reserve(nbHistograms);
			for ( int i=0 ; i<nbHistograms ; ++i ) {
				QwtPlot * plot = new QwtPlot();
				_pieChartPlots.append(plot);
				_ui->_stackedSectorsHistograms->addWidget(plot);
				_ui->_comboSelectSector->addItem(tr("Secteur %1").arg(i));
			}

			_pieChartDiagrams->attach(_pieChartPlots);
			_pieChartDiagrams->attach(_ui->_polarTest);

			for ( int i=0 ; i<nbHistograms ; ++i ) {
				_pieChartPlots[i]->replot();
			}
			_ui->_polarTest->replot();

			_ui->_labelSectorsOrientation->setNum(_ui->_spinSectorsOrientation->value());
			_ui->_labelSectorsNumber->setNum(_ui->_spinSectorsNumber->value());
		}
		else {
			_ui->_labelSectorsOrientation->setText(tr("Aucune"));
			_ui->_labelSectorsNumber->setText(tr("Aucun"));
		}
	}

	selectSectorHistogram(0);
}

void MainWindow::selectSectorHistogram( const int &sectorIdx ) {
	_ui->_stackedSectorsHistograms->setCurrentIndex(sectorIdx);
	drawSlice();
}

void MainWindow::setMinimumSectorsIntervalToCurrentSlice() {
	_ui->_spinMinSectorSlice->setValue(_ui->_sliderSelectSlice->value());
}

void MainWindow::setMaximumSectorsIntervalToCurrentSlice() {
	_ui->_spinMaxSectorSlice->setValue(_ui->_sliderSelectSlice->value());
}

void MainWindow::updateMinimumSectorsIntervalExtremum( const int &value ) {
	_ui->_spinMinSectorSlice->setMaximum(value);
}

void MainWindow::updateMaximumSectorsIntervalExtremum( const int &value ) {
	_ui->_spinMaxSectorSlice->setMinimum(value);
}

void MainWindow::setMinimalDifferenceForSectors( const int &minimalDifference ) {
	if ( _pieChartDiagrams != 0 ) {
		_pieChartDiagrams->setMinimalDifference(minimalDifference);
	}
}

/*******************************
 * Private functions
 *******************************/

void MainWindow::openNewBillon( const QString &folderName ) {
	if ( _billon != 0 ) {
		delete _billon;
		_billon = 0;
	}
	if ( !folderName.isEmpty() ) {
		_billon = DicomReader::read(folderName);
	}
	if ( _sliceView != 0 ) _sliceView->setModel(_billon);
	if ( _sliceHistogram != 0 ) _sliceHistogram->setModel(_billon);
	if ( _pieChartDiagrams != 0 ) _pieChartDiagrams->setModel(_billon);
}

void MainWindow::drawSlice() {
	drawSlice(_ui->_sliderSelectSlice->value());
}

void MainWindow::updateComponentsValues() {
	int minValue, maxValue, nbSlices;
	const bool existBillon = (_billon != 0);

	if ( existBillon ) {
		minValue = _billon->minValue();
		maxValue = _billon->maxValue();
		nbSlices = _billon->n_slices;
		_ui->_labelSliceNumber->setNum(0);
	}
	else {
		minValue = maxValue = 0;
		nbSlices = 1;
		_ui->_labelSliceNumber->setText(tr("Aucune coupe présente."));
	}

	_ui->_spansliderSliceThreshold->setMinimum(minValue);
	_ui->_spansliderSliceThreshold->setLowerValue(minValue);
	_ui->_spansliderSliceThreshold->setMaximum(maxValue);
	_ui->_spansliderSliceThreshold->setUpperValue(maxValue);

	_ui->_spinMinThreshold->setMinimum(minValue);
	_ui->_spinMinThreshold->setMaximum(maxValue);
	_ui->_spinMinThreshold->setValue(minValue);

	_ui->_spinMaxThreshold->setMinimum(minValue);
	_ui->_spinMaxThreshold->setMaximum(maxValue);
	_ui->_spinMaxThreshold->setValue(maxValue);

	_ui->_spinMinSectorSlice->setMinimum(0);
	_ui->_spinMinSectorSlice->setMaximum(nbSlices-1);
	_ui->_spinMinSectorSlice->setValue(0);

	_ui->_spinMaxSectorSlice->setMinimum(0);
	_ui->_spinMaxSectorSlice->setMaximum(nbSlices-1);
	_ui->_spinMaxSectorSlice->setValue(nbSlices-1);

	_ui->_sliderSelectSlice->setValue(0);
	_ui->_sliderSelectSlice->setRange(0,nbSlices-1);

	_ui->_spinMinimumIntensityForSum->setRange(0,maxValue-minValue);

	enabledComponents();
}

void MainWindow::enabledComponents() {
	const bool enable = (_billon != 0) && ( _groupSliceView.checkedId() == SliceType::CURRENT );
	_ui->_sliderSelectSlice->setEnabled(enable);
	_ui->_spansliderSliceThreshold->setEnabled(enable);
	_ui->_buttonComputeMarrow->setEnabled(enable);
	_ui->_buttonUpdateSliceHistogram->setEnabled(enable);
	_ui->_buttonSliceMaximalSector->setEnabled(enable);
	_ui->_buttonSliceMinimalSector->setEnabled(enable);
	_ui->_buttonUpdateSectors->setEnabled(enable);
	_ui->_comboSelectSector->setEnabled(enable);
}
