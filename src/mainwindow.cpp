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
#include "inc/datexport.h"
#include "inc/ofsexport.h"

#include <QFileDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>

MainWindow::MainWindow( QWidget *parent ) : QMainWindow(parent), _ui(new Ui::MainWindow), _billon(0), _marrow(0), _sliceView(new SliceView()), _sliceHistogram(new SliceHistogram()), _pieChart(new PieChart(0,1)), _pieChartDiagrams(new PieChartDiagrams()), _currentSlice(0), _currentMaximum(0) {
	_ui->setupUi(this);
	setCorner(Qt::TopLeftCorner,Qt::LeftDockWidgetArea);
	setCorner(Qt::TopRightCorner,Qt::RightDockWidgetArea);
	setCorner(Qt::BottomLeftCorner,Qt::LeftDockWidgetArea);
	setCorner(Qt::BottomRightCorner,Qt::RightDockWidgetArea);

	// Paramétrisation des composant graphiques
	_ui->_labelSliceView->installEventFilter(this);
	_ui->_labelSliceView->installEventFilter(&_sliceZoomer);

	_ui->_plotSliceHistogram->enableAxis(QwtPlot::yLeft,false);

	_histogramCursor.attach(_ui->_plotSliceHistogram);
	_histogramCursor.setBrush(Qt::red);

	_ui->_comboSliceType->insertItem(SliceType::CURRENT,tr("Coupe originale"));
	_ui->_comboSliceType->insertItem(SliceType::AVERAGE,tr("Coupe moyenne"));
	_ui->_comboSliceType->insertItem(SliceType::MEDIAN,tr("Coupe médiane"));
	_ui->_comboSliceType->insertItem(SliceType::MOVEMENT,tr("Coupe de mouvements"));
	_ui->_comboSliceType->insertItem(SliceType::FLOW,tr("Coupe de flots optiques"));

	_ui->_spinFlowAlpha->setValue(_sliceView->flowAlpha());
	_ui->_spinFlowEpsilon->setValue(_sliceView->flowEpsilon());
	_ui->_spinFlowMaximumIterations->setValue(_sliceView->flowMaximumIterations());

	/**** Mise en place de la communication MVC ****/

	// Évènements déclenchés par le slider de n° de coupe
	QObject::connect(_ui->_sliderSelectSlice, SIGNAL(valueChanged(int)), this, SLOT(drawSlice(int)));

	// Évènements déclenchés par les boutons de sélection de la vue
	QObject::connect(_ui->_comboSliceType, SIGNAL(activated(int)), this, SLOT(setTypeOfView(int)));
	QObject::connect(_ui->_sliderMotionThreshold, SIGNAL(valueChanged(int)), this, SLOT(setMotionThreshold(int)));
	QObject::connect(_ui->_spinMotionThreshold, SIGNAL(valueChanged(int)), this, SLOT(setMotionThreshold(int)));
	QObject::connect(_ui->_sliderMovementMinimumRadius, SIGNAL(valueChanged(int)), this, SLOT(setMotionGroupMinimumRadius(int)));
	QObject::connect(_ui->_spinMovementMinimumRadius, SIGNAL(valueChanged(int)), this, SLOT(setMotionGroupMinimumRadius(int)));
	QObject::connect(_ui->_checkDrawMotionWithBackground, SIGNAL(toggled(bool)), this, SLOT(enableMovementWithBackground(bool)));
	QObject::connect(_ui->_checkUseNextSlice, SIGNAL(toggled(bool)), this, SLOT(useNextSliceInsteadOfCurrentSlice(bool)));
	QObject::connect(_ui->_buttonFlowApplied, SIGNAL(clicked()), this, SLOT(flowApplied()));

	// Évènements déclenchés par le slider de seuillage
	QObject::connect(_ui->_spansliderSliceThreshold, SIGNAL(lowerValueChanged(int)), this, SLOT(setLowThreshold(int)));
	QObject::connect(_ui->_spinMinThreshold, SIGNAL(valueChanged(int)), this, SLOT(setLowThreshold(int)));
	QObject::connect(_ui->_spansliderSliceThreshold, SIGNAL(upperValueChanged(int)), this, SLOT(setHighThreshold(int)));
	QObject::connect(_ui->_spinMaxThreshold, SIGNAL(valueChanged(int)), this, SLOT(setHighThreshold(int)));

	// Évènements déclenchés par les boutons relatifs à l'histogramme de cumul des intensités
	QObject::connect(_ui->_buttonUpdateSliceHistogram, SIGNAL(clicked()), this, SLOT(updateSliceHistogram()));
	QObject::connect(_ui->_buttonPreviousMaximum, SIGNAL(clicked()), this, SLOT(previousMaximumInSliceHistogram()));
	QObject::connect(_ui->_buttonNextMaximum, SIGNAL(clicked()), this, SLOT(nextMaximumInSliceHistogram()));

	// Évènements déclenchés par les bouton associès à la moelle
	QObject::connect(_ui->_buttonComputeMarrow, SIGNAL(clicked()), this, SLOT(updateMarrow()));

	// Évènements déclenchés par les bouton associès aux histogrammes de secteurs
	QObject::connect(_ui->_buttonUpdateSectors, SIGNAL(clicked()), this, SLOT(updateSectorsHistograms()));
	QObject::connect(_ui->_comboSelectSector, SIGNAL(currentIndexChanged(int)), this, SLOT(selectSectorHistogram(int)));
	QObject::connect(_ui->_spinMinimumIntensityForSum, SIGNAL(valueChanged(int)), this, SLOT(setMinimalDifferenceForSectors(int)));

	// Évènements déclenchés par la souris sur le visualiseur de coupes
	QObject::connect(&_sliceZoomer, SIGNAL(zoomFactorChanged(qreal,QPoint)), this, SLOT(zoomInSliceView(qreal,QPoint)));
	QObject::connect(&_sliceZoomer, SIGNAL(isMovedFrom(QPoint)), this, SLOT(dragInSliceView(QPoint)));

	// Évènements déclenchés par les boutons relatifs aux intervalles de coupes
	QObject::connect(_ui->_spinMinSlice, SIGNAL(valueChanged(int)), &_slicesInterval, SLOT(setMin(int)));
	QObject::connect(_ui->_buttonMinSlice, SIGNAL(clicked()), this, SLOT(setMinimumOfSlicesIntervalToCurrentSlice()));
	QObject::connect(_ui->_spinMaxSlice, SIGNAL(valueChanged(int)), &_slicesInterval, SLOT(setMax(int)));
	QObject::connect(_ui->_buttonMaxSlice, SIGNAL(clicked()), this, SLOT(setMaximumOfSlicesIntervalToCurrentSlice()));
	QObject::connect(_ui->_buttonExportDat, SIGNAL(clicked()), this, SLOT(exportToDat()));
	QObject::connect(_ui->_buttonExportOfs, SIGNAL(clicked()), this, SLOT(exportToOfs()));

	// Raccourcis des actions du menu
	_ui->_actionOpenDicom->setShortcut(Qt::CTRL + Qt::Key_O);
	QObject::connect(_ui->_actionOpenDicom, SIGNAL(triggered()), this, SLOT(openDicom()));
	_ui->_actionCloseImage->setShortcut(Qt::CTRL + Qt::Key_W);
	QObject::connect(_ui->_actionCloseImage, SIGNAL(triggered()), this, SLOT(closeImage()));
	_ui->_actionQuit->setShortcut(Qt::CTRL + Qt::Key_Q);
	QObject::connect(_ui->_actionQuit, SIGNAL(triggered()), this, SLOT(close()));

	_ui->_comboSliceType->setCurrentIndex(SliceType::CURRENT);
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
	if ( obj == _ui->_labelSliceView ) {
		if ( event->type() == QEvent::MouseButtonPress ) {
			const QMouseEvent *mouseEvent = static_cast<const QMouseEvent*>(event);
			if ( (mouseEvent->button() == Qt::LeftButton) && _billon != 0 && _pieChartDiagrams != 0 ) {
				const int x = mouseEvent->x()/_sliceZoomer.factor();
				const int y = mouseEvent->y()/_sliceZoomer.factor();
				const int centerX = (_marrow !=0) ? _marrow->at(_currentSlice).x:_billon->n_cols/2;
				const int centerY = (_marrow !=0) ? _marrow->at(_currentSlice).y:_billon->n_rows/2;

				const int sector = _pieChart->partOfAngle( TWO_PI-ANGLE(centerX,centerY,x,y) );

				if ( sector > -1 && sector < _ui->_comboSelectSector->count()) {
					_ui->_comboSelectSector->setCurrentIndex(sector);
				}
			}
		}
	}
	return QMainWindow::eventFilter(obj, event);
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
	_currentSlice = sliceNumber;
	if ( _billon != 0 ) {
		_ui->_labelSliceNumber->setNum(sliceNumber);
		_pix.fill(0xff000000);
		QPainter painter(&_pix);
		if ( _sliceView != 0 ) {
			painter.save();
			_sliceView->drawSlice(painter,sliceNumber,_intensityInterval);
			painter.restore();
		}
		if ( _sliceHistogram != 0 ) highlightSliceHistogram(sliceNumber);
		if ( _marrow != 0 ) {
			painter.save();
			_marrow->draw(painter,sliceNumber);
			painter.restore();
		}
		if ( _pieChart != 0 && !_pieChartPlots.isEmpty() && sliceNumber >= _slicesInterval.min() && sliceNumber <= _slicesInterval.max()) {
			iCoord2D center(_pix.width()/2,_pix.height()/2);
			if ( _marrow != 0 && sliceNumber >= _marrow->beginSlice() && sliceNumber <= _marrow->endSlice()) {
				center = _marrow->at(sliceNumber-_marrow->beginSlice());
			}
			painter.save();
			_pieChart->draw(painter,_ui->_comboSelectSector->currentIndex(), center);
			painter.restore();
		};
		painter.end();
	}
	else {
		_ui->_labelSliceNumber->setText(tr("Aucune"));
		_pix = QImage(0,0);
	}
	_ui->_labelSliceView->setPixmap(_billon != 0 ? QPixmap::fromImage(_pix).scaled(_pix.width()*_sliceZoomer.factor(),_pix.height()*_sliceZoomer.factor(),Qt::KeepAspectRatio) : QPixmap::fromImage(_pix));
}

void MainWindow::setTypeOfView( const int &type ) {
	enabledComponents();
	if ( _sliceView != 0 ) {
		_sliceView->setTypeOfView( static_cast<const SliceType::SliceType>(type) );
		switch (type) {
			case SliceType::MOVEMENT:
				_ui->_toolboxSliceParameters->setCurrentWidget(_ui->_pageMotionParameters);
				break;
			case SliceType::FLOW:
				_ui->_toolboxSliceParameters->setCurrentWidget(_ui->_pageFlowParameters);
				break;
			default:
				break;
		}
	}
	drawSlice();
}

void MainWindow::setLowThreshold( const int &threshold ) {
	_intensityInterval.setMin(threshold);

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
	_intensityInterval.setMax(threshold);

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
	if ( _sliceHistogram != 0 ) {
		_sliceHistogram->detach();
		_sliceHistogram->clear();
		if ( _billon != 0 ) {
			if ( _marrow != 0 )	_sliceHistogram->constructHistogram(*_billon, *_marrow, _intensityInterval);
			else _sliceHistogram->constructHistogram(*_billon, _intensityInterval);
		}
		_histogramCursor.detach();
		_sliceHistogram->attach(_ui->_plotSliceHistogram);
		_histogramCursor.attach(_ui->_plotSliceHistogram);
	}
	_ui->_plotSliceHistogram->setAxisScale(QwtPlot::xBottom,0,(_billon != 0)?_billon->n_slices:0);
	highlightSliceHistogram(_currentSlice);
}


void MainWindow::highlightSliceHistogram( const int &slicePosition ) {
	qreal height = _sliceHistogram->value(slicePosition);
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
	_sliceView->setModel(_marrow);
	drawSlice();
}

void MainWindow::updateSectorsHistograms() {
	_pieChartDiagrams->detach();

	if ( _pieChart != 0 ) {
		_pieChart->setOrientation(_ui->_spinSectorsOrientation->value()*DEG_TO_RAD_FACT);
		_pieChart->setSectorsNumber(_ui->_spinSectorsNumber->value());
	}

	while ( !_pieChartPlots.isEmpty() ) {
		_ui->_stackedSectorsHistograms->removeWidget(_pieChartPlots.last());
		_pieChartPlots.removeLast();
	}

	_ui->_comboSelectSector->clear();
	_ui->_polarSectorSum->replot();

	if ( _pieChartDiagrams != 0 ) {
		if ( _marrow != 0 )	_pieChartDiagrams->compute( *_billon, *_pieChart, *_marrow, _slicesInterval, _intensityInterval );
		else _pieChartDiagrams->compute( *_billon, *_pieChart, _slicesInterval, _intensityInterval );

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
			_pieChartDiagrams->attach(_ui->_polarSectorSum);

			for ( int i=0 ; i<nbHistograms ; ++i ) {
				_pieChartPlots[i]->replot();
			}
			_ui->_polarSectorSum->replot();

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

void MainWindow::setMinimumOfSlicesIntervalToCurrentSlice() {
	_ui->_spinMaxSlice->setMinimum(_currentSlice);
	_ui->_spinMinSlice->setMaximum(qMax(_currentSlice,_ui->_spinMinSlice->maximum()));
	_ui->_spinMinSlice->setValue(_currentSlice);
}

void MainWindow::setMaximumOfSlicesIntervalToCurrentSlice() {
	_ui->_spinMinSlice->setMaximum(_currentSlice);
	_ui->_spinMaxSlice->setMinimum(qMin(_currentSlice,_ui->_spinMaxSlice->minimum()));
	_ui->_spinMaxSlice->setValue(_currentSlice);
}

void MainWindow::setMinimalDifferenceForSectors( const int &minimalDifference ) {
	if ( _pieChartDiagrams != 0 ) {
		_pieChartDiagrams->setMinimalDifference(minimalDifference);
	}
}

void MainWindow::previousMaximumInSliceHistogram() {
	if ( _sliceHistogram != 0 ) {
		const int nbMaximums = _sliceHistogram->nbMaximums();
		_currentMaximum = nbMaximums <= 0 ? -1 : _currentMaximum < 0 ? 0 : _currentMaximum == 0 ? nbMaximums-1 : ( _currentMaximum - 1 ) % nbMaximums;
		int sliceIndex = _sliceHistogram->sliceOfIemeMaximum(_currentMaximum);
		if ( sliceIndex > -1 ) {
			_ui->_sliderSelectSlice->setValue(sliceIndex);
		}
	}
}

void MainWindow::nextMaximumInSliceHistogram() {
	if ( _sliceHistogram != 0 ) {
		const int nbMaximums = _sliceHistogram->nbMaximums();
		_currentMaximum = nbMaximums>0 ? ( _currentMaximum + 1 ) % nbMaximums : -1;
		int sliceIndex = _sliceHistogram->sliceOfIemeMaximum(_currentMaximum);
		if ( sliceIndex > -1 ) {
			_ui->_sliderSelectSlice->setValue(sliceIndex);
		}
	}
}

void MainWindow::zoomInSliceView( const qreal &zoomFactor, const QPoint &focalPoint ) {
	_ui->_labelSliceView->setPixmap(_billon != 0 ? QPixmap::fromImage(_pix).scaled(_pix.width()*zoomFactor,_pix.height()*zoomFactor,Qt::KeepAspectRatio) : QPixmap::fromImage(_pix));
	QScrollArea &scrollArea = *(_ui->_scrollSliceView);
	scrollArea.horizontalScrollBar()->setValue(focalPoint.x()/(qreal)(_ui->_labelSliceView->pixmap()->width())*(scrollArea.horizontalScrollBar()->maximum()*(1./0.9)));
	scrollArea.verticalScrollBar()->setValue(focalPoint.y()/(qreal)(_ui->_labelSliceView->pixmap()->height())*(scrollArea.verticalScrollBar()->maximum()*(1./0.9)));
}

void MainWindow::dragInSliceView( const QPoint &motionVector ) {
	QScrollArea &scrollArea = *(_ui->_scrollSliceView);
	if ( motionVector.x() != 0 ) scrollArea.horizontalScrollBar()->setValue(scrollArea.horizontalScrollBar()->value()-motionVector.x());
	if ( motionVector.y() != 0 ) scrollArea.verticalScrollBar()->setValue(scrollArea.verticalScrollBar()->value()-motionVector.y());
}

void MainWindow::setMotionThreshold( const int &threshold ) {
	if ( _sliceView != 0 ) {
		_sliceView->setMotionThreshold(threshold);

		_ui->_spinMotionThreshold->blockSignals(true);
			_ui->_spinMotionThreshold->setValue(threshold);
		_ui->_spinMotionThreshold->blockSignals(false);

		_ui->_sliderMotionThreshold->blockSignals(true);
			_ui->_sliderMotionThreshold->setValue(threshold);
		_ui->_sliderMotionThreshold->blockSignals(false);

		drawSlice();
	}
}

void MainWindow::setMotionGroupMinimumRadius( const int &radius ) {
	if ( _sliceView != 0 ) {
		_sliceView->setMotionGroupMinimumRadius(radius);

		_ui->_spinMovementMinimumRadius->blockSignals(true);
			_ui->_spinMovementMinimumRadius->setValue(radius);
		_ui->_spinMovementMinimumRadius->blockSignals(false);

		_ui->_sliderMovementMinimumRadius->blockSignals(true);
			_ui->_sliderMovementMinimumRadius->setValue(radius);
		_ui->_sliderMovementMinimumRadius->blockSignals(false);

		drawSlice();
	}
}

void MainWindow::enableMovementWithBackground( const bool &enable ) {
	if ( _sliceView != 0 ) {
		_sliceView->enableMotionWithBackground(enable);
		drawSlice();
	}
}

void MainWindow::useNextSliceInsteadOfCurrentSlice( const bool &enable ) {
	if ( _sliceView != 0 ) {
		_sliceView->useNextSliceInsteadOfCurrentSlice(enable);
		drawSlice();
	}
}

void MainWindow::flowApplied() {
	if ( _sliceView != 0 ) {
		const qreal currentAlpha = _sliceView->flowAlpha();
		const qreal currentEpsilon = _sliceView->flowEpsilon();
		const qreal currentMaxIter = _sliceView->flowMaximumIterations();

		const qreal newAlpha = _ui->_spinFlowAlpha->value();
		const qreal newEpsilon = _ui->_spinFlowEpsilon->value();
		const qreal newMaxIter = _ui->_spinFlowMaximumIterations->value();

		bool hasModification = false;
		if ( currentAlpha != newAlpha ) {
			_sliceView->setFlowAlpha(newAlpha);
			hasModification = true;
		}
		if ( currentEpsilon != newEpsilon ) {
			_sliceView->setFlowEpsilon(newEpsilon);
			hasModification = true;
		}
		if ( currentMaxIter != newMaxIter ) {
			_sliceView->setFlowMaximumIterations(newMaxIter);
			hasModification = true;
		}

		if ( hasModification ) drawSlice();
	}
}

void MainWindow::exportToDat() {
	if ( _billon != 0 ) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .dat"), "output.dat", tr("Fichiers de données (*.dat);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() ) {
			DatExport::process( *_billon, _slicesInterval, fileName, _ui->_spinExportResolution->value() );
		}
	}
}

void MainWindow::exportToOfs() {
	if ( _billon != 0 && _marrow != 0 ) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .ofs"), "output.ofs", tr("Fichiers de données (*.ofs);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() ) {
			OfsExport::process( *_billon, *_marrow, _slicesInterval, fileName, _ui->_spinExportNbEdges->value(), _ui->_spinExportRadius->value() );
		}
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
	if ( _billon != 0 ) {
		_pix = QImage(_billon->n_cols, _billon->n_rows,QImage::Format_ARGB32);
		_intensityInterval.setBounds(_billon->minValue(),_billon->maxValue());
	}
	else {
		_pix = QImage(0,0,QImage::Format_ARGB32);
		_intensityInterval.setBounds(1,0);
	}
}

void MainWindow::drawSlice() {
	drawSlice(_currentSlice);
}

void MainWindow::updateComponentsValues() {
	int minValue, maxValue, nbSlices;
	const bool existBillon = (_billon != 0);

	if ( existBillon ) {
		minValue = _billon->minValue();
		maxValue = _billon->maxValue();
		nbSlices = _billon->n_slices;
		_ui->_labelSliceNumber->setNum(0);
		_ui->_scrollSliceView->setFixedSize(_billon->n_cols,_billon->n_rows);
	}
	else {
		minValue = maxValue = 0;
		nbSlices = 1;
		_ui->_labelSliceNumber->setText(tr("Aucune coupe présente."));
		_ui->_scrollSliceView->setFixedSize(0,0);
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

	_ui->_spinMinSlice->setMinimum(0);
	_ui->_spinMinSlice->setMaximum(nbSlices-1);
	_ui->_spinMinSlice->setValue(0);

	_ui->_spinMaxSlice->setMinimum(0);
	_ui->_spinMaxSlice->setMaximum(nbSlices-1);
	_ui->_spinMaxSlice->setValue(nbSlices-1);

	_ui->_sliderSelectSlice->setValue(0);
	_ui->_sliderSelectSlice->setRange(0,nbSlices-1);

	_ui->_spinMinimumIntensityForSum->setRange(0,maxValue-minValue);

	enabledComponents();
}

void MainWindow::enabledComponents() {
	const int sliceType = _ui->_comboSliceType->currentIndex();
	const bool enable = (_billon != 0) && ( sliceType == SliceType::CURRENT || sliceType == SliceType::MOVEMENT || sliceType == SliceType::FLOW );
	_ui->_sliderSelectSlice->setEnabled(enable);
	_ui->_spansliderSliceThreshold->setEnabled(enable);
	_ui->_buttonComputeMarrow->setEnabled(enable);
	_ui->_buttonUpdateSliceHistogram->setEnabled(enable);
	_ui->_buttonMaxSlice->setEnabled(enable);
	_ui->_buttonMinSlice->setEnabled(enable);
	_ui->_buttonUpdateSectors->setEnabled(enable);
	_ui->_comboSelectSector->setEnabled(enable);
}
