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
#include "inc/opticalflow.h"
#include "inc/connexcomponentextractor.h"
#include "inc/pgm3dexport.h"
#include "inc/contourcurve.h"

#include <QFileDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QMessageBox>
#include <QInputDialog>
#include <qwt_plot_renderer.h>
#include <qwt_polar_renderer.h>

#include <qwt_polar_grid.h>
#include <qwt_round_scale_draw.h>

MainWindow::MainWindow( QWidget *parent ) : QMainWindow(parent), _ui(new Ui::MainWindow), _billon(0), _sectorBillon(0), _componentBillon(0), _marrow(0),
	_sliceView(new SliceView()), _sliceHistogram(new SliceHistogram()), _pieChart(new PieChart(1)), _pieChartDiagrams(new PieChartDiagrams()), _contourCurve(new ContourCurve()),
	_currentSlice(0), _currentMaximum(0)
{
	_ui->setupUi(this);
	setCorner(Qt::TopLeftCorner,Qt::LeftDockWidgetArea);
	setCorner(Qt::TopRightCorner,Qt::RightDockWidgetArea);
	setCorner(Qt::BottomLeftCorner,Qt::LeftDockWidgetArea);
	setCorner(Qt::BottomRightCorner,Qt::RightDockWidgetArea);
	setWindowTitle("TKDetection");

	// Paramétrisation des composant graphiques
	_ui->_labelSliceView->installEventFilter(this);
	_ui->_labelSliceView->installEventFilter(&_sliceZoomer);

	_ui->_plotSliceHistogram->enableAxis(QwtPlot::yLeft,false);
	_ui->_plotAngularHistogram->enableAxis(QwtPlot::yLeft,false);

	_histogramCursor.attach(_ui->_plotSliceHistogram);
	_histogramCursor.setBrush(Qt::red);

	_ui->_comboSliceType->insertItem(SliceType::CURRENT,tr("Coupe originale"));
	_ui->_comboSliceType->insertItem(SliceType::MOVEMENT,tr("Coupe de mouvements"));
	_ui->_comboSliceType->insertItem(SliceType::EDGE_DETECTION,tr("Coupe de détection de contours"));
	_ui->_comboSliceType->insertItem(SliceType::FLOW,tr("Coupe de flots optiques"));
	_ui->_comboSliceType->insertItem(SliceType::RESTRICTED_AREA,tr("Coupe de zone réduite"));
	//_ui->_comboSliceType->insertItem(SliceType::AVERAGE,tr("Coupe moyenne"));
	//_ui->_comboSliceType->insertItem(SliceType::MEDIAN,tr("Coupe médiane"));
	_ui->_comboSliceType->setCurrentIndex(SliceType::CURRENT);

	_ui->_comboEdgeDetectionType->insertItem(EdgeDetectionType::SOBEL,tr("Sobel"));
	_ui->_comboEdgeDetectionType->insertItem(EdgeDetectionType::LAPLACIAN,tr("Laplacian"));
	_ui->_comboEdgeDetectionType->insertItem(EdgeDetectionType::CANNY,tr("Canny"));

	_ui->_spinFlowAlpha->setValue(_sliceView->flowAlpha());
	_ui->_spinFlowEpsilon->setValue(_sliceView->flowEpsilon());
	_ui->_spinFlowMaximumIterations->setValue(_sliceView->flowMaximumIterations());

	_ui->_plotAngularHistogram->enableAxis(QwtPlot::yLeft,false);

	_ui->_polarSectorSum->setScale( QwtPolar::Azimuth, TWO_PI, 0.0 );
	_pieChartDiagrams->attach(_ui->_polarSectorSum);
	_pieChartDiagrams->attach(_ui->_plotAngularHistogram);

	QwtPolarGrid *grid = new QwtPolarGrid();
	grid->showAxis(QwtPolar::AxisRight,false);
	grid->showAxis(QwtPolar::AxisBottom,false);
	grid->setMajorGridPen(QPen(Qt::lightGray));
	grid->attach(_ui->_polarSectorSum);

	_histogramDistanceMarrowToNearestPoint.attach(_ui->_plotDistanceMarrowToNearestPoint);
	_histogramDistanceMarrowToNearestPoint.setBrush(Qt::blue);
	_histogramDistanceMarrowToNearestPointCursor.setBrush(Qt::red);

	/**** Mise en place de la communication MVC ****/

	// Évènements déclenchés par le slider de n° de coupe
	QObject::connect(_ui->_sliderSelectSlice, SIGNAL(valueChanged(int)), this, SLOT(setSlice(int)));

	// Évènements déclenchés par les boutons de sélection de la vue
	QObject::connect(_ui->_comboSliceType, SIGNAL(currentIndexChanged(int)), this, SLOT(setTypeOfView(int)));
	QObject::connect(_ui->_sliderMovementThresholdInterval, SIGNAL(lowerValueChanged(int)), _ui->_spinMovementThresholdMin, SLOT(setValue(int)));
	QObject::connect(_ui->_spinMovementThresholdMin, SIGNAL(valueChanged(int)), _ui->_sliderMovementThresholdInterval, SLOT(setLowerValue(int)));
	QObject::connect(_ui->_spinMovementThresholdMin, SIGNAL(valueChanged(int)), this, SLOT(drawSlice()));
	QObject::connect(_ui->_sliderMovementThresholdInterval, SIGNAL(upperValueChanged(int)), _ui->_spinMovementThresholdMax, SLOT(setValue(int)));
	QObject::connect(_ui->_spinMovementThresholdMax, SIGNAL(valueChanged(int)), _ui->_sliderMovementThresholdInterval, SLOT(setUpperValue(int)));
	QObject::connect(_ui->_spinMovementThresholdMax, SIGNAL(valueChanged(int)), this, SLOT(drawSlice()));

	QObject::connect(_ui->_buttonFlowApplied, SIGNAL(clicked()), this, SLOT(flowApplied()));
	QObject::connect(_ui->_spinRestrictedAreaResolution, SIGNAL(valueChanged(int)), this, SLOT(setRestrictedAreaResolution(int)));
	QObject::connect(_ui->_spinRestrictedAreaThreshold, SIGNAL(valueChanged(int)), this, SLOT(setRestrictedAreaThreshold(int)));
	QObject::connect(_ui->_sliderRestrictedAreaBeginRadius, SIGNAL(valueChanged(int)), this, SLOT(setRestrictedAreaBeginRadius(int)));
	QObject::connect(_ui->_spinRestrictedAreaBeginRadius, SIGNAL(valueChanged(int)), this, SLOT(setRestrictedAreaBeginRadius(int)));
	QObject::connect(_ui->_comboEdgeDetectionType, SIGNAL(currentIndexChanged(int)), this, SLOT(setEdgeDetectionType(int)));
	QObject::connect(_ui->_spinCannyRadiusOfGaussianMask, SIGNAL(valueChanged(int)), this, SLOT(setCannyRadiusOfGaussianMask(int)));
	QObject::connect(_ui->_spinCannySigmaOfGaussianMask, SIGNAL(valueChanged(double)), this, SLOT(setCannySigmaOfGaussianMask(double)));
	QObject::connect(_ui->_spinCannyMinimumGradient, SIGNAL(valueChanged(int)), this, SLOT(setCannyMinimumGradient(int)));
	QObject::connect(_ui->_spinCannyMinimumDeviation, SIGNAL(valueChanged(double)), this, SLOT(setCannyMinimumDeviation(double)));

	// Évènements déclenchés par le slider de seuillage
	QObject::connect(_ui->_spansliderIntensityThreshold, SIGNAL(lowerValueChanged(int)), _ui->_spinMinIntensity, SLOT(setValue(int)));
	QObject::connect(_ui->_spinMinIntensity, SIGNAL(valueChanged(int)), _ui->_spansliderIntensityThreshold, SLOT(setLowerValue(int)));
	QObject::connect(_ui->_spansliderIntensityThreshold, SIGNAL(upperValueChanged(int)), _ui->_spinMaxIntensity , SLOT(setValue(int)));
	QObject::connect(_ui->_spinMaxIntensity, SIGNAL(valueChanged(int)), _ui->_spansliderIntensityThreshold, SLOT(setUpperValue(int)));
	QObject::connect(_ui->_spinMinIntensity, SIGNAL(valueChanged(int)), this, SLOT(drawSlice()));
	QObject::connect(_ui->_spinMaxIntensity, SIGNAL(valueChanged(int)), this, SLOT(drawSlice()));

	// Évènements déclenchés par les boutons relatifs à l'histogramme de cumul des intensités
	QObject::connect(_ui->_buttonUpdateSliceHistogram, SIGNAL(clicked()), this, SLOT(updateSliceHistogram()));
	QObject::connect(_ui->_buttonPreviousMaximum, SIGNAL(clicked()), this, SLOT(previousMaximumInSliceHistogram()));
	QObject::connect(_ui->_buttonNextMaximum, SIGNAL(clicked()), this, SLOT(nextMaximumInSliceHistogram()));
	QObject::connect(_ui->_checkRadiusAroundPith, SIGNAL(clicked()), this, SLOT(drawSlice()));
	QObject::connect(_ui->_sliderSmoothingRadiusOfHistogram, SIGNAL(valueChanged(int)), _ui->_spinSmoothingRadiusOfHistogram, SLOT(setValue(int)));
	QObject::connect(_ui->_spinSmoothingRadiusOfHistogram, SIGNAL(valueChanged(int)), _ui->_sliderSmoothingRadiusOfHistogram, SLOT(setValue(int)));
	QObject::connect(_ui->_sliderMinimumHeightofMaximum, SIGNAL(valueChanged(int)), _ui->_spinMinimumHeightofMaximum, SLOT(setValue(int)));
	QObject::connect(_ui->_spinMinimumHeightofMaximum, SIGNAL(valueChanged(int)), _ui->_sliderMinimumHeightofMaximum, SLOT(setValue(int)));
	QObject::connect(_ui->_sliderMaximumsNeighborhood, SIGNAL(valueChanged(int)), _ui->_spinMaximumsNeighborhood, SLOT(setValue(int)));
	QObject::connect(_ui->_spinMaximumsNeighborhood, SIGNAL(valueChanged(int)), _ui->_sliderMaximumsNeighborhood, SLOT(setValue(int)));
	QObject::connect(_ui->_sliderDerivativePercentage, SIGNAL(valueChanged(int)), _ui->_spinDerivativePercentage, SLOT(setValue(int)));
	QObject::connect(_ui->_spinDerivativePercentage, SIGNAL(valueChanged(int)), _ui->_sliderDerivativePercentage, SLOT(setValue(int)));
	QObject::connect(_ui->_sliderHistogramIntervalMinimumWidth, SIGNAL(valueChanged(int)), _ui->_spinHistogramIntervalMinimumWidth, SLOT(setValue(int)));
	QObject::connect(_ui->_spinHistogramIntervalMinimumWidth, SIGNAL(valueChanged(int)), _ui->_sliderHistogramIntervalMinimumWidth, SLOT(setValue(int)));
	QObject::connect(_ui->_sliderBorderPercentageToCut, SIGNAL(valueChanged(int)), _ui->_spinBorderPercentageToCut, SLOT(setValue(int)));
	QObject::connect(_ui->_spinBorderPercentageToCut, SIGNAL(valueChanged(int)), _ui->_sliderBorderPercentageToCut, SLOT(setValue(int)));
	QObject::connect(_ui->_buttonExportSliceHistogram, SIGNAL(clicked()), this, SLOT(exportSliceHistogram()));
	QObject::connect(_ui->_buttonExportKnotIntervalHistogram, SIGNAL(clicked()), this, SLOT(exportKnotIntervalHistogram()));

	// Évènements déclenchés par les bouton associès à la moelle
	QObject::connect(_ui->_buttonComputeMarrow, SIGNAL(clicked()), this, SLOT(updateMarrow()));

	// Évènements déclenchés par les bouton associès aux histogrammes de secteurs
	QObject::connect(_ui->_comboSelectSliceInterval, SIGNAL(currentIndexChanged(int)), this, SLOT(selectSliceInterval(int)));
	QObject::connect(_ui->_buttonSelectSliceIntervalUpdate, SIGNAL(clicked()), this, SLOT(selectCurrentSliceInterval()));
	QObject::connect(_ui->_comboSelectSectorInterval, SIGNAL(currentIndexChanged(int)), this, SLOT(selectSectorInterval(int)));
	QObject::connect(_ui->_buttonSelectSectorIntervalUpdate, SIGNAL(clicked()), this, SLOT(selectCurrentSectorInterval()));
	QObject::connect(_ui->_sliderSectorThresholding, SIGNAL(valueChanged(int)), _ui->_spinSectorThresholding, SLOT(setValue(int)));
	QObject::connect(_ui->_spinSectorThresholding, SIGNAL(valueChanged(int)), _ui->_sliderSectorThresholding, SLOT(setValue(int)));
	QObject::connect(_ui->_sliderMinimalSizeOf3DConnexComponents, SIGNAL(valueChanged(int)), _ui->_spinMinimalSizeOf3DConnexComponents, SLOT(setValue(int)));
	QObject::connect(_ui->_spinMinimalSizeOf3DConnexComponents, SIGNAL(valueChanged(int)), _ui->_sliderMinimalSizeOf3DConnexComponents, SLOT(setValue(int)));
	QObject::connect(_ui->_sliderMinimalSizeOf2DConnexComponents, SIGNAL(valueChanged(int)), _ui->_spinMinimalSizeOf2DConnexComponents, SLOT(setValue(int)));
	QObject::connect(_ui->_spinMinimalSizeOf2DConnexComponents, SIGNAL(valueChanged(int)), _ui->_sliderMinimalSizeOf2DConnexComponents, SLOT(setValue(int)));
	QObject::connect(_ui->_comboConnexComponents, SIGNAL(currentIndexChanged(int)), this, SLOT(drawSlice()));
	QObject::connect(_ui->_buttonExportSectorToPgm3D, SIGNAL(clicked()), this, SLOT(exportSectorToPgm3D()));
	QObject::connect(_ui->_buttonExportConnexComponentToPgm3D, SIGNAL(clicked()), this, SLOT(exportConnexComponentToPgm3D()));
	QObject::connect(_ui->_buttonExportSectorToOfs, SIGNAL(clicked()), this, SLOT(exportSectorToOfs()));
	QObject::connect(_ui->_buttonExportAllSectorsInAllIntervalsToOfs, SIGNAL(clicked()), this, SLOT(exportAllSectorInAllIntervalsToOfs()));
	QObject::connect(_ui->_buttonContours, SIGNAL(clicked()), this, SLOT(exportContours()));
	QObject::connect(_ui->_spinContourSmoothingRadius, SIGNAL(valueChanged(int)), this, SLOT(drawSlice()));

	// Évènements déclenchés par la souris sur le visualiseur de coupes
	QObject::connect(&_sliceZoomer, SIGNAL(zoomFactorChanged(qreal,QPoint)), this, SLOT(zoomInSliceView(qreal,QPoint)));
	QObject::connect(&_sliceZoomer, SIGNAL(isMovedFrom(QPoint)), this, SLOT(dragInSliceView(QPoint)));

	// Évènements déclenchés par les boutons relatifs aux intervalles de coupes
	QObject::connect(_ui->_spinMinSlice, SIGNAL(valueChanged(int)), _ui->_spanSliderSelectInterval, SLOT(setLowerValue(int)));
	QObject::connect(_ui->_spanSliderSelectInterval, SIGNAL(lowerValueChanged(int)), _ui->_spinMinSlice, SLOT(setValue(int)));
	QObject::connect(_ui->_buttonMinSlice, SIGNAL(clicked()), this, SLOT(setMinimumOfSliceIntervalToCurrentSlice()));
	QObject::connect(_ui->_spinMaxSlice, SIGNAL(valueChanged(int)), _ui->_spanSliderSelectInterval, SLOT(setUpperValue(int)));
	QObject::connect(_ui->_spanSliderSelectInterval, SIGNAL(upperValueChanged(int)), _ui->_spinMaxSlice, SLOT(setValue(int)));
	QObject::connect(_ui->_buttonMaxSlice, SIGNAL(clicked()), this, SLOT(setMaximumOfSliceIntervalToCurrentSlice()));
	QObject::connect(_ui->_buttonExportToDat, SIGNAL(clicked()), this, SLOT(exportToDat()));
	QObject::connect(_ui->_buttonExportToOfsAll, SIGNAL(clicked()), this, SLOT(exportToOfsAll()));
	QObject::connect(_ui->_buttonExportToOfs, SIGNAL(clicked()), this, SLOT(exportToOfs()));
	QObject::connect(_ui->_buttonExportHistogramToSep, SIGNAL(clicked()), this, SLOT(exportHistogramToSep()));
	QObject::connect(_ui->_buttonExportToV3D, SIGNAL(clicked()), this, SLOT(exportToV3D()));
	QObject::connect(_ui->_buttonExportFlowToV3D, SIGNAL(clicked()), this, SLOT(exportFlowToV3D()));
	QObject::connect(_ui->_buttonExportMovementsToV3D, SIGNAL(clicked()), this, SLOT(exportMovementsToV3D()));
	QObject::connect(_ui->_buttonExportToOfsRestricted, SIGNAL(clicked()), this, SLOT(exportToOfsRestricted()));
	QObject::connect(_ui->_buttonExportSectorsDiagramAndHistogram, SIGNAL(clicked()), this, SLOT(exportSectorDiagramAndHistogram()));
	QObject::connect(_ui->_spinBlurredSegmentsThickness, SIGNAL(valueChanged(int)), this, SLOT(drawSlice()));
	QObject::connect(_ui->_buttonExportContourComponentToPgm3D, SIGNAL(clicked()), this, SLOT(exportContourComponentToPgm3D()));
	QObject::connect(_ui->_buttonExportContourComponentAllInIntervalSDP, SIGNAL(clicked()), this, SLOT(exportAllContourComponentOfVoxels()));
	QObject::connect(_ui->_buttonExportContourComponentAllIntervalsSDP, SIGNAL(clicked()), this, SLOT(exportAllContourComponentOfVoxelsAllIntervals()));
	QObject::connect(_ui->_buttonExportContourComponentAllIntervalsSDPOldMethod, SIGNAL(clicked()), this, SLOT(exportAllContourComponentOfVoxelsAllIntervalsOldMethod()));

	// Raccourcis des actions du menu
	_ui->_actionOpenDicom->setShortcut(Qt::CTRL + Qt::Key_O);
	QObject::connect(_ui->_actionOpenDicom, SIGNAL(triggered()), this, SLOT(openDicom()));
	_ui->_actionCloseImage->setShortcut(Qt::CTRL + Qt::Key_W);
	QObject::connect(_ui->_actionCloseImage, SIGNAL(triggered()), this, SLOT(closeImage()));
	_ui->_actionQuit->setShortcut(Qt::CTRL + Qt::Key_Q);
	QObject::connect(_ui->_actionQuit, SIGNAL(triggered()), this, SLOT(close()));

	closeImage();
	initComponentsValues();
}

MainWindow::~MainWindow() {
	delete _contourCurve;
	delete _pieChartDiagrams;
	delete _pieChart;
	delete _sliceHistogram;
	delete _sliceView;
	if ( _marrow != 0 ) delete _marrow;
	if ( _billon != 0 ) delete _billon;
}


/*******************************
 * Public fonctions
 *******************************/

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
	if ( obj == _ui->_labelSliceView )
	{
		if ( event->type() == QEvent::MouseButtonPress )
		{
			const QMouseEvent *mouseEvent = static_cast<const QMouseEvent*>(event);
			if ( (mouseEvent->button() == Qt::LeftButton) && _billon != 0 ) {
				iCoord2D mousePos(mouseEvent->x(),mouseEvent->y());
				mousePos /= _sliceZoomer.factor();
				iCoord2D center = _marrow != 0 ? _marrow->at(_currentSlice) : iCoord2D(_billon->n_cols/2,_billon->n_rows/2);
				const int sector = _pieChart->sectorIndexOfAngle( center.angle(mousePos) );
				highlightSectorHistogram(sector);
				_pieChartDiagrams->highlightCurve(sector,*_pieChart);
			}
		}
	}
	return QMainWindow::eventFilter(obj, event);
}

/*******************************
 * Private slots
 *******************************/

void MainWindow::openDicom()
{
	QString folderName = QFileDialog::getExistingDirectory(0,tr("Sélection du répertoire DICOM"),QDir::homePath(),QFileDialog::ShowDirsOnly);
	if ( !folderName.isEmpty() ) {
		closeImage();
		openNewBillon(folderName);
		updateUiComponentsValues();
		updateSliceHistogram();
		drawSlice();
		setWindowTitle(QString("TKDetection - %1").arg(folderName.section(QDir::separator(),-1)));
	}
}

void MainWindow::closeImage()
{
	openNewBillon();
	updateMarrow();
	updateSliceHistogram();
	_pieChartDiagrams->clear();
	_ui->_plotAngularHistogram->replot();
	_ui->_polarSectorSum->replot();
	selectSectorInterval(0);
	updateUiComponentsValues();
	drawSlice();
	setWindowTitle("TKDetection");
}

void MainWindow::drawSlice( const int &sliceNumber )
{
	_currentSlice = sliceNumber;
	if ( _billon != 0 )
	{
		_ui->_labelSliceNumber->setNum(sliceNumber);
		_pix.fill(0xff000000);
		_sliceView->drawSlice(_pix,*_billon,_marrow,sliceNumber,Interval<int>(_ui->_spinMinIntensity->value(),_ui->_spinMaxIntensity->value()), Interval<int>(_ui->_spinMovementThresholdMin->value(),_ui->_spinMovementThresholdMax->value()));
		highlightSliceHistogram(sliceNumber);
		if ( _marrow != 0 )
		{
			_marrow->draw(_pix,sliceNumber);
			if ( _ui->_checkRadiusAroundPith->isChecked() && _ui->_checkRadiusAroundPith->text().toInt() > 0 )
			{
				QPainter painter(&_pix);
				painter.setPen(Qt::yellow);
				iCoord2D center = _marrow->at(sliceNumber-_marrow->interval().min());
				painter.drawEllipse(QPointF(center.x,center.y),_ui->_checkRadiusAroundPith->text().toInt(),_ui->_checkRadiusAroundPith->text().toInt());
			}
		}

		const bool inDrawingArea = (_ui->_comboSelectSliceInterval->currentIndex() > 0 && _sliceHistogram->knotAreas().at(_ui->_comboSelectSliceInterval->currentIndex()-1).containsClosed(sliceNumber));
		if ( (_pieChartDiagrams->size() != 0) && inDrawingArea )
		{
			iCoord2D center(_pix.width()/2,_pix.height()/2);
			if ( _marrow != 0 &&  _marrow->interval().containsClosed(sliceNumber) )
			{
				center = _marrow->at(sliceNumber-_marrow->interval().min());
			}
			_pieChartDiagrams->draw(_pix,center,*_pieChart);
			_pieChart->draw(_pix,_currentSector, center);
		}
		if ( _sectorBillon != 0 && inDrawingArea )
		{
			if ( !_ui->_checkEnableConnexComponents->isChecked() || _componentBillon == 0 )
			{
				const int width = _sectorBillon->n_cols;
				const int height = _sectorBillon->n_rows;
				const int threshold = _ui->_sliderSectorThresholding->value();

				const Interval<int> &sliceInterval = _sliceHistogram->knotAreas()[_ui->_comboSelectSliceInterval->currentIndex()-1];
				const Slice &sectorSlice = _sectorBillon->slice(sliceNumber-sliceInterval.min());

				QPainter painter(&_pix);
				QColor color(255,0,0,127);
				painter.setPen(color);

				int i, j;
				for ( j=0 ; j<height ; ++j )
				{
					for ( i=0 ; i<width ; ++i )
					{
						if ( sectorSlice.at(j,i) > threshold )
						{
							painter.drawPoint(i,j);
						}
					}
				}
			}
			else
			{
				const int width = _componentBillon->n_cols;
				const int height = _componentBillon->n_rows;

				const QColor colors[] = { QColor(0,0,255,127), QColor(255,0,255,127), QColor(255,0,0,127), QColor(255,255,0,127), QColor(0,255,0,127) };
				const int nbColors = sizeof(colors)/sizeof(QColor);


				const Interval<int> &sliceInterval = _sliceHistogram->knotAreas()[_ui->_comboSelectSliceInterval->currentIndex()-1];
				Billon *biggestComponents = ConnexComponentExtractor::extractConnexComponents( _componentBillon->slice(sliceNumber-sliceInterval.min()), qPow(_ui->_spinMinimalSizeOf2DConnexComponents->value(),2), 0 );
				const Slice &sectorSlice = biggestComponents->slice(0);
//				const Slice &sectorSlice = _componentBillon->slice(sliceNumber-sliceInterval.minValue());
				const int selectedComponents = _ui->_comboConnexComponents->currentIndex();

				QPainter painter(&_pix);
				int i, j, color;
				if ( selectedComponents )
				{
					for ( j=0 ; j<height ; ++j )
					{
						for ( i=0 ; i<width ; ++i )
						{
							color = sectorSlice.at(j,i);
							if ( color == selectedComponents )
							{
								painter.setPen(colors[color%nbColors]);
								painter.drawPoint(i,j);
							}
						}
					}
				}
				else
				{
					for ( j=0 ; j<height ; ++j )
					{
						for ( i=0 ; i<width ; ++i )
						{
							color = sectorSlice.at(j,i);
							if ( color )
							{
								painter.setPen(colors[color%nbColors]);
								painter.drawPoint(i,j);
							}
						}
					}
				}
				painter.end();

				_contourCurve->constructCurve( *biggestComponents, _marrow != 0 ? _marrow->at(sliceNumber) : iCoord2D(width/2,height/2), 0, 0, _ui->_spinBlurredSegmentsThickness->value(), _ui->_spinContourSmoothingRadius->value() );
				//_contourCurve->constructCurve( *_componentBillon, _marrow != 0 ? _marrow->at(sliceNumber) : iCoord2D(width/2,height/2), sliceNumber-sliceInterval.minValue(), 1, _ui->_spinBlurredSegmentsThickness->value(), _ui->_spinContourSmoothingRadius->value() );
				_contourCurve->drawRestrictedComponent(_pix);
				_contourCurve->draw(_pix);

				delete biggestComponents;
			}
		}
	}
	else
	{
		_ui->_labelSliceNumber->setText(tr("Aucune"));
		_pix = QImage(0,0);
	}
	_ui->_labelSliceView->setPixmap(_billon != 0 ? QPixmap::fromImage(_pix).scaled(_pix.width()*_sliceZoomer.factor(),_pix.height()*_sliceZoomer.factor(),Qt::KeepAspectRatio) : QPixmap::fromImage(_pix));
}

void MainWindow::setSlice( const int &sliceNumber )
{
	drawSlice(sliceNumber);
}

void MainWindow::setTypeOfView( const int &type )
{
	enabledComponents();
	_sliceView->setTypeOfView( static_cast<const SliceType::SliceType>(type) );
	switch (type)
	{
		case SliceType::MOVEMENT:
			_ui->_toolboxSliceParameters->setCurrentWidget(_ui->_pageMovementParameters);
			break;
		case SliceType::EDGE_DETECTION :
			_ui->_toolboxSliceParameters->setCurrentWidget(_ui->_pageEdgeDetection);
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

void MainWindow::updateSliceHistogram()
{
	_sliceHistogram->detach();
	_sliceHistogram->clear();
	if ( _billon != 0 )
	{
		_sliceHistogram->constructHistogram(*_billon, _marrow, Interval<int>(_ui->_spinMinIntensity->value(),_ui->_spinMaxIntensity->value()),
											Interval<int>(_ui->_spinMovementThresholdMin->value(),_ui->_spinMovementThresholdMax->value()),
											_ui->_spinSmoothingRadiusOfHistogram->value(), _ui->_spinMinimumHeightofMaximum->value(),
											_ui->_spinMaximumsNeighborhood->value(), _ui->_spinDerivativePercentage->value(),
											_ui->_spinHistogramIntervalMinimumWidth->value(), _ui->_spinBorderPercentageToCut->value(), _ui->_checkRadiusAroundPith->text().toInt());
	}
	_histogramCursor.detach();
	_histogramDistanceMarrowToNearestPointCursor.detach();
	_sliceHistogram->attach(_ui->_plotSliceHistogram);
	_histogramCursor.attach(_ui->_plotSliceHistogram);
	_histogramDistanceMarrowToNearestPointCursor.attach(_ui->_plotDistanceMarrowToNearestPoint);
	_ui->_plotSliceHistogram->setAxisScale(QwtPlot::xBottom,0,(_billon != 0)?_billon->n_slices:0);
	highlightSliceHistogram(_currentSlice);

	const int oldIntervalIndex = _ui->_comboSelectSliceInterval->currentIndex();
	_ui->_comboSelectSliceInterval->clear();
	_ui->_comboSelectSliceInterval->addItem(tr("Aucun"));
	const QVector< Interval<int> > &intervals = _sliceHistogram->knotAreas();
	if ( !intervals.isEmpty() )
	{
		for ( int i=0 ; i<intervals.size() ; ++i )
		{
			const Interval<int> &interval = intervals[i];
			_ui->_comboSelectSliceInterval->addItem(tr("Interval %1 : [ %2, %3 ] (%4 coupes)").arg(i).arg(interval.min()).arg(interval.max()).arg(interval.width()));
		}
	}
	_ui->_comboSelectSliceInterval->setCurrentIndex(oldIntervalIndex<=intervals.size()?oldIntervalIndex:0);
}

void MainWindow::highlightSliceHistogram( const int &slicePosition )
{
	qreal height = _sliceHistogram->value(slicePosition);
	qreal x[4] = {slicePosition,slicePosition, slicePosition+1,slicePosition+1};
	qreal y[4] = {0,height,height,0};
	_histogramCursor.setSamples(x,y,4);
	_ui->_plotSliceHistogram->replot();
	if ( _componentBillon != 0 && _ui->_checkEnableConnexComponents->isChecked() && _ui->_comboSelectSectorInterval->currentIndex() > 0 )
	{
		int position = qMax(0,qMin(slicePosition-_sliceHistogram->knotAreas()[_ui->_comboSelectSliceInterval->currentIndex()-1].min(),_sliceHistogram->knotAreas()[_ui->_comboSelectSliceInterval->currentIndex()-1].width()));
		height = _histogramDistanceMarrowToNearestPoint.sample( position ).value;
		qreal x[4] = {position,position, position+1,position+1};
		qreal y[4] = {0,height,height,0};
		_histogramDistanceMarrowToNearestPointCursor.setSamples(x,y,4);
		_ui->_plotDistanceMarrowToNearestPoint->replot();
	}
}

void MainWindow::updateMarrow()
{
	if ( _marrow != 0 )
	{
		delete _marrow;
		_marrow = 0;
	}
	_ui->_checkRadiusAroundPith->setText( QString::number(100) );
	if ( _billon != 0 )
	{
		MarrowExtractor extractor;
		_marrow = extractor.process(*_billon,0,_billon->n_slices-1);
		if ( _marrow != 0 )
		{
			_ui->_checkRadiusAroundPith->setText( QString::number(static_cast<int>(_billon->getRestrictedAreaMeansRadius(*_marrow,20,_ui->_spinMinIntensity->value())*0.75)) );
		}
	}
	drawSlice();
	updateSliceHistogram();
}

void MainWindow::highlightSectorHistogram( const int &sectorIdx )
{
	_currentSector = sectorIdx;
	drawSlice();
}

void MainWindow::setMinimumOfSliceInterval( const int &min )
{
	_ui->_spinMaxSlice->setMinimum(min);
	_ui->_spinMinSlice->setMaximum(qMax(min,_ui->_spinMinSlice->maximum()));
	_ui->_spinMinSlice->blockSignals(true);
		_ui->_spinMinSlice->setValue(min);
	_ui->_spinMinSlice->blockSignals(false);
	_ui->_spanSliderSelectInterval->blockSignals(true);
		_ui->_spanSliderSelectInterval->setLowerValue(min);
	_ui->_spanSliderSelectInterval->blockSignals(false);
}

void MainWindow::setMinimumOfSliceIntervalToCurrentSlice()
{
	setMinimumOfSliceInterval(_currentSlice);
}

void MainWindow::setMaximumOfSliceInterval(const int &max)
{
	_ui->_spinMinSlice->setMaximum(max);
	_ui->_spinMaxSlice->setMinimum(qMin(max,_ui->_spinMaxSlice->minimum()));
	_ui->_spinMaxSlice->blockSignals(true);
		_ui->_spinMaxSlice->setValue(max);
	_ui->_spinMaxSlice->blockSignals(false);
	_ui->_spanSliderSelectInterval->blockSignals(true);
		_ui->_spanSliderSelectInterval->setUpperValue(max);
	_ui->_spanSliderSelectInterval->blockSignals(false);
}

void MainWindow::setMaximumOfSliceIntervalToCurrentSlice()
{
	setMaximumOfSliceInterval(_currentSlice);
}

void MainWindow::previousMaximumInSliceHistogram()
{
	const int nbMaximums = _sliceHistogram->maximums().size();
	_currentMaximum = nbMaximums <= 0 ? -1 : _currentMaximum < 0 ? 0 : _currentMaximum == 0 ? nbMaximums-1 : ( _currentMaximum - 1 ) % nbMaximums;
	int sliceIndex = _sliceHistogram->maximumIndex(_currentMaximum);
	if ( sliceIndex > -1 )
	{
		_ui->_sliderSelectSlice->setValue(sliceIndex);
	}
}

void MainWindow::nextMaximumInSliceHistogram()
{
	const int nbMaximums = _sliceHistogram->nbMaximums();
	_currentMaximum = nbMaximums>0 ? ( _currentMaximum + 1 ) % nbMaximums : -1;
	int sliceIndex = _sliceHistogram->maximumIndex(_currentMaximum);
	if ( sliceIndex > -1 )
	{
		_ui->_sliderSelectSlice->setValue(sliceIndex);
	}
}

void MainWindow::zoomInSliceView( const qreal &zoomFactor, const QPoint & /*focalPoint*/ )
{
	_ui->_labelSliceView->setPixmap(_billon != 0 ? QPixmap::fromImage(_pix).scaled(_pix.width()*zoomFactor,_pix.height()*zoomFactor,Qt::KeepAspectRatio) : QPixmap::fromImage(_pix));
}

void MainWindow::dragInSliceView( const QPoint &movementVector )
{
	QScrollArea &scrollArea = *(_ui->_scrollSliceView);
	if ( movementVector.x() != 0 ) scrollArea.horizontalScrollBar()->setValue(scrollArea.horizontalScrollBar()->value()-movementVector.x());
	if ( movementVector.y() != 0 ) scrollArea.verticalScrollBar()->setValue(scrollArea.verticalScrollBar()->value()-movementVector.y());
}

void MainWindow::flowApplied()
{
	const qreal currentAlpha = _sliceView->flowAlpha();
	const qreal currentEpsilon = _sliceView->flowEpsilon();
	const qreal currentMaxIter = _sliceView->flowMaximumIterations();

	const qreal newAlpha = _ui->_spinFlowAlpha->value();
	const qreal newEpsilon = _ui->_spinFlowEpsilon->value();
	const qreal newMaxIter = _ui->_spinFlowMaximumIterations->value();

	bool hasModification = false;
	if ( currentAlpha != newAlpha )
	{
		_sliceView->setFlowAlpha(newAlpha);
		hasModification = true;
	}
	if ( currentEpsilon != newEpsilon )
	{
		_sliceView->setFlowEpsilon(newEpsilon);
		hasModification = true;
	}
	if ( currentMaxIter != newMaxIter )
	{
		_sliceView->setFlowMaximumIterations(newMaxIter);
		hasModification = true;
	}

	if ( hasModification ) drawSlice();
}

void MainWindow::setRestrictedAreaResolution( const int &resolution )
{
	_sliceView->setRestrictedAreaResolution(resolution);
	drawSlice();
}

void MainWindow::setRestrictedAreaThreshold( const int &threshold )
{
	_sliceView->setRestrictedAreaThreshold(threshold);
	drawSlice();
}

void MainWindow::setRestrictedAreaBeginRadius( const int &radius )
{
	_sliceView->setRestrictedAreaBeginRadius( radius );

	_ui->_sliderRestrictedAreaBeginRadius->blockSignals(true);
		_ui->_sliderRestrictedAreaBeginRadius->setValue(radius);
	_ui->_sliderRestrictedAreaBeginRadius->blockSignals(false);

	_ui->_spinRestrictedAreaBeginRadius->blockSignals(true);
		_ui->_spinRestrictedAreaBeginRadius->setValue(radius);
	_ui->_spinRestrictedAreaBeginRadius->blockSignals(false);

	drawSlice();
}

void MainWindow::setEdgeDetectionType( const int &type )
{
	_sliceView->setEdgeDetectionType( static_cast<const EdgeDetectionType::EdgeDetectionType>(type) );
	drawSlice();
}

void MainWindow::setCannyRadiusOfGaussianMask( const int &radius )
{
	_sliceView->setRadiusOfGaussianMask(radius);
	drawSlice();
}

void MainWindow::setCannySigmaOfGaussianMask( const double &sigma )
{
	_sliceView->setSigmaOfGaussianMask(sigma);
	drawSlice();
}

void MainWindow::setCannyMinimumGradient( const int &minimumGradient )
{
	_sliceView->setCannyMinimumGradient(minimumGradient);
	drawSlice();
}

void MainWindow::setCannyMinimumDeviation( const double &minimumDeviation )
{
	_sliceView->setCannyMinimumDeviation(minimumDeviation);
	drawSlice();
}

void MainWindow::exportToDat()
{
	if ( _billon != 0 ) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .dat"), "output.dat", tr("Fichiers de données (*.dat);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			bool ok;
			qreal contrastFactor = QInputDialog::getInt(this,tr("Facteur de contraste"), tr("Contraste de l'image (image originale avec contraste à 0)"), 0, -100, 100, 1, &ok);
			if ( ok )
			{
				DatExport::process( *_billon, Interval<int>(_ui->_spanSliderSelectInterval->lowerValue(),_ui->_spanSliderSelectInterval->upperValue()), Interval<int>(_ui->_spinMinIntensity->value(),_ui->_spinMaxIntensity->value()), fileName, _ui->_spinExportResolution->value(), (contrastFactor+100.)/100. );
			}
		}
	}
}

void MainWindow::exportToOfs()
{
	if ( _billon != 0 && _marrow != 0 )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .ofs"), "output.ofs", tr("Fichiers de données (*.ofs);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
		  OfsExport::process( *_billon, *_marrow, Interval<int>(_ui->_spanSliderSelectInterval->lowerValue(),_ui->_spanSliderSelectInterval->upperValue()), fileName, _ui->_spinExportNbEdges->value(), _ui->_spinExportRadius->value(), false );
		}
	}
}
void MainWindow::exportToOfsAll()
{
	if (  _marrow != 0 )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .ofs"), "output.ofs", tr("Fichiers de données (*.ofs);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
		  OfsExport::process( *_billon, *_marrow, _marrow->interval(), fileName, _ui->_spinExportNbEdges->value(), _ui->_spinExportRadius->value(), false );
		}
	}
}

void MainWindow::exportToOfsRestricted()
{
	if ( _billon != 0 && _marrow != 0 )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .ofs"), "output.ofs", tr("Fichiers de données (*.ofs);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
		  OfsExport::processRestrictedMesh( *_billon, *_marrow, Interval<uint>(_ui->_spanSliderSelectInterval->lowerValue(),_ui->_spanSliderSelectInterval->upperValue()), fileName, 100, MINIMUM_INTENSITY, false, (_ui->_closeTrunk)->isChecked() );
		}
	}
}

void MainWindow::exportSectorToOfs() {
	int index = _ui->_comboSelectSectorInterval->currentIndex();
	if ( _billon != 0 && _marrow != 0 && index > 0 && index <= _pieChartDiagrams->knotIntervals().size() ) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter le secteur %1 en .ofs").arg(index), QString("sector_%1.ofs").arg(index), tr("Fichiers de données (*.ofs);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() ) {
			const Interval<int> &sectorInterval = _pieChartDiagrams->knotIntervals()[_ui->_comboSelectSectorInterval->currentIndex()-1];
			const Interval<int> &slicesInterval = _sliceHistogram->knotAreas()[_ui->_comboSelectSliceInterval->currentIndex()-1];
			OfsExport::processOnSector( *_billon, *_marrow, slicesInterval, fileName, _pieChart->sector(sectorInterval.min()).rightAngle(), _pieChart->sector(sectorInterval.max()).leftAngle(), _ui->_spinExportNbEdges->value() );
		}
	}
}

void MainWindow::exportAllSectorInAllIntervalsToOfs() {
	if ( _billon != 0 && _marrow != 0 && _ui->_comboSelectSectorInterval->count() > 0 ) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter tous les secteurs de tous les intervalles en .ofs"), "allSector.ofs", tr("Fichiers de données (*.ofs);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() ) {
			QVector< QPair< Interval<int>, QPair<qreal,qreal> > > intervals;
			for ( int i=0 ; i<_ui->_comboSelectSliceInterval->count()-1 ; i++ ) {
				const Interval<int> &slicesInterval = _sliceHistogram->knotAreas()[i];
				for ( int j=0 ; j<_ui->_comboSelectSectorInterval->count()-1 ; j++ ) {
					const Interval<int> &sectorInterval = _pieChartDiagrams->knotIntervals()[j];
					const QPair<qreal,qreal> angles( _pieChart->sector(sectorInterval.min()).rightAngle(), _pieChart->sector(sectorInterval.max()).leftAngle() );
					intervals.append( QPair< Interval<int>, QPair<qreal,qreal> >( slicesInterval, angles ) );
				}
			}
			OfsExport::processOnAllSectorInAllIntervals( *_billon, *_marrow, intervals, fileName, _ui->_spinExportNbEdges->value() );
		}
	}
}

void MainWindow::exportHistogramToSep()
{
	if ( _sliceHistogram != 0 )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter l'histogramme en .sep"), "output.sep", tr("Fichiers séquences de point euclidiens (*.sep);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if ( file.open(QIODevice::WriteOnly) )
			{
				QTextStream stream(&file);
				stream << *_sliceHistogram;
				file.close();
			}
		}
	}
}

void MainWindow::exportToV3D() {
	if ( _billon != 0 ) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .v3d"), "output.v3d", tr("Fichiers de données (*.v3d);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() ) {
			V3DExport::process( *_billon, _marrow, fileName, Interval<int>(_ui->_spanSliderSelectInterval->lowerValue(),_ui->_spanSliderSelectInterval->upperValue()), _ui->_spinExportThreshold->value() );
		}
	}
}

void MainWindow::exportFlowToV3D() {
	if ( _billon != 0 ) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .v3d"), "output_flow.v3d", tr("Fichiers de données (*.v3d);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() ) {
			const Interval<int> slicesInterval(_ui->_spanSliderSelectInterval->lowerValue(),_ui->_spanSliderSelectInterval->upperValue());
			const int width = _billon->n_cols;
			const int height = _billon->n_rows;
			const int depth = slicesInterval.width()+1;
			Billon billonFlow( width, height, depth );
			billonFlow.setMinValue(_billon->minValue());
			billonFlow.setMaxValue(_billon->maxValue());
			billonFlow.setVoxelSize(_billon->voxelWidth(),_billon->voxelHeight(),_billon->voxelDepth());
			VectorsField *field = 0;
			int i,j,k;

			for ( k=0 ; k<depth ; ++k ) {
				field = OpticalFlow::compute(*_billon,k+slicesInterval.min(),_ui->_spinFlowAlpha->value(),_ui->_spinFlowEpsilon->value(),_ui->_spinFlowMaximumIterations->value());
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

			V3DExport::process( billonFlow, _marrow, fileName, Interval<int>(0,depth-1), _ui->_spinExportThreshold->value() );
		}
	}
}

void MainWindow::exportMovementsToV3D()
{
	if ( _billon != 0 )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .v3d"), "output_diag.v3d", tr("Fichiers de données (*.v3d);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			const Interval<int> slicesInterval(_ui->_spanSliderSelectInterval->lowerValue(),_ui->_spanSliderSelectInterval->upperValue());
			const int width = _billon->n_cols;
			const int height = _billon->n_rows;
			const int depth = slicesInterval.width()+1;
			const int minValue = _ui->_spinMinIntensity->value();
			const int maxValue = _ui->_spinMaxIntensity->value();
			const int thresholdMin = _ui->_sliderMovementThresholdInterval->lowerValue();
			const int thresholdMax = _ui->_sliderMovementThresholdInterval->upperValue();

			Billon billonDiag( *_billon );
			int i,j,k, pixAbsDiff;

			for ( k=slicesInterval.min() ; k<slicesInterval.max() ; ++k )
			{
				const Slice &currentSlice = _billon->slice(k);
				const Slice &previousSlice = _billon->slice(k>0?k-1:k+1);
				Slice &sliceDiag = billonDiag.slice(k-slicesInterval.min()-1);
				for ( j=0 ; j<height ; ++j )
				{
					for ( i=0 ; i<width ; ++i )
					{
						pixAbsDiff = qAbs(RESTRICT_TO(minValue,previousSlice.at(j,i),maxValue) - RESTRICT_TO(minValue,currentSlice.at(j,i),maxValue));
						if ( (pixAbsDiff <= thresholdMin) || (pixAbsDiff >= thresholdMax) ) sliceDiag.at(j,i) = minValue;
						else sliceDiag.at(i,j) = maxValue;
					}
				}
			}

			V3DExport::process( billonDiag, _marrow, fileName, Interval<int>(0,depth-2), _ui->_spinExportThreshold->value() );
		}
	}
}

void MainWindow::selectSliceInterval( const int &index )
{
	selectSectorInterval(0);

	_ui->_comboSelectSectorInterval->clear();
	_ui->_comboSelectSectorInterval->addItem(tr("Aucun"));
	_ui->_spanSliderSelectInterval->setLowerValue(0);
	_ui->_spanSliderSelectInterval->setUpperValue(0);
	if ( index > 0 && index <= _sliceHistogram->knotAreas().size() )
	{
		const Interval<int> &sliceInterval = _sliceHistogram->knotAreas()[index-1];
		computeSectorsHistogramForInterval(sliceInterval);
		_ui->_sliderSelectSlice->setValue(_sliceHistogram->intervalIndex(index-1));

		const QVector< Interval<int> > &angularIntervals = _pieChartDiagrams->knotIntervals();
		if ( !angularIntervals.isEmpty() )
		{
			qreal rightAngle, leftAngle;
			for ( int i=0 ; i<angularIntervals.size() ; ++i )
			{
				const Interval<int> currentAngularInterval = angularIntervals[i];
				rightAngle = _pieChart->sector(currentAngularInterval.min()).rightAngle()*RAD_TO_DEG_FACT;
				leftAngle = _pieChart->sector(currentAngularInterval.max()).leftAngle()*RAD_TO_DEG_FACT;
				_ui->_comboSelectSectorInterval->addItem(tr("Secteur %1 : [ %2, %3 ] (%4 degres)").arg(i).arg(rightAngle).arg(leftAngle).arg(currentAngularInterval.isValid()?leftAngle-rightAngle:leftAngle-rightAngle+360.));
			}
		}
		_ui->_spanSliderSelectInterval->setUpperValue(sliceInterval.max());
		_ui->_spanSliderSelectInterval->setLowerValue(sliceInterval.min());
	}
}

void MainWindow::selectCurrentSliceInterval() {
	selectSliceInterval(_ui->_comboSelectSliceInterval->currentIndex());
}

void MainWindow::selectSectorInterval( const int &index ) {
	if ( _sectorBillon != 0 )
	{
		delete _sectorBillon;
		_sectorBillon = 0;
	}
	if ( _componentBillon != 0 )
	{
		delete _componentBillon;
		_componentBillon = 0;
	}
	_ui->_comboConnexComponents->clear();
	_ui->_comboConnexComponents->addItem(tr("Toutes"));
	_knotIntervalInDistanceMarrowToNearestPointHistogram.setBounds(-1,-1);
	if ( index > 0 && index <= _pieChartDiagrams->knotIntervals().size() )
	{
		const Interval<int> &sectorInterval = _pieChartDiagrams->knotIntervals()[_ui->_comboSelectSectorInterval->currentIndex()-1];
		const Interval<int> &sliceInterval = _sliceHistogram->knotAreas()[_ui->_comboSelectSliceInterval->currentIndex()-1];
		const int firstSlice = sliceInterval.min();
		const int lastSlice = sliceInterval.max()+1;
		const int width = _billon->n_cols;
		const int height = _billon->n_rows;
		const Interval<int> intensityInterval(_ui->_spinSectorThresholding->value(), _ui->_spinMaxIntensity->value());

		_sectorBillon = new Billon(_billon->n_cols,_billon->n_rows,sliceInterval.width()+1);
		_sectorBillon->setMinValue(intensityInterval.min());
		_sectorBillon->setMaxValue(MAXIMUM_INTENSITY);
		_sectorBillon->setVoxelSize(_billon->voxelWidth(),_billon->voxelHeight(),_billon->voxelDepth());
		_sectorBillon->fill(intensityInterval.min());

		int i, j, k;
		for ( k=firstSlice ; k<lastSlice ; ++k )
		{
			const Slice &originalSlice = _billon->slice(k);
			Slice &sectorSlice = _sectorBillon->slice(k-firstSlice);
			const iCoord2D &marrowCoord = _marrow->at(k);
			for ( j=0 ; j<height ; ++j )
			{
				for ( i=0 ; i<width ; ++i )
				{
					if ( intensityInterval.containsOpen(originalSlice.at(j,i)) && sectorInterval.containsClosed(_pieChart->sectorIndexOfAngle(marrowCoord.angle(iCoord2D(i,j)))) )
					{
						sectorSlice.at(j,i) = originalSlice.at(j,i);
					}
				}
			}
		}

		if ( _ui->_checkEnableConnexComponents->isChecked() )
		{
			_componentBillon = ConnexComponentExtractor::extractConnexComponents(*_sectorBillon,qPow(_ui->_spinMinimalSizeOf3DConnexComponents->value(),3),intensityInterval.min());
			//_componentBillon = ConnexComponentExtractor::extractBiggestConnexComponent(*_sectorBillon,threshold);
			const int nbComponents = _componentBillon->maxValue();
			for ( i=1 ; i<nbComponents ; ++i )
			{
				_ui->_comboConnexComponents->addItem(tr("Composante %1").arg(i));
			}
			const int depth = _componentBillon->n_slices;
			QVector<QwtIntervalSample> histData(depth,QwtIntervalSample(0,0,0));
			Billon *biggestComponents;
			iCoord2D nearestPoint;

			int minIndex;
			qreal minVal;

			minIndex = 0;
			minVal = width;
			for ( i=0 ; i<depth ; ++i )
			{
				biggestComponents = ConnexComponentExtractor::extractConnexComponents( _componentBillon->slice(i), qPow(_ui->_spinMinimalSizeOf2DConnexComponents->value(),2), 0 );
				nearestPoint = biggestComponents->findNearestPointOfThePith( _marrow->at(firstSlice+i), 0, 0 );
				//nearestPoint = _componentBillon->findNearestPointOfThePith( _marrow->at(firstSlice+i), i, 0 );
				histData[i].value = nearestPoint.euclideanDistance(_marrow->at(firstSlice+i));
				histData[i].interval.setInterval(i,i+1);
				if ( minVal > histData[i].value )
				{
					minVal = histData[i].value;
					minIndex = i;
				}
				delete biggestComponents;
				biggestComponents = 0;
			}

			_histogramDistanceMarrowToNearestPoint.setSamples(histData);
			_ui->_plotDistanceMarrowToNearestPoint->replot();

			int upperIndex, lowerIndex;

			upperIndex = qMin(depth-5,minIndex);
			while ( upperIndex < depth-4 && (histData[upperIndex+5].value - histData[upperIndex].value > 5.) ) upperIndex++;
			lowerIndex = qMax(5,minIndex);
			while ( lowerIndex > 3 && histData[lowerIndex-5].value - histData[lowerIndex].value > 5. ) lowerIndex--;

			_knotIntervalInDistanceMarrowToNearestPointHistogram.setBounds(lowerIndex+3,upperIndex-3);

			std::cout << "Bornes du noeud : [ " << lowerIndex+1+firstSlice << ", " << upperIndex-1+firstSlice << " ]" << std::endl;
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
			bool ok;
			qreal contrastFactor = QInputDialog::getInt(this,tr("Facteur de contraste"), tr("Contraste de l'image (image originale avec contraste à 0)"), 0, -100, 100, 1, &ok);
			if ( ok ) {
				Pgm3dExport::process( *_sectorBillon, fileName, (contrastFactor+100.)/100. );
			}
		}
	}
}

void MainWindow::exportConnexComponentToPgm3D() {
	if ( _componentBillon != 0 ) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter la composante connexe en .pgm3d"), "output.pgm3d", tr("Fichiers de données (*.pgm3d);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() ) {
			bool ok;
			qreal contrastFactor = QInputDialog::getInt(this,tr("Facteur de contraste"), tr("Contraste de l'image (image originale avec contraste à 0)"), 0, -100, 100, 1, &ok);
			if ( ok ) {
				Pgm3dExport::process( *_componentBillon, fileName, (contrastFactor+100.)/100., _ui->_comboConnexComponents->currentIndex() );
			}
		}
	}
}

void MainWindow::exportSliceHistogram() {
	QwtPlotRenderer histoRenderer;
	QLabel label;
	QString fileName;
	QMessageBox::StandardButton button;
	int sizeFact;
	bool sizeOk;
	sizeOk = false;

	_ui->_plotSliceHistogram->setAxisTitle(QwtPlot::xBottom,tr("Indice de la coupe"));
	_ui->_plotSliceHistogram->enableAxis(QwtPlot::yLeft);
	_ui->_plotSliceHistogram->setAxisTitle(QwtPlot::yLeft,tr("Cumul du z-mouvement"));

	while (!sizeOk) {
		sizeFact = QInputDialog::getInt(this,tr("Taille de l'image"), tr("Pourcentage"), 100, 10, 100, 1, &sizeOk);
		if ( sizeOk ) {
			QPixmap image( 1240*sizeFact/100 , 874*sizeFact/100 );
			image.fill();
			histoRenderer.renderTo(_ui->_plotSliceHistogram,image);
			image = image.scaledToHeight(600,Qt::SmoothTransformation);
			label.setPixmap(image);
			label.show();
			button = QMessageBox::question(&label,tr("Taille correcte"),tr("La taille de l'image est-elle correcte ?"),QMessageBox::Abort|QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
			switch (button) {
				case QMessageBox::Yes:
					fileName = QFileDialog::getSaveFileName(&label, tr("Exporter l'histogramme de coupes"), "output.pdf", tr("Fichiers PDF (*.pdf);;Fichiers PS (*.ps);;Fichiers PNG (*.png);;Fichiers SVG (*.svg);;Tous les fichiers (*.*)"));
					if ( !fileName.isEmpty() ) {
						histoRenderer.renderDocument(_ui->_plotSliceHistogram,fileName,QSize(297*sizeFact/100,210*sizeFact/100),100);
					}
					sizeOk = true;
					break;
				case QMessageBox::Abort:
					sizeOk = true;
					break;
				default :
					sizeOk = false;
					break;
			}
		}
		else {
			sizeOk = true;
		}
	}

	_ui->_plotSliceHistogram->setAxisTitle(QwtPlot::xBottom,"");
	_ui->_plotSliceHistogram->setAxisTitle(QwtPlot::yLeft,"");
	_ui->_plotSliceHistogram->enableAxis(QwtPlot::yLeft,false);
}

void MainWindow::exportSectorDiagramAndHistogram() {
	QString fileName;
	QwtPlotRenderer histoRenderer;
	QwtPolarRenderer diagramRenderer;
	QLabel label1, label2;
	QMessageBox::StandardButton button;
	int sizeFact;
	bool sizeOk, abort;
	sizeOk = abort = false;

	_ui->_plotAngularHistogram->setAxisTitle(QwtPlot::xBottom,tr("Secteur angulaire en radians"));
	_ui->_plotAngularHistogram->enableAxis(QwtPlot::yLeft);
	_ui->_plotAngularHistogram->setAxisTitle(QwtPlot::yLeft,tr("Cumul du z-mouvement"));

	while (!sizeOk && !abort)
	{
		sizeFact = QInputDialog::getInt(this,tr("Taille de l'image"), tr("Pourcentage"), 100, 10, 100, 1, &sizeOk);
		if ( sizeOk ) {
			QPixmap image1( 1240*sizeFact/100 , 874*sizeFact/100 );
			QPixmap image2( 1240*sizeFact/100 , 874*sizeFact/100 );
			image1.fill();
			image2.fill();
			histoRenderer.renderTo(_ui->_plotAngularHistogram,image1);
			diagramRenderer.renderTo(_ui->_polarSectorSum,image2);
			image1 = image1.scaledToHeight(600,Qt::SmoothTransformation);
			image2 = image2.scaledToHeight(600,Qt::SmoothTransformation);
			label1.setPixmap(image1);
			label2.setPixmap(image2);
			label1.show();
			label2.show();
			button = QMessageBox::question(&label1,tr("Taille correcte"),tr("La taille de l'image est-elle correcte ?"),QMessageBox::Abort|QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
			switch (button) {
				case QMessageBox::Yes:
					fileName = QFileDialog::getSaveFileName(&label1, tr("Exporter le diagramme et l'histogramme."), "output.pdf", tr("Fichiers PDF (*.pdf);;Fichiers PS (*.ps);;Fichiers PNG (*.png);;Fichiers SVG (*.svg);;Tous les fichiers (*.*)"));
					if ( !fileName.isEmpty() ) {
						QString chemin = fileName.section(QDir::separator(),0,-2)+QDir::separator();
						QString name = fileName.section(QDir::separator(),-1);
						histoRenderer.renderDocument(_ui->_plotAngularHistogram, chemin+"histo_"+name, QSize(297*sizeFact/100,210*sizeFact/100), 100);
						diagramRenderer.renderDocument(_ui->_polarSectorSum, chemin+"diag_"+name, QSize(297*sizeFact/100,210*sizeFact/100), 100);
					}
					sizeOk = true;
					break;
				case QMessageBox::Abort:
					abort = true;
					break;
				default :
					sizeOk = false;
					break;
			}
		}
		else {
			sizeOk = true;
		}
	}

	_ui->_plotAngularHistogram->setAxisTitle(QwtPlot::xBottom,"");
	_ui->_plotAngularHistogram->setAxisTitle(QwtPlot::yLeft,"");
	_ui->_plotAngularHistogram->enableAxis(QwtPlot::yLeft,false);
}

void MainWindow::exportKnotIntervalHistogram()
{
	QwtPlotRenderer histoRenderer;
	QLabel label;
	QString fileName;
	QMessageBox::StandardButton button;
	int sizeFact;
	bool sizeOk;
	sizeOk = false;

	_ui->_plotDistanceMarrowToNearestPoint->setAxisTitle(QwtPlot::xBottom,tr("Slice index"));
	_ui->_plotDistanceMarrowToNearestPoint->setAxisTitle(QwtPlot::yLeft,tr("Distance to the pith"));

	while (!sizeOk)
	{
		sizeFact = QInputDialog::getInt(this,tr("Taille de l'image"), tr("Pourcentage"), 100, 10, 100, 1, &sizeOk);
		if ( sizeOk )
		{
			QPixmap image( 1240*sizeFact/100 , 874*sizeFact/100 );
			image.fill();
			histoRenderer.renderTo(_ui->_plotDistanceMarrowToNearestPoint,image);
			image = image.scaledToHeight(600,Qt::SmoothTransformation);
			label.setPixmap(image);
			label.show();
			button = QMessageBox::question(&label,tr("Taille correcte"),tr("La taille de l'image est-elle correcte ?"),QMessageBox::Abort|QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
			switch (button) {
				case QMessageBox::Yes:
					fileName = QFileDialog::getSaveFileName(&label, tr("Exporter l'histogramme de distances"), "output.pdf", tr("Fichiers PDF (*.pdf);;Fichiers PS (*.ps);;Fichiers PNG (*.png);;Fichiers SVG (*.svg);;Tous les fichiers (*.*)"));
					if ( !fileName.isEmpty() ) {
						histoRenderer.renderDocument(_ui->_plotDistanceMarrowToNearestPoint,fileName,QSize(297*sizeFact/100,140*sizeFact/100),100);
					}
					sizeOk = true;
					break;
				case QMessageBox::Abort:
					sizeOk = true;
					break;
				default :
					sizeOk = false;
					break;
			}
		}
		else {
			sizeOk = true;
		}
	}

	_ui->_plotDistanceMarrowToNearestPoint->setAxisTitle(QwtPlot::xBottom,"");
	_ui->_plotDistanceMarrowToNearestPoint->setAxisTitle(QwtPlot::yLeft,"");
}

void MainWindow::exportContours() {
	if ( _componentBillon != 0 ) {
		const Interval<int> &sliceInterval = _sliceHistogram->knotAreas()[_ui->_comboSelectSliceInterval->currentIndex()-1];

		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter le contour en .ctr"), "output.ctr", tr("Fichiers de contours (*.ctr);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() ) {
			QFile file(fileName);
			if( !file.open(QIODevice::WriteOnly) ) {
				qDebug() << QObject::tr("ERREUR : Impossible de créer le ficher de contours %1.").arg(fileName);
				return;
			}
			Billon *biggestComponent = ConnexComponentExtractor::extractBiggestConnexComponent( _componentBillon->slice(_currentSlice-sliceInterval.min()), 0 );
			QVector<iCoord2D> contourPoints = biggestComponent->extractContour( _marrow != 0 ? _marrow->at(_currentSlice) : iCoord2D(_billon->n_cols/2,_billon->n_rows/2), 0, 0 );

			QTextStream stream(&file);
			stream << contourPoints.size() << endl;
			for ( int i=0 ; i<contourPoints.size() ; ++i ) {
				stream << contourPoints.at(i).x << " " << contourPoints.at(i).y << endl;
			}
			file.close();
			delete biggestComponent;
		}
	}
}

void MainWindow::exportContourComponentToPgm3D()
{
	if ( _componentBillon != 0 ) {
		const Interval<int> &sliceInterval = _sliceHistogram->knotAreas()[_ui->_comboSelectSliceInterval->currentIndex()-1];

		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter la composante délimitée par le contour en PGM3D"), "output.pgm3d", tr("Fichiers PGM3D (*.pgm3d);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() ) {
			QFile file(fileName);
			if( !file.open(QIODevice::WriteOnly) ) {
				qDebug() << QObject::tr("ERREUR : Impossible de créer le ficher %1.").arg(fileName);
				return;
			}

			const int &width = _componentBillon->n_cols;
			const int &height = _componentBillon->n_rows;
			const int &depth = _knotIntervalInDistanceMarrowToNearestPointHistogram.width()+1;

			QTextStream stream(&file);
			stream << "P3D" << endl;
			stream << width << " " << height << " " << depth << endl;
			stream << 1 << endl;

			QDataStream dstream(&file);

			Billon *biggestComponents;
			ContourCurve contourCurve;
			iCoord2D marrowCoord;
			for ( int k=_knotIntervalInDistanceMarrowToNearestPointHistogram.min() ; k<=_knotIntervalInDistanceMarrowToNearestPointHistogram.max() ; ++k )
			{
				marrowCoord = _marrow != 0 ? _marrow->at(sliceInterval.min()+k) : iCoord2D(width/2,height/2);
				biggestComponents = ConnexComponentExtractor::extractConnexComponents( _componentBillon->slice(k), qPow(_ui->_spinMinimalSizeOf2DConnexComponents->value(),2), 0 );
				contourCurve.constructCurve( *biggestComponents, marrowCoord, 0, 1, _ui->_spinBlurredSegmentsThickness->value(), _ui->_spinContourSmoothingRadius->value() );
				contourCurve.writeContourContentInPgm3D(dstream);
				delete biggestComponents;
				biggestComponents = 0;
			}

			file.close();

			QMessageBox::information(this,"Export de la composante délimitée par le contour en PGM3D", "Export réussi !");
		}
	}
}

void MainWindow::createVoxelSetAllIntervals(std::vector<iCoord3D> &vectVoxels, bool useOldMethod)
{
	for ( int l=1 ; l< _ui->_comboSelectSliceInterval->count() ; l++ )
	{
		cerr << "-------------------------"<< endl;
		cerr << "processing Interval Num=" << l << endl;

		_ui->_comboSelectSliceInterval->setCurrentIndex(l);
		selectSliceInterval(l);
		const Interval<int> &sliceInterval = _sliceHistogram->knotAreas()[l-1];
		const QVector< Interval<int> > &intervals = _pieChartDiagrams->knotIntervals();

		if ( !intervals.isEmpty() )
		{
			Billon *biggestComponents;
			iCoord2D marrowCoord;
			ContourCurve contourCurve;

			for ( int i=0 ; i<intervals.size()-1 ; ++i )
			{
				cerr << "Generating contours knot num " << i ;
				_ui->_comboSelectSectorInterval->setCurrentIndex(i+1);

				const int &width = _componentBillon->n_cols;
				const int &height = _componentBillon->n_rows;

				for ( int k=_knotIntervalInDistanceMarrowToNearestPointHistogram.min() ; k<=_knotIntervalInDistanceMarrowToNearestPointHistogram.max() ; ++k )
				{
					marrowCoord = _marrow != 0 ? _marrow->at(sliceInterval.min()+k) : iCoord2D(width/2,height/2);
					biggestComponents = ConnexComponentExtractor::extractConnexComponents( _componentBillon->slice(k), qPow(_ui->_spinMinimalSizeOf2DConnexComponents->value(),2), 0 );
					if(useOldMethod){
					  contourCurve.constructCurveOldMethod( *biggestComponents, marrowCoord, 0, 0, _ui->_spinContourSmoothingRadius->value() );
					}else{
					  contourCurve.constructCurve( *biggestComponents, marrowCoord, 0, 0, _ui->_spinBlurredSegmentsThickness->value(), _ui->_spinContourSmoothingRadius->value() );
					}
					contourCurve.getContourContentPoints(vectVoxels, k+sliceInterval.min());

					delete biggestComponents;
					biggestComponents = 0;
				}
				cerr << " ... [done]" <<endl;
			}
		}
	}
}

void MainWindow::createVoxelSet(std::vector<iCoord3D> &vectVoxels)
{
	const Interval<int> &sliceInterval = _sliceHistogram->knotAreas()[_ui->_comboSelectSliceInterval->currentIndex()-1];
	const int minSlice = sliceInterval.min();
	const QVector< Interval<int> > &intervals = _pieChartDiagrams->knotIntervals();

	if ( !intervals.isEmpty() )
	{
		Billon *biggestComponents;
		iCoord2D marrowCoord;
		ContourCurve contourCurve;

		for ( int i=0 ; i<intervals.size() ; ++i )
		{
			cerr << "Generating contours knot num " << i ;
			_ui->_comboSelectSectorInterval->setCurrentIndex(i+1);

			const int &width = _componentBillon->n_cols;
			const int &height = _componentBillon->n_rows;

			for ( int k=_knotIntervalInDistanceMarrowToNearestPointHistogram.min() ; k<=_knotIntervalInDistanceMarrowToNearestPointHistogram.max() ; ++k )
			{
				marrowCoord = _marrow != 0 ? _marrow->at(minSlice+k) : iCoord2D(width/2,height/2);
				biggestComponents = ConnexComponentExtractor::extractConnexComponents( _componentBillon->slice(k), qPow(_ui->_spinMinimalSizeOf2DConnexComponents->value(),2), 0 );
				contourCurve.constructCurve( *biggestComponents, marrowCoord, 0, 0, _ui->_spinBlurredSegmentsThickness->value(), _ui->_spinContourSmoothingRadius->value() );
				contourCurve.getContourContentPoints(vectVoxels, minSlice+ k);

				delete biggestComponents;
				biggestComponents = 0;
			}
			cerr << " ... [done]" <<endl;
		}
	}
}

void MainWindow::exportAllContourComponentOfVoxels()
{
	if ( _componentBillon != 0 ) {
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter la composante délimitée par le contour en SDP"), "output.sdp", tr("Fichiers SDP (*.sdp);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if( !file.open(QIODevice::WriteOnly) )
			{
				qDebug() << QObject::tr("ERREUR : Impossible de créer le ficher %1.").arg(fileName);
				return;
			}
			QTextStream stream(&file);
			stream << "#SDP (Sequence of Discrete Points)" << endl;
			std::vector<iCoord3D> voxelSet;
			createVoxelSet(voxelSet);
			for ( unsigned int i=0 ; i<voxelSet.size() ; i++)
			{
			  // invert X/Y (due to compatibility with DGtalViewer
				stream << voxelSet.at(i).y << " " << voxelSet.at(i).x << " " << voxelSet.at(i).z << endl;
			}
			QMessageBox::information(this,"Export nœuds en SDP réussie", "Export réussi !");
		}
	}
}



void MainWindow::exportAllContourComponentOfVoxelsAllIntervals()
{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter la composante délimitée par le contour en SDP"), "output.sdp", tr("Fichiers SDP (*.sdp);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if( !file.open(QIODevice::WriteOnly) )
			{
				qDebug() << QObject::tr("ERREUR : Impossible de créer le ficher %1.").arg(fileName);
				return;
			}

			QTextStream stream(&file);
			stream << "#SDP (Sequence of Discrete Points)" << endl;
			std::vector<iCoord3D> voxelSet;
			createVoxelSetAllIntervals(voxelSet);
			for ( unsigned int i=0 ; i<voxelSet.size() ; i++ )
			{
			  // invert X/Y (due to compatibility with DGtalViewer
				stream << voxelSet.at(i).y << " " << voxelSet.at(i).x << " " << voxelSet.at(i).z << endl;
			}

			QMessageBox::information(this,"Export nœuds en SDP réussie", "Export réussi !");
		}

}




void MainWindow::exportAllContourComponentOfVoxelsAllIntervalsOldMethod()
{
	if ( _componentBillon != 0 )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter la composante délimitée par le contour en SDP"), "output.sdp", tr("Fichiers SDP (*.sdp);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if( !file.open(QIODevice::WriteOnly) )
			{
				qDebug() << QObject::tr("ERREUR : Impossible de créer le ficher %1.").arg(fileName);
				return;
			}

			QTextStream stream(&file);
			stream << "#SDP (Sequence of Discrete Points)" << endl;
			std::vector<iCoord3D> voxelSet;
			createVoxelSetAllIntervals(voxelSet, true);
			for ( unsigned int i=0 ; i<voxelSet.size() ; i++ )
			{
			  // invert X/Y (due to compatibility with DGtalViewer
				stream << voxelSet.at(i).y << " " << voxelSet.at(i).x << " " << voxelSet.at(i).z << endl;
			}

			QMessageBox::information(this,"Export nœuds en SDP réussie", "Export réussi !");
		}
	}
}




/*******************************
 * Private functions
 *******************************/

void MainWindow::openNewBillon( const QString &folderName )
{
	if ( _billon != 0 )
	{
		delete _billon;
		_billon = 0;
	}
	if ( !folderName.isEmpty() )
	{
		_billon = DicomReader::read(folderName);
	}
	if ( _billon != 0 )
	{
		_pix = QImage(_billon->n_cols, _billon->n_rows,QImage::Format_ARGB32);
	}
	else
	{
		_pix = QImage(0,0,QImage::Format_ARGB32);
	}
}

void MainWindow::drawSlice() {
	drawSlice(_currentSlice);
}

void MainWindow::initComponentsValues() {
	_ui->_spansliderIntensityThreshold->setMinimum(MINIMUM_INTENSITY);
	_ui->_spansliderIntensityThreshold->setLowerValue(MINIMUM_INTENSITY);
	_ui->_spansliderIntensityThreshold->setMaximum(MAXIMUM_INTENSITY);
	_ui->_spansliderIntensityThreshold->setUpperValue(MAXIMUM_INTENSITY);

	_ui->_spinMinIntensity->setMinimum(MINIMUM_INTENSITY);
	_ui->_spinMinIntensity->setMaximum(MAXIMUM_INTENSITY);
	_ui->_spinMinIntensity->setValue(MINIMUM_INTENSITY);

	_ui->_spinMaxIntensity->setMinimum(MINIMUM_INTENSITY);
	_ui->_spinMaxIntensity->setMaximum(MAXIMUM_INTENSITY);
	_ui->_spinMaxIntensity->setValue(MAXIMUM_INTENSITY);

	_ui->_spinMinSlice->setMinimum(0);
	_ui->_spinMinSlice->setMaximum(0);
	_ui->_spinMinSlice->setValue(0);

	_ui->_spinMaxSlice->setMinimum(0);
	_ui->_spinMaxSlice->setMaximum(0);
	_ui->_spinMaxSlice->setValue(0);

	_ui->_spanSliderSelectInterval->setMinimum(0);
	_ui->_spanSliderSelectInterval->setMaximum(0);
	_ui->_spanSliderSelectInterval->setLowerValue(0);
	_ui->_spanSliderSelectInterval->setUpperValue(0);

	_ui->_sliderSelectSlice->setValue(0);
	_ui->_sliderSelectSlice->setRange(0,0);

	_ui->_sliderMovementThresholdInterval->setMinimum(0);
	_ui->_sliderMovementThresholdInterval->setMaximum(1000);
	_ui->_sliderMovementThresholdInterval->setLowerValue(0);
	_ui->_sliderMovementThresholdInterval->setUpperValue(MAXIMUM_Z_MOTION);
	_ui->_sliderMovementThresholdInterval->setLowerValue(MINIMUM_Z_MOTION);

	_ui->_sliderMaximumsNeighborhood->setMinimum(0);
	_ui->_sliderMaximumsNeighborhood->setMaximum(50);
	_ui->_sliderMaximumsNeighborhood->setValue(DEFAULT_MINIMUM_WIDTH_OF_NEIGHBORHOOD);

	_ui->_spinMaximumsNeighborhood->setMinimum(0);
	_ui->_spinMaximumsNeighborhood->setMaximum(50);
	_ui->_spinMaximumsNeighborhood->setValue(DEFAULT_MINIMUM_WIDTH_OF_NEIGHBORHOOD);

	_ui->_sliderHistogramIntervalMinimumWidth->setMinimum(0);
	_ui->_sliderHistogramIntervalMinimumWidth->setMaximum(50);
	_ui->_sliderHistogramIntervalMinimumWidth->setValue(DEFAULT_MINIMUM_WIDTH_OF_INTERVALS);

	_ui->_spinHistogramIntervalMinimumWidth->setMinimum(0);
	_ui->_spinHistogramIntervalMinimumWidth->setMaximum(50);
	_ui->_spinHistogramIntervalMinimumWidth->setValue(DEFAULT_MINIMUM_WIDTH_OF_INTERVALS);

	_ui->_spinSectorsNumber->setMinimum(0);
	_ui->_spinSectorsNumber->setMaximum(500);
	_ui->_spinSectorsNumber->setValue(360);
}

void MainWindow::updateUiComponentsValues() {
	int minValue, maxValue, nbSlices;
	const bool existBillon = (_billon != 0);

	if ( existBillon ) {
		minValue = _billon->minValue();
		maxValue = _billon->maxValue();
		nbSlices = _billon->n_slices-1;
		_ui->_labelSliceNumber->setNum(0);
		_ui->_scrollSliceView->setFixedSize(_billon->n_cols,_billon->n_rows);
		_ui->_labelXwidth->setText(tr("Larg. %1").arg(_billon->voxelWidth()));
		_ui->_labelYwidth->setText(tr("Haut.%1").arg(_billon->voxelHeight()));
		_ui->_labelZwidth->setText(tr("Prof. %1").arg(_billon->voxelDepth()));
	}
	else {
		minValue = maxValue = 0;
		nbSlices = 0;
		_ui->_labelSliceNumber->setText(tr("Aucune coupe présente."));
		_ui->_scrollSliceView->setFixedSize(0,0);
		_ui->_labelXwidth->setText("NaN");
		_ui->_labelYwidth->setText("NaN");
		_ui->_labelZwidth->setText("NaN");
	}

	_ui->_spansliderIntensityThreshold->setMinimum(minValue);
	_ui->_spansliderIntensityThreshold->setLowerValue(MINIMUM_INTENSITY);
	_ui->_spansliderIntensityThreshold->setMaximum(maxValue);
	_ui->_spansliderIntensityThreshold->setUpperValue(MAXIMUM_INTENSITY);

	_ui->_spinMinIntensity->setMinimum(minValue);
	_ui->_spinMinIntensity->setMaximum(maxValue);
	_ui->_spinMinIntensity->setValue(MINIMUM_INTENSITY);

	_ui->_spinMaxIntensity->setMinimum(minValue);
	_ui->_spinMaxIntensity->setMaximum(maxValue);
	_ui->_spinMaxIntensity->setValue(MAXIMUM_INTENSITY);

	_ui->_spinMinSlice->setMinimum(0);
	_ui->_spinMinSlice->setMaximum(nbSlices);
	_ui->_spinMinSlice->setValue(0);

	_ui->_spinMaxSlice->setMinimum(0);
	_ui->_spinMaxSlice->setMaximum(nbSlices);
	_ui->_spinMaxSlice->setValue(nbSlices);

	_ui->_spanSliderSelectInterval->setMinimum(0);
	_ui->_spanSliderSelectInterval->setMaximum(nbSlices);
	_ui->_spanSliderSelectInterval->setLowerValue(0);
	_ui->_spanSliderSelectInterval->setUpperValue(nbSlices);

	_ui->_sliderSelectSlice->setValue(0);
	_ui->_sliderSelectSlice->setRange(0,nbSlices);

	_ui->_checkRadiusAroundPith->setText( QString::number(100) );

	enabledComponents();
}

void MainWindow::enabledComponents() {
	const int sliceType = _ui->_comboSliceType->currentIndex();
	const bool enable = (_billon != 0) && ( sliceType == SliceType::CURRENT || sliceType == SliceType::MOVEMENT || sliceType == SliceType::EDGE_DETECTION || sliceType == SliceType::FLOW || sliceType == SliceType::RESTRICTED_AREA );
	_ui->_sliderSelectSlice->setEnabled(enable);
	_ui->_spansliderIntensityThreshold->setEnabled(enable);
	_ui->_buttonComputeMarrow->setEnabled(enable);
	_ui->_buttonUpdateSliceHistogram->setEnabled(enable);
	_ui->_buttonExportSliceHistogram->setEnabled(enable);
	_ui->_buttonMaxSlice->setEnabled(enable);
	_ui->_buttonMinSlice->setEnabled(enable);
	_ui->_buttonExportToDat->setEnabled(enable);
	_ui->_buttonExportToOfs->setEnabled(enable);
	_ui->_buttonNextMaximum->setEnabled(enable);
	_ui->_buttonPreviousMaximum->setEnabled(enable);
}


void MainWindow::computeSectorsHistogramForInterval( const Interval<int> &interval ) {
	_pieChart->setSectorsNumber(_ui->_spinSectorsNumber->value());

	if ( _billon != 0 ) _pieChartDiagrams->compute( *_billon, _marrow, *_pieChart, interval, Interval<int>(_ui->_spinMinIntensity->value(),_ui->_spinMaxIntensity->value()),
													Interval<int>(_ui->_spinMovementThresholdMin->value(),_ui->_spinMovementThresholdMax->value()),
													_ui->_spinSmoothingRadiusOfHistogram->value(), _ui->_spinMinimumHeightofMaximum->value(),
													_ui->_spinMaximumsNeighborhood->value(), _ui->_spinDerivativePercentage->value(),
													_ui->_spinHistogramIntervalMinimumWidth->value(), _ui->_checkRadiusAroundPith->text().toInt());

	_ui->_plotAngularHistogram->replot();
	_ui->_polarSectorSum->replot();
	highlightSectorHistogram(0);
}
