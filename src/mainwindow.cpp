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
#include "inc/v3dexport.h"
#include "inc/histoexport.h"
#include "inc/opticalflow.h"
#include "inc/connexcomponentextractor.h"
#include "inc/pgm3dexport.h"

#include <QFileDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QMessageBox>
#include <qwt_plot_renderer.h>
#include <qwt_polar_renderer.h>

MainWindow::MainWindow( QWidget *parent ) : QMainWindow(parent), _ui(new Ui::MainWindow), _billon(0), _unusedBillon(0), _sectorBillon(0), _componentBillon(0), _marrow(0), _sliceView(new SliceView()), _sliceHistogram(new SliceHistogram()), _pieChart(new PieChart(0,1)), _pieChartDiagrams(new PieChartDiagrams()), _currentSlice(0), _currentMaximum(0), _isUsedOriginalBillon(true) {
	_ui->setupUi(this);
	setCorner(Qt::TopLeftCorner,Qt::LeftDockWidgetArea);
	setCorner(Qt::TopRightCorner,Qt::RightDockWidgetArea);
	setCorner(Qt::BottomLeftCorner,Qt::LeftDockWidgetArea);
	setCorner(Qt::BottomRightCorner,Qt::RightDockWidgetArea);

	// Paramétrisation des composant graphiques
	_ui->_labelSliceView->installEventFilter(this);
	_ui->_labelSliceView->installEventFilter(&_sliceZoomer);

	_ui->_plotSliceHistogram->enableAxis(QwtPlot::yLeft,false);
	_ui->_plotAngularHistogram->enableAxis(QwtPlot::yLeft,false);

	_histogramCursor.attach(_ui->_plotSliceHistogram);
	_histogramCursor.setBrush(Qt::red);

	_ui->_comboSliceType->insertItem(SliceType::CURRENT,tr("Coupe originale"));
	_ui->_comboSliceType->insertItem(SliceType::MOVEMENT,tr("Coupe de mouvements"));
	_ui->_comboSliceType->insertItem(SliceType::FLOW,tr("Coupe de flots optiques"));
	_ui->_comboSliceType->insertItem(SliceType::RESTRICTED_AREA,tr("Coupe de zone réduite"));
	//_ui->_comboSliceType->insertItem(SliceType::AVERAGE,tr("Coupe moyenne"));
	//_ui->_comboSliceType->insertItem(SliceType::MEDIAN,tr("Coupe médiane"));

	_ui->_spinFlowAlpha->setValue(_sliceView->flowAlpha());
	_ui->_spinFlowEpsilon->setValue(_sliceView->flowEpsilon());
	_ui->_spinFlowMaximumIterations->setValue(_sliceView->flowMaximumIterations());

	_ui->_polarSectorSum->setScale( QwtPolar::Azimuth, 0.0, TWO_PI );
	_pieChartDiagrams->attach(_ui->_polarSectorSum);
	_pieChartDiagrams->attach(_ui->_plotAngularHistogram);


	/**** Mise en place de la communication MVC ****/

	// Évènements déclenchés par le slider de n° de coupe
	QObject::connect(_ui->_sliderSelectSlice, SIGNAL(valueChanged(int)), this, SLOT(drawSlice(int)));

	// Évènements déclenchés par les boutons de sélection de la vue
	QObject::connect(_ui->_comboSliceType, SIGNAL(activated(int)), this, SLOT(setTypeOfView(int)));
	QObject::connect(_ui->_sliderMovementThresholdMin, SIGNAL(valueChanged(int)), this, SLOT(setMovementThresholdMin(int)));
	QObject::connect(_ui->_spinMovementThresholdMin, SIGNAL(valueChanged(int)), this, SLOT(setMovementThresholdMin(int)));
	QObject::connect(_ui->_sliderMovementThresholdMax, SIGNAL(valueChanged(int)), this, SLOT(setMovementThresholdMax(int)));
	QObject::connect(_ui->_spinMovementThresholdMax, SIGNAL(valueChanged(int)), this, SLOT(setMovementThresholdMax(int)));
	QObject::connect(_ui->_checkDrawMovementWithBackground, SIGNAL(toggled(bool)), this, SLOT(enableMovementWithBackground(bool)));
	QObject::connect(_ui->_checkUseNextSlice, SIGNAL(toggled(bool)), this, SLOT(useNextSliceInsteadOfCurrentSlice(bool)));
	QObject::connect(_ui->_buttonFlowApplied, SIGNAL(clicked()), this, SLOT(flowApplied()));
	QObject::connect(_ui->_spinRestrictedAreaResolution, SIGNAL(valueChanged(int)), this, SLOT(setRestrictedAreaResolution(int)));
	QObject::connect(_ui->_spinRestrictedAreaThreshold, SIGNAL(valueChanged(int)), this, SLOT(setRestrictedAreaThreshold(int)));
	QObject::connect(_ui->_checkRestrictedAreaEnableCircle, SIGNAL(toggled(bool)), this, SLOT(enableRestrictedAreaCircle(bool)));
	QObject::connect(_ui->_sliderRestrictedAreaBeginRadius, SIGNAL(valueChanged(int)), this, SLOT(setRestrictedAreaBeginRadius(int)));
	QObject::connect(_ui->_spinRestrictedAreaBeginRadius, SIGNAL(valueChanged(int)), this, SLOT(setRestrictedAreaBeginRadius(int)));
	QObject::connect(_ui->_buttonComputeRestrictedBillon, SIGNAL(clicked()), this, SLOT(computeRestrictedBillon()));
	QObject::connect(_ui->_buttonChangeBillonUsed, SIGNAL(clicked()), this, SLOT(changeBillonUsed()));

	// Évènements déclenchés par le slider de seuillage
	QObject::connect(_ui->_spansliderSliceThreshold, SIGNAL(lowerValueChanged(int)), this, SLOT(setLowThreshold(int)));
	QObject::connect(_ui->_spinMinThreshold, SIGNAL(valueChanged(int)), this, SLOT(setLowThreshold(int)));
	QObject::connect(_ui->_spansliderSliceThreshold, SIGNAL(upperValueChanged(int)), this, SLOT(setHighThreshold(int)));
	QObject::connect(_ui->_spinMaxThreshold, SIGNAL(valueChanged(int)), this, SLOT(setHighThreshold(int)));

	// Évènements déclenchés par les boutons relatifs à l'histogramme de cumul des intensités
	QObject::connect(_ui->_buttonUpdateSliceHistogram, SIGNAL(clicked()), this, SLOT(updateSliceHistogram()));
	QObject::connect(_ui->_buttonPreviousMaximum, SIGNAL(clicked()), this, SLOT(previousMaximumInSliceHistogram()));
	QObject::connect(_ui->_buttonNextMaximum, SIGNAL(clicked()), this, SLOT(nextMaximumInSliceHistogram()));
	QObject::connect(_ui->_sliderMarrowAroundDiameter, SIGNAL(valueChanged(int)), this, SLOT(setMarrowAroundDiameter(int)));
	QObject::connect(_ui->_spinMarrowAroundDiameter, SIGNAL(valueChanged(int)), this, SLOT(setMarrowAroundDiameter(int)));
	QObject::connect(_ui->_checkMarrowAroundDiameter, SIGNAL(clicked()), this, SLOT(drawSlice()));
	QObject::connect(_ui->_comboHistogramInterval, SIGNAL(activated(int)), this, SLOT(setHistogramIntervalType(int)));
	QObject::connect(_ui->_checkHistogramSmoothing, SIGNAL(toggled(bool)), this, SLOT(enableHistogramSmoothing(bool)));
	QObject::connect(_ui->_sliderMaximumsNeighborhood, SIGNAL(valueChanged(int)), this, SLOT(setHistogramMaximumsNeighborhood(int)));
	QObject::connect(_ui->_spinMaximumsNeighborhood, SIGNAL(valueChanged(int)), this, SLOT(setHistogramMaximumsNeighborhood(int)));
	QObject::connect(_ui->_sliderHistogramIntervalMinimumWidth, SIGNAL(valueChanged(int)), this, SLOT(setHistogramIntervalMinimumWidth(int)));
	QObject::connect(_ui->_spinHistogramIntervalMinimumWidth, SIGNAL(valueChanged(int)), this, SLOT(setHistogramIntervalMinimumWidth(int)));
	QObject::connect(_ui->_buttonExportSliceHistogram, SIGNAL(clicked()), this, SLOT(exportSliceHistogram()));

	// Évènements déclenchés par les bouton associès à la moelle
	QObject::connect(_ui->_buttonComputeMarrow, SIGNAL(clicked()), this, SLOT(updateMarrow()));

	// Évènements déclenchés par les bouton associès aux histogrammes de secteurs
	QObject::connect(_ui->_comboSelectSliceInterval, SIGNAL(activated(int)), this, SLOT(selectSliceInterval(int)));
	QObject::connect(_ui->_buttonSelectSliceIntervalUpdate, SIGNAL(clicked()), this, SLOT(selectCurrentSliceInterval()));
	QObject::connect(_ui->_comboSelectSectorInterval, SIGNAL(activated(int)), this, SLOT(selectSectorInterval(int)));
	QObject::connect(_ui->_buttonSelectSectorIntervalUpdate, SIGNAL(clicked()), this, SLOT(selectCurrentSectorInterval()));
	QObject::connect(_ui->_sliderSectorThresholding, SIGNAL(valueChanged(int)), _ui->_spinSectorThresholding, SLOT(setValue(int)));
	QObject::connect(_ui->_spinSectorThresholding, SIGNAL(valueChanged(int)), _ui->_sliderSectorThresholding, SLOT(setValue(int)));
	QObject::connect(_ui->_sliderMinimalSizeOfConnexComponents, SIGNAL(valueChanged(int)), _ui->_spinMinimalSizeOfConnexComponents, SLOT(setValue(int)));
	QObject::connect(_ui->_spinMinimalSizeOfConnexComponents, SIGNAL(valueChanged(int)), _ui->_sliderMinimalSizeOfConnexComponents, SLOT(setValue(int)));
	QObject::connect(_ui->_comboConnexComponents, SIGNAL(activated(int)), this, SLOT(drawSlice()));
	QObject::connect(_ui->_buttonExportSectorToPgm3D, SIGNAL(clicked()), this, SLOT(exportSectorToPgm3D()));
	QObject::connect(_ui->_buttonExportConnexComponentToPgm3D, SIGNAL(clicked()), this, SLOT(exportConnexComponentToPgm3D()));

	// Évènements déclenchés par la souris sur le visualiseur de coupes
	QObject::connect(&_sliceZoomer, SIGNAL(zoomFactorChanged(qreal,QPoint)), this, SLOT(zoomInSliceView(qreal,QPoint)));
	QObject::connect(&_sliceZoomer, SIGNAL(isMovedFrom(QPoint)), this, SLOT(dragInSliceView(QPoint)));

	// Évènements déclenchés par les boutons relatifs aux intervalles de coupes
	QObject::connect(_ui->_spinMinSlice, SIGNAL(valueChanged(int)), &_slicesInterval, SLOT(setMin(int)));
	QObject::connect(_ui->_buttonMinSlice, SIGNAL(clicked()), this, SLOT(setMinimumOfSlicesIntervalToCurrentSlice()));
	QObject::connect(_ui->_spinMaxSlice, SIGNAL(valueChanged(int)), &_slicesInterval, SLOT(setMax(int)));
	QObject::connect(_ui->_buttonMaxSlice, SIGNAL(clicked()), this, SLOT(setMaximumOfSlicesIntervalToCurrentSlice()));
	QObject::connect(_ui->_buttonExportToDat, SIGNAL(clicked()), this, SLOT(exportToDat()));
	QObject::connect(_ui->_buttonExportToOfs, SIGNAL(clicked()), this, SLOT(exportToOfs()));
	QObject::connect(_ui->_buttonExportHistogramToSep, SIGNAL(clicked()), this, SLOT(exportHistogramToV3D()));
	QObject::connect(_ui->_buttonExportToV3D, SIGNAL(clicked()), this, SLOT(exportToV3D()));
	QObject::connect(_ui->_buttonExportFlowToV3D, SIGNAL(clicked()), this, SLOT(exportFlowToV3D()));
	QObject::connect(_ui->_buttonExportMovementsToV3D, SIGNAL(clicked()), this, SLOT(exportMovementsToV3D()));
	QObject::connect(_ui->_buttonExportSectorsDiagramAndHistogram, SIGNAL(clicked()), this, SLOT(exportSectorDiagramAndHistogram()));

	// Raccourcis des actions du menu
	_ui->_actionOpenDicom->setShortcut(Qt::CTRL + Qt::Key_O);
	QObject::connect(_ui->_actionOpenDicom, SIGNAL(triggered()), this, SLOT(openDicom()));
	_ui->_actionCloseImage->setShortcut(Qt::CTRL + Qt::Key_W);
	QObject::connect(_ui->_actionCloseImage, SIGNAL(triggered()), this, SLOT(closeImage()));
	_ui->_actionQuit->setShortcut(Qt::CTRL + Qt::Key_Q);
	QObject::connect(_ui->_actionQuit, SIGNAL(triggered()), this, SLOT(close()));

	closeImage();
}

MainWindow::~MainWindow() {
	delete _pieChartDiagrams;
	delete _pieChart;
	delete _sliceHistogram;
	delete _sliceView;
	if ( _marrow != 0 ) delete _marrow;
	if ( _billon != 0 ) delete _billon;
	if ( _unusedBillon != 0 ) delete _unusedBillon;
}


/*******************************
 * Public fonctions
 *******************************/

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
	if ( obj == _ui->_labelSliceView ) {
		if ( event->type() == QEvent::MouseButtonPress ) {
			const QMouseEvent *mouseEvent = static_cast<const QMouseEvent*>(event);
			if ( (mouseEvent->button() == Qt::LeftButton) && _billon != 0 ) {
				const int x = mouseEvent->x()/_sliceZoomer.factor();
				const int y = mouseEvent->y()/_sliceZoomer.factor();
				const int centerX = (_marrow !=0) ? _marrow->at(_currentSlice).x:_billon->n_cols/2;
				const int centerY = (_marrow !=0) ? _marrow->at(_currentSlice).y:_billon->n_rows/2;

				const int sector = _pieChart->partOfAngle( TWO_PI-ANGLE(centerX,centerY,x,y) );

				highlightSectorHistogram(sector);

				_pieChartDiagrams->highlightCurve(sector);
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
	computeSectorsHistogramsForCurrentSliceIntervall();
	_pieChartDiagrams->clearAll();
	_ui->_plotAngularHistogram->replot();
	_ui->_polarSectorSum->replot();
	selectSectorInterval(0);
	updateComponentsValues();
	drawSlice();
}

void MainWindow::drawSlice( const int &sliceNumber ) {
	_currentSlice = sliceNumber;
	if ( _billon != 0 ) {
		_ui->_labelSliceNumber->setNum(sliceNumber);
		_pix.fill(0xff000000);
		_sliceView->drawSlice(_pix,*_billon,_marrow,sliceNumber,_intensityInterval);
		highlightSliceHistogram(sliceNumber);
		if ( _marrow != 0 ) {
			_marrow->draw(_pix,sliceNumber);
			if ( _ui->_checkMarrowAroundDiameter->isChecked() && _sliceHistogram->marrowAroundDiameter() > 0 ) {
				QPainter painter(&_pix);
				iCoord2D center = _marrow->at(sliceNumber-_marrow->interval().min());
				painter.drawEllipse(QPointF(center.x,center.y),_sliceHistogram->marrowAroundDiameter()/(2.*_billon->voxelWidth()),_sliceHistogram->marrowAroundDiameter()/(2.*_billon->voxelHeight()));
			}
		}
		const bool inDrawingArea = (_ui->_comboSelectSliceInterval->currentIndex() == 0 ? _slicesInterval.containsClosed(sliceNumber) : _sliceHistogram->branchesAreas().at(_ui->_comboSelectSliceInterval->currentIndex()-1).contains(sliceNumber));
		if ( (_pieChartDiagrams->count() != 0) && inDrawingArea ) {
			iCoord2D center(_pix.width()/2,_pix.height()/2);
			if ( _marrow != 0 &&  _marrow->interval().containsClosed(sliceNumber) ) {
				center = _marrow->at(sliceNumber-_marrow->interval().min());
			}
			_pieChartDiagrams->draw(_pix,center);
			_pieChart->draw(_pix,_currentSector, center);
		}
		if ( _sectorBillon != 0 && inDrawingArea ) {

			if ( !_ui->_checkEnableConnexComponents->isChecked() || _componentBillon == 0 ) {
				const int width = _sectorBillon->n_cols;
				const int height = _sectorBillon->n_rows;
				const int threshold = _ui->_sliderSectorThresholding->value();

				const QwtInterval &sliceInterval = _sliceHistogram->branchesAreas()[_ui->_comboSelectSliceInterval->currentIndex()-1];
				const Slice &sectorSlice = _sectorBillon->slice(sliceNumber-sliceInterval.minValue());

				QPainter painter(&_pix);
				QColor color(255,0,0,127);
				painter.setPen(color);

				int i, j;
				for ( j=0 ; j<height ; ++j ) {
					for ( i=0 ; i<width ; ++i ) {
						if ( sectorSlice.at(j,i) > threshold ) {
							painter.drawPoint(i,j);
						}
					}
				}
			}
			else {
				const int width = _componentBillon->n_cols;
				const int height = _componentBillon->n_rows;

				const QColor colors[] = { QColor(0,0,255,127), QColor(255,0,255,127), QColor(255,0,0,127), QColor(255,255,0,127), QColor(0,255,0,127) };
				const int nbColors = sizeof(colors)/sizeof(QColor);

				const QwtInterval &sliceInterval = _sliceHistogram->branchesAreas()[_ui->_comboSelectSliceInterval->currentIndex()-1];
				const Slice &sectorSlice = _componentBillon->slice(sliceNumber-sliceInterval.minValue());
				const int selectedComponents = _ui->_comboConnexComponents->currentIndex();

				QPainter painter(&_pix);
				int i, j, color;
				if ( selectedComponents ) {
					for ( j=0 ; j<height ; ++j ) {
						for ( i=0 ; i<width ; ++i ) {
							color = sectorSlice.at(j,i);
							if ( color == selectedComponents ) {
								painter.setPen(colors[color%nbColors]);
								painter.drawPoint(i,j);
							}
						}
					}
				}
				else {
					for ( j=0 ; j<height ; ++j ) {
						for ( i=0 ; i<width ; ++i ) {
							color = sectorSlice.at(j,i);
							if ( color ) {
								painter.setPen(colors[color%nbColors]);
								painter.drawPoint(i,j);
							}
						}
					}
				}
			}
		}
	}
	else {
		_ui->_labelSliceNumber->setText(tr("Aucune"));
		_pix = QImage(0,0);
	}
	_ui->_labelSliceView->setPixmap(_billon != 0 ? QPixmap::fromImage(_pix).scaled(_pix.width()*_sliceZoomer.factor(),_pix.height()*_sliceZoomer.factor(),Qt::KeepAspectRatio) : QPixmap::fromImage(_pix));
}

void MainWindow::setTypeOfView( const int &type ) {
	enabledComponents();
	_sliceView->setTypeOfView( static_cast<const SliceType::SliceType>(type) );
	switch (type) {
		case SliceType::MOVEMENT:
			_ui->_toolboxSliceParameters->setCurrentWidget(_ui->_pageMovementParameters);
			break;
		case SliceType::FLOW:
			_ui->_toolboxSliceParameters->setCurrentWidget(_ui->_pageFlowParameters);
			break;
		case SliceType::RESTRICTED_AREA:
			_ui->_toolboxSliceParameters->setCurrentWidget(_ui->_pageRestrictedAreaParameters);
			break;
		default:
			break;
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
	_sliceHistogram->detach();
	_sliceHistogram->clear();
	if ( _billon != 0 ) {
		_sliceHistogram->constructHistogram(*_billon, _marrow, _intensityInterval);
	}
	_histogramCursor.detach();
	_sliceHistogram->attach(_ui->_plotSliceHistogram);
	_histogramCursor.attach(_ui->_plotSliceHistogram);
	_ui->_plotSliceHistogram->setAxisScale(QwtPlot::xBottom,0,(_billon != 0)?_billon->n_slices:0);
	highlightSliceHistogram(_currentSlice);

	const int oldIntervalIndex = _ui->_comboSelectSliceInterval->currentIndex();
	_ui->_comboSelectSliceInterval->clear();
	_ui->_comboSelectSliceInterval->addItem(tr("Personnalisé"));
	const QVector<QwtInterval> &intervals = _sliceHistogram->branchesAreas();
	if ( !intervals.isEmpty() ) {
		for ( int i=0 ; i<intervals.size() ; ++i ) {
			const QwtInterval interval = intervals.at(i);
			_ui->_comboSelectSliceInterval->addItem(tr("Interval %1 : [ %2, %3 ] (%4 coupes)").arg(i).arg(interval.minValue()).arg(interval.maxValue()).arg(interval.width()));
		}
	}
	_ui->_comboSelectSliceInterval->setCurrentIndex(oldIntervalIndex<=intervals.size()?oldIntervalIndex:0);
}

void MainWindow::setMarrowAroundDiameter( const int &diameter ) {
	_sliceHistogram->setMarrowAroundDiameter(diameter);
	_pieChartDiagrams->setMarrowAroundDiameter(diameter);

	_ui->_spinMarrowAroundDiameter->blockSignals(true);
		_ui->_spinMarrowAroundDiameter->setValue(diameter);
	_ui->_spinMarrowAroundDiameter->blockSignals(false);

	_ui->_sliderMarrowAroundDiameter->blockSignals(true);
		_ui->_sliderMarrowAroundDiameter->setValue(diameter);
	_ui->_sliderMarrowAroundDiameter->blockSignals(false);

	drawSlice();
}

void MainWindow::setHistogramIntervalType( const int &type ) {
	_sliceHistogram->setIntervalType( static_cast<const HistogramIntervalType::HistogramIntervalType>(type) );
	_pieChartDiagrams->setIntervalType( static_cast<const HistogramIntervalType::HistogramIntervalType>(type) );
}

void MainWindow::setHistogramIntervalMinimumWidth( const int &width ) {
	_sliceHistogram->setMinimumIntervalWidth(width);

	_ui->_spinHistogramIntervalMinimumWidth->blockSignals(true);
		_ui->_spinHistogramIntervalMinimumWidth->setValue(width);
	_ui->_spinHistogramIntervalMinimumWidth->blockSignals(false);

	_ui->_sliderHistogramIntervalMinimumWidth->blockSignals(true);
		_ui->_sliderHistogramIntervalMinimumWidth->setValue(width);
	_ui->_sliderHistogramIntervalMinimumWidth->blockSignals(false);
}

void MainWindow::enableHistogramSmoothing( const bool &enable ) {
	_sliceHistogram->enableSmoothing(enable);
	_pieChartDiagrams->enableSmoothing(enable);
}

void MainWindow::setHistogramMaximumsNeighborhood( const int &neighborhood ) {
	_sliceHistogram->setMaximumsNeighborhood(neighborhood);

	_ui->_spinMaximumsNeighborhood->blockSignals(true);
		_ui->_spinMaximumsNeighborhood->setValue(neighborhood);
	_ui->_spinMaximumsNeighborhood->blockSignals(false);

	_ui->_sliderMaximumsNeighborhood->blockSignals(true);
		_ui->_sliderMaximumsNeighborhood->setValue(neighborhood);
	_ui->_sliderMaximumsNeighborhood->blockSignals(false);
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
	drawSlice();
}

void MainWindow::computeSectorsHistogramsForCurrentSliceIntervall() {
	computeSectorsHistogramForInterval(_slicesInterval);
}

void MainWindow::highlightSectorHistogram( const int &sectorIdx ) {
	_currentSector = sectorIdx;
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

void MainWindow::previousMaximumInSliceHistogram() {
	const int nbMaximums = _sliceHistogram->nbMaximums();
	_currentMaximum = nbMaximums <= 0 ? -1 : _currentMaximum < 0 ? 0 : _currentMaximum == 0 ? nbMaximums-1 : ( _currentMaximum - 1 ) % nbMaximums;
	int sliceIndex = _sliceHistogram->sliceOfIemeMaximum(_currentMaximum);
	if ( sliceIndex > -1 ) {
		_ui->_sliderSelectSlice->setValue(sliceIndex);
	}
}

void MainWindow::nextMaximumInSliceHistogram() {
	const int nbMaximums = _sliceHistogram->nbMaximums();
	_currentMaximum = nbMaximums>0 ? ( _currentMaximum + 1 ) % nbMaximums : -1;
	int sliceIndex = _sliceHistogram->sliceOfIemeMaximum(_currentMaximum);
	if ( sliceIndex > -1 ) {
		_ui->_sliderSelectSlice->setValue(sliceIndex);
	}
}

void MainWindow::zoomInSliceView( const qreal &zoomFactor, const QPoint &focalPoint ) {
	_ui->_labelSliceView->setPixmap(_billon != 0 ? QPixmap::fromImage(_pix).scaled(_pix.width()*zoomFactor,_pix.height()*zoomFactor,Qt::KeepAspectRatio) : QPixmap::fromImage(_pix));
	QScrollArea &scrollArea = *(_ui->_scrollSliceView);
	scrollArea.horizontalScrollBar()->setValue(focalPoint.x()/(qreal)(_ui->_labelSliceView->pixmap()->width())*(scrollArea.horizontalScrollBar()->maximum()*(1./0.9)));
	scrollArea.verticalScrollBar()->setValue(focalPoint.y()/(qreal)(_ui->_labelSliceView->pixmap()->height())*(scrollArea.verticalScrollBar()->maximum()*(1./0.9)));
}

void MainWindow::dragInSliceView( const QPoint &movementVector ) {
	QScrollArea &scrollArea = *(_ui->_scrollSliceView);
	if ( movementVector.x() != 0 ) scrollArea.horizontalScrollBar()->setValue(scrollArea.horizontalScrollBar()->value()-movementVector.x());
	if ( movementVector.y() != 0 ) scrollArea.verticalScrollBar()->setValue(scrollArea.verticalScrollBar()->value()-movementVector.y());
}

void MainWindow::setMovementThresholdMin( const int &threshold ) {
	_sliceView->setMovementThresholdMin(threshold);
	_sliceHistogram->setMovementThresholdMin(threshold);
	_pieChartDiagrams->setMovementsThresholdMin(threshold);

	_ui->_spinMovementThresholdMin->blockSignals(true);
		_ui->_spinMovementThresholdMin->setValue(threshold);
	_ui->_spinMovementThresholdMin->blockSignals(false);

	_ui->_sliderMovementThresholdMin->blockSignals(true);
		_ui->_sliderMovementThresholdMin->setValue(threshold);
	_ui->_sliderMovementThresholdMin->blockSignals(false);

	drawSlice();
}

void MainWindow::setMovementThresholdMax( const int &threshold ) {
	_sliceView->setMovementThresholdMax(threshold);
	_sliceHistogram->setMovementThresholdMax(threshold);
	_pieChartDiagrams->setMovementsThresholdMax(threshold);

	_ui->_spinMovementThresholdMax->blockSignals(true);
		_ui->_spinMovementThresholdMax->setValue(threshold);
	_ui->_spinMovementThresholdMax->blockSignals(false);

	_ui->_sliderMovementThresholdMax->blockSignals(true);
		_ui->_sliderMovementThresholdMax->setValue(threshold);
	_ui->_sliderMovementThresholdMax->blockSignals(false);

	drawSlice();
}

void MainWindow::enableMovementWithBackground( const bool &enable ) {
	_sliceView->enableMovementWithBackground(enable);
	drawSlice();
}

void MainWindow::useNextSliceInsteadOfCurrentSlice( const bool &enable ) {
	_sliceView->useNextSliceInsteadOfCurrentSlice(enable);
	_sliceHistogram->useNextSliceInsteadOfCurrentSlice(enable);
	_pieChartDiagrams->useNextSliceInsteadOfCurrentSlice(enable);
	drawSlice();
}

void MainWindow::flowApplied() {
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

void MainWindow::setRestrictedAreaResolution( const int &resolution ) {
	_sliceView->setRestrictedAreaResolution(resolution);
	drawSlice();
}

void MainWindow::setRestrictedAreaThreshold( const int &threshold ) {
	_sliceView->setRestrictedAreaThreshold(threshold);
	drawSlice();
}

void MainWindow::enableRestrictedAreaCircle( const bool &enable )  {
	_sliceView->enableRestrictedAreaCircle(enable);
	drawSlice();
}

void MainWindow::setRestrictedAreaBeginRadius( const int &radius ) {
	_sliceView->setRestrictedAreaBeginRadius( radius );

	_ui->_sliderRestrictedAreaBeginRadius->blockSignals(true);
		_ui->_sliderRestrictedAreaBeginRadius->setValue(radius);
	_ui->_sliderRestrictedAreaBeginRadius->blockSignals(false);

	_ui->_spinRestrictedAreaBeginRadius->blockSignals(true);
		_ui->_spinRestrictedAreaBeginRadius->setValue(radius);
	_ui->_spinRestrictedAreaBeginRadius->blockSignals(false);

	drawSlice();
}

void MainWindow::computeRestrictedBillon() {
	if ( _billon != 0 ) {
		Billon *originalBillon = 0;
		Billon *restrictedBillon = 0;
		if ( _unusedBillon != 0 ) {
			if ( _isUsedOriginalBillon ) {
				restrictedBillon = _unusedBillon;
				originalBillon = _billon;
			}
			else {
				restrictedBillon = _billon;
				originalBillon = _unusedBillon;
			}
		}
		else {
			originalBillon = _billon;
		}
		Billon * newBillon = originalBillon->restrictToArea( _ui->_spinRestrictedAreaResolution->value(), _ui->_spinRestrictedAreaThreshold->value(), _marrow );
		if ( newBillon != 0 ) {
			if ( restrictedBillon != 0 ) delete restrictedBillon;
			restrictedBillon = 0;
			if ( _isUsedOriginalBillon ) {
				_unusedBillon = newBillon;
			}
			else {
				_billon = newBillon;
				_ui->_spansliderSliceThreshold->setMinimum(newBillon->minValue());
				_ui->_spansliderSliceThreshold->setMaximum(newBillon->maxValue());
			}
			drawSlice();
		}
	}
}

void MainWindow::changeBillonUsed() {
	if ( _billon != 0 && _unusedBillon != 0 ) {
		_isUsedOriginalBillon = !_isUsedOriginalBillon;
		Billon *temporaryBillon = _billon;
		_billon = _unusedBillon;
		_unusedBillon = temporaryBillon;
		QPalette palette;
		if ( _isUsedOriginalBillon ) {
			palette.setBrush(QPalette::Active,QPalette::Window,QBrush(QColor(170,255,100)));
			_ui->_labelBillonUsed->setPalette(palette);
			_ui->_labelBillonUsed->setText(tr("Actuellement : Billon original"));
		}
		else {
			palette.setBrush(QPalette::Active,QPalette::Window,QBrush(QColor(255,170,100)));
			_ui->_labelBillonUsed->setPalette(palette);
			_ui->_labelBillonUsed->setText(tr("Actuellement : Billon restreint"));
		}
		_ui->_spansliderSliceThreshold->setMinimum(_billon->minValue());
		_ui->_spansliderSliceThreshold->setMaximum(_billon->maxValue());
		drawSlice();
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

void MainWindow::exportHistogramToV3D() {
	if ( _sliceHistogram != 0 ) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter l'histo' .sep"), "output.sep", tr("Fichiers séquences de point euclidiens (*.sep);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() ) {
			HistoExport::process( *_sliceHistogram, _slicesInterval, fileName );
		}
	}
}

void MainWindow::exportToV3D() {
	if ( _billon != 0 ) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .v3d"), "output.v3d", tr("Fichiers de données (*.v3d);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() ) {
			V3DExport::process( *_billon, _marrow, fileName, _slicesInterval, _ui->_spinExportThreshold->value() );
		}
	}
}

void MainWindow::exportFlowToV3D() {
	if ( _billon != 0 ) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .v3d"), "output_flow.v3d", tr("Fichiers de données (*.v3d);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() ) {
			const int width = _billon->n_cols;
			const int height = _billon->n_rows;
			const int depth = _slicesInterval.count();
			Billon billonFlow( width, height, _slicesInterval.count() );
			billonFlow.setMinValue(_billon->minValue());
			billonFlow.setMaxValue(_billon->maxValue());
			billonFlow.setVoxelSize(_billon->voxelWidth(),_billon->voxelHeight(),_billon->voxelDepth());
			VectorsField *field = 0;
			int i,j,k;

			for ( k=0 ; k<depth ; ++k ) {
				field = OpticalFlow::compute(*_billon,k+_slicesInterval.min(),_ui->_spinFlowAlpha->value(),_ui->_spinFlowEpsilon->value(),_ui->_spinFlowMaximumIterations->value());
				if ( field != 0 ) {
					Slice &slice = billonFlow.slice(k);
					for ( j=0 ; j<height ; ++j ) {
						for ( i=0 ; i<width ; ++i ) {
							slice.at(j,i) = qSqrt( qPow((*field)[j][i].x(),2) + qPow((*field)[j][i].y(),2) )*20.;
						}
					}
					delete field;
					field = 0;
				}
			}

			V3DExport::process( billonFlow, _marrow, fileName, SlicesInterval(0,depth-1), _ui->_spinExportThreshold->value() );
		}
	}
}

void MainWindow::exportMovementsToV3D() {
	if ( _billon != 0 ) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .v3d"), "output_diag.v3d", tr("Fichiers de données (*.v3d);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() ) {
			const int width = _billon->n_cols;
			const int height = _billon->n_rows;
			const int depth = _slicesInterval.count();
			const int minValue = _intensityInterval.min();
			const int maxValue = _intensityInterval.max();
			const int thresholdMin = _ui->_sliderMovementThresholdMin->value();
			const int thresholdMax = _ui->_sliderMovementThresholdMax->value();

			Billon billonDiag( *_billon );
			int i,j,k, pixAbsDiff;

			for ( k=_slicesInterval.min()+1 ; k<_slicesInterval.max() ; ++k ) {
				const Slice &previousSlice = _billon->slice(k-1);
				const Slice &toCompareSlice = _billon->slice(_ui->_checkUseNextSlice && k < _slicesInterval.max()-1 ? k+1 : k );
				Slice &sliceDiag = billonDiag.slice(k-_slicesInterval.min()-1);
				for ( j=0 ; j<height ; ++j ) {
					for ( i=0 ; i<width ; ++i ) {
						pixAbsDiff = qAbs(qBound(minValue,previousSlice.at(j,i),maxValue) - qBound(minValue,toCompareSlice.at(j,i),maxValue));
						if ( (pixAbsDiff <= thresholdMin) && (pixAbsDiff >= thresholdMax) ) sliceDiag.at(j,i) = minValue;
					}
				}
			}

			V3DExport::process( billonDiag, _marrow, fileName, SlicesInterval(0,depth-2), _ui->_spinExportThreshold->value() );
		}
	}
}

void MainWindow::selectSliceInterval( const int &index ) {
	selectSectorInterval(0);

	if ( index > 0 && index <= _sliceHistogram->branchesAreas().size() ) {
		const QwtInterval &interval = _sliceHistogram->branchesAreas()[index-1];
		computeSectorsHistogramForInterval(SlicesInterval(interval.minValue(),interval.maxValue()));
		_ui->_sliderSelectSlice->setValue(_sliceHistogram->sliceOfIemeInterval(index-1));
	}
	else {
		computeSectorsHistogramForInterval(_slicesInterval);
		_ui->_sliderSelectSlice->setValue((_slicesInterval.min()+_slicesInterval.max())/2);
	}

	_ui->_comboSelectSectorInterval->clear();
	_ui->_comboSelectSectorInterval->addItem(tr("Aucun"));
	const QVector<QwtInterval> &intervals = _pieChartDiagrams->branchesSectors();
	if ( !intervals.isEmpty() ) {
		qreal rightAngle, leftAngle;
		for ( int i=0 ; i<intervals.size() ; ++i ) {
			const QwtInterval interval = intervals.at(i);
			rightAngle = _pieChart->sector(interval.minValue()).rightAngle()*RAD_TO_DEG_FACT;
			leftAngle = _pieChart->sector(interval.maxValue()).leftAngle()*RAD_TO_DEG_FACT;
			_ui->_comboSelectSectorInterval->addItem(tr("Secteur %1 : [ %2, %3 ] (%4 degres)").arg(i).arg(rightAngle).arg(leftAngle).arg(interval.isValid()?leftAngle-rightAngle:leftAngle-rightAngle+360.));
		}
	}
}

void MainWindow::selectCurrentSliceInterval() {
	selectSliceInterval(_ui->_comboSelectSliceInterval->currentIndex());
}

void MainWindow::selectSectorInterval( const int &index ) {
	if ( _sectorBillon != 0 ) {
		delete _sectorBillon;
		_sectorBillon = 0;
	}
	if ( _componentBillon != 0 ) {
		delete _componentBillon;
		_componentBillon = 0;
	}
	_ui->_comboConnexComponents->clear();
	_ui->_comboConnexComponents->addItem(tr("Toutes"));
	if ( index > 0 && index <= _pieChartDiagrams->branchesSectors().size() ) {
		const QwtInterval &sectorInterval = _pieChartDiagrams->branchesSectors()[_ui->_comboSelectSectorInterval->currentIndex()-1];
		const QwtInterval &sliceInterval = _sliceHistogram->branchesAreas()[_ui->_comboSelectSliceInterval->currentIndex()-1];
		const int firstSlice = sliceInterval.minValue();
		const int lastSlice = sliceInterval.maxValue()+1;
		const int width = _billon->n_cols;
		const int height = _billon->n_rows;
		const int threshold = _ui->_sliderSectorThresholding->value();

		_sectorBillon = new Billon(_billon->n_cols,_billon->n_rows,sliceInterval.width()+1);
		_sectorBillon->setMinValue(threshold);
		_sectorBillon->setMaxValue(500);
		_sectorBillon->setVoxelSize(_billon->voxelWidth(),_billon->voxelHeight(),_billon->voxelDepth());
		_sectorBillon->fill(threshold);

		int i, j, k;
		if ( sectorInterval.isValid() ) {
			for ( k=firstSlice ; k<lastSlice ; ++k ) {
				const Slice &originalSlice = _billon->slice(k);
				Slice &sectorSlice = _sectorBillon->slice(k-firstSlice);
				const int marrowX = _marrow->at(k).x;
				const int marrowY = _marrow->at(k).y;
				for ( j=0 ; j<height ; ++j ) {
					for ( i=0 ; i<width ; ++i ) {
						if ( originalSlice.at(j,i) > threshold && originalSlice.at(j,i) < 500 && sectorInterval.contains(_pieChart->partOfAngle(TWO_PI-ANGLE(marrowX, marrowY, i, j))) ) {
							sectorSlice.at(j,i) = originalSlice.at(j,i);
						}
					}
				}
			}
		}
		else {
			const QwtInterval sectorIntervalInverted = sectorInterval.inverted();
			for ( k=firstSlice ; k<lastSlice ; ++k ) {
				const Slice &originalSlice = _billon->slice(k);
				Slice &sectorSlice = _sectorBillon->slice(k-firstSlice);
				const int marrowX = _marrow->at(k).x;
				const int marrowY = _marrow->at(k).y;
				for ( j=0 ; j<height ; ++j ) {
					for ( i=0 ; i<width ; ++i ) {
						if ( originalSlice.at(j,i) > threshold && originalSlice.at(j,i) < 500 && !sectorIntervalInverted.contains(_pieChart->partOfAngle(TWO_PI-ANGLE(marrowX, marrowY, i, j))) ) {
							sectorSlice.at(j,i) = originalSlice.at(j,i);
						}
					}
				}
			}
		}

		if ( _ui->_checkEnableConnexComponents->isChecked() ) {
			_componentBillon = ConnexComponentExtractor::extractConnexComponent(*_sectorBillon,_ui->_sliderMinimalSizeOfConnexComponents->value(),threshold);
			const int nbComponents = _componentBillon->maxValue()+1;
			for ( i=1 ; i<nbComponents ; ++i ) {
				_ui->_comboConnexComponents->addItem(tr("Composante %1").arg(i));
			}
		}
	}
	drawSlice();
}

void MainWindow::selectCurrentSectorInterval() {
	selectSectorInterval(_ui->_comboSelectSectorInterval->currentIndex());
}

void MainWindow::exportSectorToPgm3D() {
	if ( _sectorBillon != 0 ) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter le secteur en .pgm3d"), "output.pgm3d", tr("Fichiers de données (*.pgm3d);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() ) {
			Pgm3dExport::process( *_sectorBillon, fileName );
		}
	}
}

void MainWindow::exportConnexComponentToPgm3D() {
	if ( _componentBillon != 0 ) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter la composante connexe en .pgm3d"), "output.pgm3d", tr("Fichiers de données (*.pgm3d);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() ) {
			Pgm3dExport::process( *_componentBillon, fileName );
		}
	}
}

void MainWindow::exportSliceHistogram() {
	QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter l'histogramme de coupes"), "output.pdf", tr("Fichiers PDF (*.pdf);;Fichiers PS (*.ps);;Fichiers PNG (*.png);;Fichiers SVG (*.svg);;Tous les fichiers (*.*)"));
	if ( !fileName.isEmpty() ) {
		QwtPlotRenderer histoRenderer;
		histoRenderer.renderDocument(_ui->_plotSliceHistogram,fileName,QSize(297,210),100);
	}
}

void MainWindow::exportSectorDiagramAndHistogram() {
	QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter le diagramme et l'histogramme."), "output.pdf", tr("Fichiers PDF (*.pdf);;Fichiers PS (*.ps);;Fichiers PNG (*.png);;Fichiers SVG (*.svg);;Tous les fichiers (*.*)"));
	if ( !fileName.isEmpty() ) {

		QString chemin = fileName.section(QDir::separator(),0,-2)+QDir::separator();
		QString name = fileName.section(QDir::separator(),-1);

		QwtPlotRenderer histoRenderer;
		histoRenderer.renderDocument(_ui->_plotAngularHistogram, chemin+"histo_"+name, QSize(297,210), 100);
		QwtPolarRenderer diagramRenderer;
		diagramRenderer.renderDocument(_ui->_polarSectorSum, chemin+"diag_"+name, QSize(297,210), 100);
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
	if ( _unusedBillon != 0 ) {
		delete _unusedBillon;
		_unusedBillon = 0;
	}
	if ( !folderName.isEmpty() ) {
		_billon = DicomReader::read(folderName);
	}
	if ( _billon != 0 ) {
		_pix = QImage(_billon->n_cols, _billon->n_rows,QImage::Format_ARGB32);
		_intensityInterval.setBounds(_billon->minValue(),_billon->maxValue());
	}
	else {
		_pix = QImage(0,0,QImage::Format_ARGB32);
		_intensityInterval.setBounds(1,0);
	}
	_isUsedOriginalBillon = true;
	QPalette palette;
	palette.setBrush(QPalette::Active,QPalette::Window,QBrush(QColor(170,255,100)));
	_ui->_labelBillonUsed->setPalette(palette);
	_ui->_labelBillonUsed->setText(tr("Actuellement : Billon original"));
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
	_ui->_spansliderSliceThreshold->setLowerValue(-700);
	_ui->_spansliderSliceThreshold->setMaximum(maxValue);
	_ui->_spansliderSliceThreshold->setUpperValue(500);

	_ui->_spinMinThreshold->setMinimum(minValue);
	_ui->_spinMinThreshold->setMaximum(maxValue);
	_ui->_spinMinThreshold->setValue(-700);

	_ui->_spinMaxThreshold->setMinimum(minValue);
	_ui->_spinMaxThreshold->setMaximum(maxValue);
	_ui->_spinMaxThreshold->setValue(500);

	_ui->_spinMinSlice->setMinimum(0);
	_ui->_spinMinSlice->setMaximum(nbSlices-1);
	_ui->_spinMinSlice->setValue(0);

	_ui->_spinMaxSlice->setMinimum(0);
	_ui->_spinMaxSlice->setMaximum(nbSlices-1);
	_ui->_spinMaxSlice->setValue(nbSlices-1);

	_ui->_sliderSelectSlice->setValue(0);
	_ui->_sliderSelectSlice->setRange(0,nbSlices-1);

	enabledComponents();
}

void MainWindow::enabledComponents() {
	const int sliceType = _ui->_comboSliceType->currentIndex();
	const bool enable = (_billon != 0) && ( sliceType == SliceType::CURRENT || sliceType == SliceType::MOVEMENT || sliceType == SliceType::FLOW || sliceType == SliceType::RESTRICTED_AREA );
	_ui->_sliderSelectSlice->setEnabled(enable);
	_ui->_spansliderSliceThreshold->setEnabled(enable);
	_ui->_buttonComputeMarrow->setEnabled(enable);
	_ui->_buttonUpdateSliceHistogram->setEnabled(enable);
	_ui->_buttonMaxSlice->setEnabled(enable);
	_ui->_buttonMinSlice->setEnabled(enable);
	_ui->_buttonExportToDat->setEnabled(enable);
	_ui->_buttonExportToOfs->setEnabled(enable);
	_ui->_buttonNextMaximum->setEnabled(enable);
	_ui->_buttonPreviousMaximum->setEnabled(enable);
}


void MainWindow::computeSectorsHistogramForInterval( const SlicesInterval &interval ) {
	_pieChart->setOrientation(_ui->_spinSectorsOrientation->value()*DEG_TO_RAD_FACT);
	_pieChart->setSectorsNumber(_ui->_spinSectorsNumber->value());

	if ( _billon != 0 ) _pieChartDiagrams->compute( *_billon, _marrow, *_pieChart, interval, _intensityInterval );

	_ui->_plotAngularHistogram->replot();
	_ui->_polarSectorSum->replot();
	highlightSectorHistogram(0);
}
