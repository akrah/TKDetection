#include "inc/mainwindow.h"

#include "ui_mainwindow.h"

#include "inc/billon.h"
#include "inc/billonalgorithms.h"
#include "inc/connexcomponentextractor.h"
#include "inc/contourbillon.h"
#include "inc/contourslice.h"
#include "inc/datexport.h"
#include "inc/define.h"
#include "inc/dicomreader.h"
#include "inc/intensitydistributionhistogram.h"
#include "inc/nearestpointshistogram.h"
#include "inc/pith.h"
#include "inc/pithextractor.h"
#include "inc/ofsexport.h"
#include "inc/opticalflow.h"
#include "inc/pgm3dexport.h"
#include "inc/piechart.h"
#include "inc/piepart.h"
#include "inc/plotcontourdistanceshistogram.h"
#include "inc/plotcurvaturehistogram.h"
#include "inc/plotintensitydistributionhistogram.h"
#include "inc/plotnearestpointshistogram.h"
#include "inc/plotsectorhistogram.h"
#include "inc/plotslicehistogram.h"
#include "inc/plotzmotiondistributionhistogram.h"
#include "inc/sectorhistogram.h"
#include "inc/slicealgorithm.h"
#include "inc/slicehistogram.h"
#include "inc/sliceview.h"
#include "inc/v3dexport.h"
#include "inc/v3dreader.h"
#include "inc/zmotiondistributionhistogram.h"

#include <QLabel>
#include <QFileDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QMessageBox>
#include <QInputDialog>
#include <QXmlStreamWriter>

#include <qwt_plot_renderer.h>
#include <qwt_polar_renderer.h>
#include <qwt_polar_grid.h>

MainWindow::MainWindow( QWidget *parent ) : QMainWindow(parent), _ui(new Ui::MainWindow), _labelSliceView(new QLabel), _billon(0), _componentBillon(0), _knotBillon(0),
	_sliceView(new SliceView()), _sliceHistogram(new SliceHistogram()), _plotSliceHistogram(new PlotSliceHistogram()),
	_pieChart(new PieChart(360)), _sectorHistogram(new SectorHistogram()), _plotSectorHistogram(new PlotSectorHistogram()),
	_nearestPointsHistogram(new NearestPointsHistogram()), _plotNearestPointsHistogram(new PlotNearestPointsHistogram()),
	_plotCurvatureHistogram(new PlotCurvatureHistogram()), _plotContourDistancesHistogram(new PlotContourDistancesHistogram()),
	_intensityDistributionHistogram(new IntensityDistributionHistogram()), _plotIntensityDistributionHistogram(new PlotIntensityDistributionHistogram()),
	_zMotionDistributionHistogram(new ZMotionDistributionHistogram()), _plotZMotionDistributionHistogram(new PlotZMotionDistributionHistogram()),
	_contourBillon(new ContourBillon()), _currentSlice(0), _currentYSlice(0), _currentMaximum(0), _currentSector(0), _treeRadius(0)
{
	_ui->setupUi(this);
//	setCorner(Qt::TopLeftCorner,Qt::LeftDockWidgetArea);
//	setCorner(Qt::TopRightCorner,Qt::RightDockWidgetArea);
//	setCorner(Qt::BottomLeftCorner,Qt::LeftDockWidgetArea);
//	setCorner(Qt::BottomRightCorner,Qt::RightDockWidgetArea);
	setWindowTitle("TKDetection");

	// Paramétrisation des composant graphiques
	_labelSliceView->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	_labelSliceView->setScaledContents(true);
	_labelSliceView->installEventFilter(&_sliceZoomer);
	_labelSliceView->installEventFilter(this);

	_ui->_scrollSliceView->setBackgroundRole(QPalette::Dark);
	_ui->_scrollSliceView->setWidget(_labelSliceView);

	_ui->_plotSliceHistogram->enableAxis(QwtPlot::yLeft,false);
	_ui->_plotSectorHistogram->enableAxis(QwtPlot::yLeft,false);

	_ui->_comboSliceType->insertItem(TKD::CURRENT,tr("Coupe originale"));
	_ui->_comboSliceType->insertItem(TKD::MOVEMENT,tr("Coupe de mouvements"));
	_ui->_comboSliceType->insertItem(TKD::EDGE_DETECTION,tr("Coupe de détection de contours"));
	_ui->_comboSliceType->insertItem(TKD::FLOW,tr("Coupe de flots optiques"));
	_ui->_comboSliceType->setCurrentIndex(TKD::CURRENT);

	_ui->_comboEdgeDetectionType->insertItem(TKD::SOBEL,tr("Sobel"));
	_ui->_comboEdgeDetectionType->insertItem(TKD::LAPLACIAN,tr("Laplacian"));
	_ui->_comboEdgeDetectionType->insertItem(TKD::CANNY,tr("Canny"));

	_ui->_spinFlowAlpha->setValue(_sliceView->flowAlpha());
	_ui->_spinFlowEpsilon->setValue(_sliceView->flowEpsilon());
	_ui->_spinFlowMaximumIterations->setValue(_sliceView->flowMaximumIterations());

	// Histogrammes
	_plotSliceHistogram->attach(_ui->_plotSliceHistogram);

	_ui->_plotSectorHistogram->enableAxis(QwtPlot::yLeft,false);
	_ui->_plotSectorHistogram->setAxisScale(QwtPlot::xBottom,0,TWO_PI);
	_ui->_polarSectorHistogram->setScale( QwtPolar::Azimuth, TWO_PI, 0.0 );
	_plotSectorHistogram->attach(_ui->_polarSectorHistogram);
	_plotSectorHistogram->attach(_ui->_plotSectorHistogram);

	QwtPolarGrid *grid = new QwtPolarGrid();
	grid->showAxis(QwtPolar::AxisRight,false);
	grid->showAxis(QwtPolar::AxisBottom,false);
	grid->setMajorGridPen(QPen(Qt::lightGray));
	grid->attach(_ui->_polarSectorHistogram);

	_plotContourDistancesHistogram->attach(_ui->_plotContourDistancesHistogram);
	_plotCurvatureHistogram->attach(_ui->_plotCurvatureHistogram);
	_plotIntensityDistributionHistogram->attach(_ui->_plotIntensityDistributionHistogram);
	_plotNearestPointsHistogram->attach(_ui->_plotNearestPointsHistogram);
	_plotZMotionDistributionHistogram->attach(_ui->_plotZMotionDistributionHistogram);

	/**** Mise en place de la communication MVC ****/

	// Évènements déclenchés par le slider de n° de coupe
	QObject::connect(_ui->_sliderSelectSlice, SIGNAL(valueChanged(int)), this, SLOT(setSlice(int)));
	QObject::connect(_ui->_sliderSelectYSlice, SIGNAL(valueChanged(int)), this, SLOT(setYSlice(int)));
	QObject::connect(_ui->_radioYView, SIGNAL(clicked()), this, SLOT(drawSlice()));
	QObject::connect(_ui->_radioZView, SIGNAL(clicked()), this, SLOT(drawSlice()));
	QObject::connect(_ui->_sliderContour, SIGNAL(valueChanged(int)), this, SLOT(moveContourCursor(int)));

	// Évènements déclenchés par les boutons de sélection de la vue
	QObject::connect(_ui->_comboSliceType, SIGNAL(currentIndexChanged(int)), this, SLOT(setTypeOfView(int)));
	QObject::connect(_ui->_spanSliderZMotionThreshold, SIGNAL(lowerValueChanged(int)), _ui->_spinMinZMotion, SLOT(setValue(int)));
	QObject::connect(_ui->_spinMinZMotion, SIGNAL(valueChanged(int)), _ui->_spanSliderZMotionThreshold, SLOT(setLowerValue(int)));
	QObject::connect(_ui->_spinMinZMotion, SIGNAL(valueChanged(int)), this, SLOT(drawSlice()));
	QObject::connect(_ui->_spanSliderZMotionThreshold, SIGNAL(upperValueChanged(int)), _ui->_spinMaxZMotion, SLOT(setValue(int)));
	QObject::connect(_ui->_spinMaxZMotion, SIGNAL(valueChanged(int)), _ui->_spanSliderZMotionThreshold, SLOT(setUpperValue(int)));
	QObject::connect(_ui->_spinMaxZMotion, SIGNAL(valueChanged(int)), this, SLOT(drawSlice()));
	QObject::connect(_ui->_buttonFlowUpdate, SIGNAL(clicked()), this, SLOT(updateOpticalFlow()));
	QObject::connect(_ui->_comboEdgeDetectionType, SIGNAL(currentIndexChanged(int)), this, SLOT(setEdgeDetectionType(int)));
	QObject::connect(_ui->_spinCannyRadiusOfGaussianMask, SIGNAL(valueChanged(int)), this, SLOT(setCannyRadiusOfGaussianMask(int)));
	QObject::connect(_ui->_spinCannySigmaOfGaussianMask, SIGNAL(valueChanged(double)), this, SLOT(setCannySigmaOfGaussianMask(double)));
	QObject::connect(_ui->_spinCannyMinimumGradient, SIGNAL(valueChanged(int)), this, SLOT(setCannyMinimumGradient(int)));
	QObject::connect(_ui->_spinCannyMinimumDeviation, SIGNAL(valueChanged(double)), this, SLOT(setCannyMinimumDeviation(double)));
	QObject::connect(_ui->_sliderRestrictedAreaPercentage, SIGNAL(valueChanged(int)), _ui->_spinRestrictedAreaPercentage, SLOT(setValue(int)));
	QObject::connect(_ui->_spinRestrictedAreaPercentage, SIGNAL(valueChanged(int)), _ui->_sliderRestrictedAreaPercentage, SLOT(setValue(int)));

	// Évènements déclenchés par le slider de seuillage
	QObject::connect(_ui->_spanSliderIntensityThreshold, SIGNAL(lowerValueChanged(int)), _ui->_spinMinIntensity, SLOT(setValue(int)));
	QObject::connect(_ui->_spinMinIntensity, SIGNAL(valueChanged(int)), _ui->_spanSliderIntensityThreshold, SLOT(setLowerValue(int)));
	QObject::connect(_ui->_spanSliderIntensityThreshold, SIGNAL(upperValueChanged(int)), _ui->_spinMaxIntensity , SLOT(setValue(int)));
	QObject::connect(_ui->_spinMaxIntensity, SIGNAL(valueChanged(int)), _ui->_spanSliderIntensityThreshold, SLOT(setUpperValue(int)));
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
	QObject::connect(_ui->_buttonUpdateIntensityDistributionHistogram, SIGNAL(clicked()), this, SLOT(updateIntensityDistributionHistogram()));
	QObject::connect(_ui->_buttonUpdateZMotionDistributionHistogram, SIGNAL(clicked()), this, SLOT(updateZMotionDistributionHistogram()));

	// Évènements déclenchés par les bouton associès à la moelle
	QObject::connect(_ui->_buttonComputePith, SIGNAL(clicked()), this, SLOT(updatePith()));

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
	QObject::connect(_ui->_spinContourSmoothingRadius, SIGNAL(valueChanged(int)), this, SLOT(drawSlice()));
	QObject::connect(_ui->_sliderCurvatureWidth, SIGNAL(valueChanged(int)), _ui->_spinCurvatureWidth, SLOT(setValue(int)));
	QObject::connect(_ui->_spinCurvatureWidth, SIGNAL(valueChanged(int)), _ui->_sliderCurvatureWidth, SLOT(setValue(int)));

	// Évènements déclenchés par la souris sur le visualiseur de coupes
	QObject::connect(&_sliceZoomer, SIGNAL(zoomFactorChanged(qreal,qreal)), this, SLOT(zoomInSliceView(qreal,qreal)));
	QObject::connect(&_sliceZoomer, SIGNAL(isMovedFrom(QPoint)), this, SLOT(dragInSliceView(QPoint)));

	// Évènements déclenchés par les boutons relatifs aux intervalles de coupes
	QObject::connect(_ui->_spinMinSlice, SIGNAL(valueChanged(int)), _ui->_spanSliderSelectInterval, SLOT(setLowerValue(int)));
	QObject::connect(_ui->_spanSliderSelectInterval, SIGNAL(lowerValueChanged(int)), _ui->_spinMinSlice, SLOT(setValue(int)));
	QObject::connect(_ui->_buttonMinSlice, SIGNAL(clicked()), this, SLOT(setMinimumOfSliceIntervalToCurrentSlice()));
	QObject::connect(_ui->_spinMaxSlice, SIGNAL(valueChanged(int)), _ui->_spanSliderSelectInterval, SLOT(setUpperValue(int)));
	QObject::connect(_ui->_spanSliderSelectInterval, SIGNAL(upperValueChanged(int)), _ui->_spinMaxSlice, SLOT(setValue(int)));
	QObject::connect(_ui->_buttonMaxSlice, SIGNAL(clicked()), this, SLOT(setMaximumOfSliceIntervalToCurrentSlice()));
	QObject::connect(_ui->_spinBlurredSegmentsThickness, SIGNAL(valueChanged(int)), this, SLOT(drawSlice()));

	// Export
	QObject::connect(_ui->_sliderDatExportContrast, SIGNAL(valueChanged(int)), _ui->_spinDatExportContrast, SLOT(setValue(int)));
	QObject::connect(_ui->_spinDatExportContrast, SIGNAL(valueChanged(int)), _ui->_sliderDatExportContrast, SLOT(setValue(int)));
	QObject::connect(_ui->_buttonExportToDat, SIGNAL(clicked()), this, SLOT(exportToDat()));
	QObject::connect(_ui->_buttonExportToOfs, SIGNAL(clicked()), this, SLOT(exportToOfs()));
	QObject::connect(_ui->_buttonExportHistograms, SIGNAL(clicked()), this, SLOT(exportHistograms()));
	QObject::connect(_ui->_buttonExportToSDP, SIGNAL(clicked()), this, SLOT(exportToSdp()));
	QObject::connect(_ui->_sliderPgm3dExportContrast, SIGNAL(valueChanged(int)), _ui->_spinPgm3dExportContrast, SLOT(setValue(int)));
	QObject::connect(_ui->_spinPgm3dExportContrast, SIGNAL(valueChanged(int)), _ui->_sliderPgm3dExportContrast, SLOT(setValue(int)));
	QObject::connect(_ui->_buttonExportToPgm3d, SIGNAL(clicked()), this, SLOT(exportToPgm3D()));
	QObject::connect(_ui->_buttonExportToV3D, SIGNAL(clicked()), this, SLOT(exportToV3D()));

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

MainWindow::~MainWindow()
{
	delete _contourBillon;
	delete _plotZMotionDistributionHistogram;
	delete _plotIntensityDistributionHistogram;
	delete _plotContourDistancesHistogram;
	delete _plotCurvatureHistogram;
	delete _plotNearestPointsHistogram;
	delete _nearestPointsHistogram;
	delete _plotSectorHistogram;
	delete _sectorHistogram;
	delete _pieChart;
	delete _plotSliceHistogram;
	delete _sliceHistogram;
	delete _sliceView;
	if ( _knotBillon != 0 ) delete _knotBillon;
	if ( _componentBillon != 0 ) delete _componentBillon;
	if ( _billon != 0 ) delete _billon;
}


/*******************************
 * Public fonctions
 *******************************/

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
	if ( obj == _labelSliceView && _billon != 0 && _billon->hasPith() )
	{
		if ( event->type() == QEvent::MouseButtonRelease )
		{
			const QMouseEvent *mouseEvent = static_cast<const QMouseEvent*>(event);
			if ( (mouseEvent->button() == Qt::LeftButton) )
			{
				iCoord2D pos = iCoord2D(mouseEvent->x(),mouseEvent->y())/_sliceZoomer.factor();
				qDebug() << "Position (i,j) = " << pos.x << " , " << pos.y << " )";
				_currentSector = _pieChart->sectorIndexOfAngle( _billon->pithCoord(_currentSlice).angle(iCoord2D(mouseEvent->x(),mouseEvent->y())/_sliceZoomer.factor()) );
				_plotSectorHistogram->moveCursor(_currentSector);
				_ui->_plotSectorHistogram->replot();
				_ui->_polarSectorHistogram->replot();
				drawSlice();
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
	if ( !folderName.isEmpty() )
	{
		closeImage();
		openNewBillon(folderName);
		updateUiComponentsValues();
		updateSliceHistogram();
		//updateIntensityDistributionHistogram();
		//updateZMotionDistributionHistogram();
		drawSlice();
		setWindowTitle(QString("TKDetection - %1").arg(folderName.section(QDir::separator(),-1)));
	}
}

void MainWindow::closeImage()
{
	if ( _billon != 0 )
	{
		delete _billon;
		_billon = 0;
	}
	if ( _componentBillon != 0 )
	{
		delete _componentBillon;
		_componentBillon = 0;
	}

	if ( _knotBillon != 0 )
	{
		delete _knotBillon;
		_knotBillon = 0;
	}
	_contourBillon->clear();

	_mainPix = QImage(0,0,QImage::Format_ARGB32);
	_treeRadius = 133.33;
	_ui->_checkRadiusAroundPith->setText( QString::number(_treeRadius) );
	updateSliceHistogram();
	updateContourHistograms(0);

	_sectorHistogram->clear();
	_plotSectorHistogram->update(*_sectorHistogram,*_pieChart);
	_ui->_plotSectorHistogram->replot();
	_ui->_polarSectorHistogram->replot();
	selectSectorInterval(0);
	updateUiComponentsValues();
	drawSlice();
	setWindowTitle("TKDetection");
}

void MainWindow::drawSlice()
{
	if ( _billon != 0 )
	{
		const TKD::ViewType selectedAxe = _ui->_radioYView->isChecked()?TKD::Y_VIEW:TKD::Z_VIEW;
		const uint &currentSlice = _ui->_radioYView->isChecked()?_currentYSlice:_currentSlice;
		uint width, height;

		switch (selectedAxe)
		{
			case TKD::Y_VIEW : width = _billon->n_cols; height = _billon->n_slices; break;
			case TKD::Z_VIEW :
			default : width = _billon->n_cols; height = _billon->n_rows; break;
		}


		_mainPix = QImage(width,height,QImage::Format_ARGB32);
		_mainPix.fill(0xff000000);

		_sliceView->drawSlice(_mainPix,*_billon,_billon->hasPith()?_billon->pithCoord(_currentSlice):iCoord2D(_billon->n_cols/2,_billon->n_rows/2),
							  currentSlice,Interval<int>(_ui->_spinMinIntensity->value(),_ui->_spinMaxIntensity->value()),
							  Interval<int>(_ui->_spinMinZMotion->value(),_ui->_spinMaxZMotion->value()), selectedAxe);

		if ( selectedAxe == TKD::Z_VIEW && _billon->hasPith() )
		{
			_billon->pith().draw(_mainPix,_currentSlice);

			const iCoord2D &pithCoord = _billon->pithCoord(_currentSlice);
			if ( _ui->_checkRadiusAroundPith->isChecked() && _treeRadius > 0 )
			{
				QPainter painter(&_mainPix);
				painter.setPen(Qt::yellow);
				painter.drawEllipse(QPointF(pithCoord.x,pithCoord.y),_treeRadius*_ui->_spinRestrictedAreaPercentage->value()/100.,_treeRadius*_ui->_spinRestrictedAreaPercentage->value()/100.);
			}

			const bool inDrawingArea = (_ui->_comboSelectSliceInterval->currentIndex() > 0 &&
										_sliceHistogram->interval(_ui->_comboSelectSliceInterval->currentIndex()-1).containsClosed(_currentSlice));
			if ( inDrawingArea )
			{
				if ( !_sectorHistogram->isEmpty() )
				{
					_pieChart->draw(_mainPix, pithCoord, _sectorHistogram->intervals());
					_pieChart->draw(_mainPix, pithCoord, _currentSector);
				}
				if ( _componentBillon != 0 )
				{
					const uint slicePosition = _currentSlice-_componentBillon->zPos();
					const Slice &componentSlice = _componentBillon->slice(slicePosition);

					const int &width = componentSlice.n_cols;
					const int &height = componentSlice.n_rows;

					const QColor colors[] = { QColor(0,0,255,127), QColor(255,0,255,127), QColor(255,0,0,127), QColor(255,255,0,127), QColor(0,255,0,127) };
					const int nbColors = sizeof(colors)/sizeof(QColor);

					QPainter painter(&_mainPix);
					int i, j, color;
					for ( j=0 ; j<height ; ++j )
					{
						for ( i=0 ; i<width ; ++i )
						{
							color = componentSlice.at(j,i);
							if ( color )
							{
								painter.setPen(colors[color%nbColors]);
								painter.drawPoint(i,j);
							}
						}
					}

					if ( _ui->_checkEnableConnexComponents->isChecked() && _knotBillon != 0 )
					{
						SliceAlgorithm::draw( painter, _knotBillon->slice(slicePosition), 0 );
						_contourBillon->contourSlice(slicePosition).draw( painter, _ui->_sliderContour->value() );
					}
					painter.end();
				}
			}
		}
	}
	else
	{
		_ui->_labelSliceNumber->setText(tr("Aucune"));
		_mainPix = QImage(1,1,QImage::Format_ARGB32);
	}

	_labelSliceView->setPixmap( QPixmap::fromImage(_mainPix) );
	zoomInSliceView(_sliceZoomer.factor(),_sliceZoomer.coefficient());
}

void MainWindow::setSlice( const int &sliceNumber )
{
	_currentSlice = sliceNumber;
	_ui->_labelSliceNumber->setNum(sliceNumber);

	_plotSliceHistogram->moveCursor(sliceNumber);
	_ui->_plotSliceHistogram->replot();

	moveNearestPointsCursor(sliceNumber);

	updateContourHistograms(sliceNumber);

	drawSlice();
}

void MainWindow::setYSlice( const int &yPosition )
{
	_currentYSlice = yPosition;
	drawSlice();
}

void MainWindow::moveNearestPointsCursor( const int &position )
{
	if ( !_nearestPointsHistogram->intervals().isEmpty()
		 && _ui->_comboSelectSliceInterval->count()>1
		 && _ui->_comboSelectSliceInterval->currentIndex()<static_cast<int>(_sliceHistogram->nbIntervals())
		 && _sliceHistogram->interval(_ui->_comboSelectSliceInterval->currentIndex()-1).containsClosed(position) )
	{
		_plotNearestPointsHistogram->moveCursor(position-_sliceHistogram->interval(_ui->_comboSelectSliceInterval->currentIndex()-1).min());
		_ui->_plotNearestPointsHistogram->replot();
	}
}

void MainWindow::moveContourCursor( const int &position )
{
	if ( !_contourBillon->isEmpty() && _sliceHistogram->interval(_ui->_comboSelectSliceInterval->currentIndex()-1).containsClosed(_currentSlice) )
	{
		_plotCurvatureHistogram->moveCursor(position);
		_ui->_plotCurvatureHistogram->replot();
		_plotContourDistancesHistogram->moveCursor(position);
		_ui->_plotContourDistancesHistogram->replot();
		drawSlice();
	}
}

void MainWindow::setTypeOfView( const int &type )
{
	enabledComponents();
	_sliceView->setTypeOfView( static_cast<const TKD::SliceType>(type) );
	switch (type)
	{
		case TKD::MOVEMENT:
			_ui->_toolboxSliceParameters->setCurrentWidget(_ui->_pageMovementParameters);
			break;
		case TKD::EDGE_DETECTION :
			_ui->_toolboxSliceParameters->setCurrentWidget(_ui->_pageEdgeDetection);
			break;
		case TKD::FLOW:
			_ui->_toolboxSliceParameters->setCurrentWidget(_ui->_pageFlowParameters);
			break;
		default:
			break;
	}
	drawSlice();
}

void MainWindow::updateSliceHistogram()
{
	_sliceHistogram->clear();

	if ( _billon != 0 && _billon->hasPith() )
	{
		_sliceHistogram->construct(*_billon, Interval<int>(_ui->_spinMinIntensity->value(),_ui->_spinMaxIntensity->value()),
								   Interval<int>(_ui->_spinMinZMotion->value(),_ui->_spinMaxZMotion->value()),
								   _ui->_spinBorderPercentageToCut->value(), _treeRadius*_ui->_spinRestrictedAreaPercentage->value()/100.);
		_sliceHistogram->computeMaximumsAndIntervals( _ui->_spinSmoothingRadiusOfHistogram->value(), _ui->_spinMinimumHeightofMaximum->value(),
													  _ui->_spinMaximumsNeighborhood->value(), _ui->_spinDerivativePercentage->value(),
													  _ui->_spinHistogramIntervalMinimumWidth->value(), false);
	}
	_plotSliceHistogram->update( *_sliceHistogram );
	_plotSliceHistogram->moveCursor( _currentSlice );
	_plotSliceHistogram->updatePercentageCurve( _sliceHistogram->thresholdOfMaximums( _ui->_spinMinimumHeightofMaximum->value() ) );
	_ui->_plotSliceHistogram->setAxisScale(QwtPlot::xBottom,0,_sliceHistogram->size());
	_ui->_plotSliceHistogram->replot();

	const int oldIntervalIndex = _ui->_comboSelectSliceInterval->currentIndex();
	_ui->_comboSelectSliceInterval->clear();
	_ui->_comboSelectSliceInterval->addItem(tr("Aucun"));
	const QVector< Interval<uint> > &intervals = _sliceHistogram->intervals();
	if ( !intervals.isEmpty() )
	{
		for ( int i=0 ; i<intervals.size() ; ++i )
		{
			const Interval<uint> &interval = intervals[i];
			_ui->_comboSelectSliceInterval->addItem(tr("Interval %1 : [ %2, %3 ] (%4 coupes)").arg(i).arg(interval.min()).arg(interval.max()).arg(interval.width()+1));
		}
	}
	_ui->_comboSelectSliceInterval->setCurrentIndex(oldIntervalIndex<=intervals.size()?oldIntervalIndex:0);
}

void MainWindow::updateContourHistograms( const int &sliceNumber )
{
	_ui->_sliderContour->setValue(0);
	_ui->_sliderContour->setMaximum(0);
	_plotCurvatureHistogram->clear();
	_plotContourDistancesHistogram->clear();
	const int sliceIntervalIndex = _ui->_comboSelectSliceInterval->currentIndex();
	if ( sliceIntervalIndex > 0 )
	{
		const Interval<uint> &sliceInterval = _sliceHistogram->interval(sliceIntervalIndex-1);
		if ( !_contourBillon->isEmpty() && sliceInterval.containsClosed(_currentSlice) )
		{
			const ContourSlice &contourSlice = _contourBillon->contourSlice(sliceNumber-sliceInterval.min());

			_plotCurvatureHistogram->update(contourSlice.curvatureHistogram(),contourSlice.dominantPointIndex());
			_ui->_plotCurvatureHistogram->setAxisScale(QwtPlot::xBottom,0,contourSlice.curvatureHistogram().size());

			_plotContourDistancesHistogram->update(contourSlice.contourDistancesHistogram(),contourSlice.dominantPointIndex());
			_ui->_plotContourDistancesHistogram->setAxisScale(QwtPlot::xBottom,0,contourSlice.contourDistancesHistogram().size());

			_ui->_sliderContour->setMaximum(contourSlice.curvatureHistogram().size()-1);
			moveContourCursor(0);
		}
	}
	_ui->_plotCurvatureHistogram->replot();
	_ui->_plotContourDistancesHistogram->replot();
}

void MainWindow::updateIntensityDistributionHistogram()
{
	_intensityDistributionHistogram->clear();

	if ( _billon != 0 )
	{
		_intensityDistributionHistogram->construct(*_billon,Interval<int>(_ui->_spinMinIntensity->value(),_ui->_spinMaxIntensity->value()),_ui->_spinSmoothingRadiusOfHistogram->value());
	}

	_plotIntensityDistributionHistogram->update(*_intensityDistributionHistogram,Interval<int>(_ui->_spinMinIntensity->value(),_ui->_spinMaxIntensity->value()));
	_ui->_plotIntensityDistributionHistogram->replot();
}

void MainWindow::updateZMotionDistributionHistogram()
{
	_zMotionDistributionHistogram->clear();

	if ( _billon != 0 )
	{
		_zMotionDistributionHistogram->construct(*_billon,Interval<int>(_ui->_spinMinIntensity->value(),_ui->_spinMaxIntensity->value()),
												 Interval<uint>(_ui->_spinMinZMotion->value(),_ui->_spinMaxZMotion->value()),_ui->_spinSmoothingRadiusOfHistogram->value(),
												 _treeRadius);
	}

	_plotZMotionDistributionHistogram->update(*_zMotionDistributionHistogram,Interval<uint>(_ui->_spinMinZMotion->value(),_ui->_spinMaxZMotion->value()));
	_ui->_plotZMotionDistributionHistogram->replot();
}

void MainWindow::updatePith()
{
	if ( _billon != 0 )
	{
		PithExtractor::instance().process(*_billon);
	}
	_treeRadius = BillonAlgorithms::restrictedAreaMeansRadius(*_billon,_ui->_spinRestrictedAreaResolution->value(),_ui->_spinRestrictedAreaThreshold->value());
	_ui->_checkRadiusAroundPith->setText( QString::number(_treeRadius) );
	drawSlice();
	updateSliceHistogram();
}

void MainWindow::setMinimumOfSliceIntervalToCurrentSlice()
{
	_ui->_spinMinSlice->setValue(_currentSlice);
}

void MainWindow::setMaximumOfSliceIntervalToCurrentSlice()
{
	_ui->_spinMaxSlice->setValue(_currentSlice);
}

void MainWindow::previousMaximumInSliceHistogram()
{
	const uint nbMaximums = _sliceHistogram->maximums().size();
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

void MainWindow::zoomInSliceView( const qreal &zoomFactor, const qreal &zoomCoefficient )
{
	_labelSliceView->resize(zoomFactor * _mainPix.size());
	QScrollBar *hBar = _ui->_scrollSliceView->horizontalScrollBar();
	hBar->setValue(int(zoomCoefficient * hBar->value() + ((zoomCoefficient - 1) * hBar->pageStep()/2)));
	QScrollBar *vBar = _ui->_scrollSliceView->verticalScrollBar();
	vBar->setValue(int(zoomCoefficient * vBar->value() + ((zoomCoefficient - 1) * vBar->pageStep()/2)));
}

void MainWindow::dragInSliceView( const QPoint &movementVector )
{
	QScrollArea &scrollArea = *(_ui->_scrollSliceView);
	if ( movementVector.x() != 0 ) scrollArea.horizontalScrollBar()->setValue(scrollArea.horizontalScrollBar()->value()-movementVector.x());
	if ( movementVector.y() != 0 ) scrollArea.verticalScrollBar()->setValue(scrollArea.verticalScrollBar()->value()-movementVector.y());
}

void MainWindow::updateOpticalFlow()
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

void MainWindow::setEdgeDetectionType( const int &type )
{
	_sliceView->setEdgeDetectionType( static_cast<const TKD::EdgeDetectionType>(type) );
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

void MainWindow::selectSliceInterval( const int &index )
{
	if ( _componentBillon != 0 )
	{
		delete _componentBillon;
		_componentBillon = 0;
	}

	if ( _knotBillon != 0 )
	{
		delete _knotBillon;
		_knotBillon = 0;
	}

	_contourBillon->clear();

	_ui->_comboSelectSectorInterval->clear();
	_ui->_comboSelectSectorInterval->addItem(tr("Aucun"));
	_ui->_spanSliderSelectInterval->setLowerValue(0);
	_ui->_spanSliderSelectInterval->setUpperValue(0);
	if ( index > 0 && index <= static_cast<int>(_sliceHistogram->nbIntervals()) )
	{
		const Interval<uint> &sliceInterval = _sliceHistogram->interval(index-1);
		updateSectorHistogram(sliceInterval);
		_ui->_sliderSelectSlice->setValue(_sliceHistogram->intervalIndex(index-1));

		const QVector< Interval<uint> > &angularIntervals = _sectorHistogram->intervals();
		if ( !angularIntervals.isEmpty() )
		{
			qreal rightAngle, leftAngle;
			for ( int i=0 ; i<angularIntervals.size() ; ++i )
			{
				const Interval<uint> &currentAngularInterval = angularIntervals[i];
				rightAngle = _pieChart->sector(currentAngularInterval.min()).rightAngle()*RAD_TO_DEG_FACT;
				leftAngle = _pieChart->sector(currentAngularInterval.max()).leftAngle()*RAD_TO_DEG_FACT;
				_ui->_comboSelectSectorInterval->addItem(tr("Secteur %1 : [ %2, %3 ] (%4 degres)").arg(i).arg(rightAngle).arg(leftAngle)
														 .arg(currentAngularInterval.isValid()?leftAngle-rightAngle:leftAngle-rightAngle+360.));
			}
		}
		_ui->_spanSliderSelectInterval->setUpperValue(sliceInterval.max());
		_ui->_spanSliderSelectInterval->setLowerValue(sliceInterval.min());
	}
}

void MainWindow::selectCurrentSliceInterval()
{
	selectSliceInterval(_ui->_comboSelectSliceInterval->currentIndex());
}

void MainWindow::selectSectorInterval(const int &index, const bool &draw )
{
	if ( _componentBillon != 0 )
	{
		delete _componentBillon;
		_componentBillon = 0;
	}

	if ( _knotBillon != 0 )
	{
		delete _knotBillon;
		_knotBillon = 0;
	}

	_contourBillon->clear();

	if ( index > 0 && index <= static_cast<int>(_sectorHistogram->nbIntervals()) && _billon->hasPith() )
	{
		const Interval<uint> &sectorInterval = _sectorHistogram->interval(index-1);
		const Interval<uint> &sliceInterval = _sliceHistogram->interval(_ui->_comboSelectSliceInterval->currentIndex()-1);
		const Interval<int> intensityInterval(_ui->_spinSectorThresholding->value(), _ui->_spinMaxIntensity->value());
		const uint &firstSlice = sliceInterval.min();
		const uint &lastSlice = sliceInterval.max();
		const uint &width = _billon->n_cols;
		const uint &height = _billon->n_rows;
		uint i, j, k;

		_componentBillon = new Billon(*_billon,sliceInterval);
		_componentBillon->fill(intensityInterval.min());

		// TODO : Utiliser la copie d'armadillo sur l'intervalle de coupe.
		for ( k=firstSlice ; k<=lastSlice ; ++k )
		{
			const Slice &originalSlice = _billon->slice(k);
			Slice &componentSlice = _componentBillon->slice(k-firstSlice);
			const iCoord2D &pithCoord = _componentBillon->pithCoord(k-firstSlice);
			for ( j=0 ; j<height ; ++j )
			{
				for ( i=0 ; i<width ; ++i )
				{
					if ( intensityInterval.containsOpen(originalSlice.at(j,i)) && sectorInterval.containsClosed(_pieChart->sectorIndexOfAngle(pithCoord.angle(iCoord2D(i,j)))) )
					{
						componentSlice.at(j,i) = originalSlice.at(j,i);
					}
				}
			}
		}

		ConnexComponentExtractor::extractConnexComponents(*_componentBillon,*_componentBillon,qPow(_ui->_spinMinimalSizeOf3DConnexComponents->value(),3),intensityInterval.min());

		for ( k=0 ; k<_componentBillon->n_slices ; ++k )
		{
			ConnexComponentExtractor::extractConnexComponents( _componentBillon->slice(k), _componentBillon->slice(k), qPow(_ui->_spinMinimalSizeOf2DConnexComponents->value(),2), 0 );
		}

		_nearestPointsHistogram->construct( *_componentBillon );
		_nearestPointsHistogram->computeMaximumsAndIntervals( _ui->_spinNearestHistogramNeighborhood->value(), _ui->_spinNearestHistogramDifference->value() );
		_plotNearestPointsHistogram->update( *_nearestPointsHistogram );
		_ui->_plotNearestPointsHistogram->setAxisScale(QwtPlot::xBottom,0,_nearestPointsHistogram->size());
		_ui->_plotNearestPointsHistogram->replot();

		if ( _ui->_checkEnableConnexComponents->isChecked() )
		{
			_knotBillon = new Billon(*_componentBillon);
			_contourBillon->compute( *_knotBillon, *_componentBillon, 0, _ui->_spinBlurredSegmentsThickness->value(), _ui->_spinContourSmoothingRadius->value(),
									 _ui->_spinCurvatureWidth->value(), _ui->_spinMinimumOriginDistance->value() );
		}
	}
	if (draw) drawSlice();
}

void MainWindow::selectCurrentSectorInterval()
{
	selectSectorInterval(_ui->_comboSelectSectorInterval->currentIndex());
}

void MainWindow::exportToDat()
{
	if ( _billon != 0 )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .dat"), "output.dat", tr("Fichiers de données (*.dat);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if ( file.open(QIODevice::WriteOnly) )
			{
				QTextStream stream(&file);
				DatExport::process( stream, *_billon, Interval<int>(_ui->_spinMinSlice->value(),_ui->_spinMaxSlice->value()),
									Interval<int>(_ui->_spinMinIntensity->value(),_ui->_spinMaxIntensity->value()), _ui->_spinDatExportResolution->value(),
									(_ui->_spinDatExportContrast->value()+100.)/100. );
				file.close();
				QMessageBox::information(this,tr("Export en .dat"), tr("Terminé avec succés !"));
			}
			else QMessageBox::warning(this,tr("Export en .dat"), tr("L'export a échoué"));
		}
	}
	else QMessageBox::warning(this,tr("Export en .dat"), tr("Aucun fichier de billon ouvert."));
}

void MainWindow::exportToOfs()
{
	if ( _billon != 0 )
	{
		switch (_ui->_comboOfsExportType->currentIndex())
		{
			case TKD::PITH:
				exportPithToOfs();
				break;
			case TKD::BILLON_RESTRICTED_AREA:
				exportBillonRestrictedAreaToOfs();
				break;
			case TKD::CURENT_ANGULAR_SECTOR_LARGE_AREA:
				exportCurrentAngularSectorLargeAreaToOfs();
				break;
			case TKD::ALL_ANGULAR_SECTORS_ALL_SLICE_INTERVALS_LARGE_AREA:
				exportAllAngularSectorsOfAllSliceIntervalsLargeAreaToOfs();
				break;
			default:
				QMessageBox::warning(this,tr("Export en .ofs"), tr("Contenu à exporter inconnu."));
				break;
		}
	}
	else QMessageBox::warning(this,tr("Export en .ofs"), tr("Aucun fichier de billon ouvert."));
}

void MainWindow::exportHistograms()
{
	int type = _ui->_comboExportHistoType->currentIndex();
	int format = _ui->_comboExportHistoFormat->currentIndex();
	switch (type)
	{
		case TKD::SLICE_HISTOGRAM:
			switch ( format )
			{
				case 0: exportSliceHistogramToSep(); break;
				case 1: exportSliceHistogramToImage(); break;
				default: QMessageBox::warning(this,tr("Exporter l'histogramme de coupes"),tr("L'export a échoué : format inconnu.")); break;
			}
			break;
		case TKD::SECTOR_HISTOGRAM:
			switch ( format )
			{
				case 0: exportSectorHistogramToSep(); break;
				case 1: exportSectorHistogramToImage(); break;
				default: QMessageBox::warning(this,tr("Exporter l'histogramme de secteurs"),tr("L'export a échoué : format inconnu.")); break;
			}
			break;
		case TKD::PITH_KNOT_DISTANCE_HISTOGRAM:
			switch ( format )
			{
				case 0: exportKnotHistogramToSep(); break;
				case 1: exportknotHistogramToImage(); break;
				default: QMessageBox::warning(this,tr("Exporter l'histogramme de zone de nœuds"),tr("L'export a échoué : format inconnu.")); break;
			}
			break;
		default: QMessageBox::warning(this,tr("Exporter les histogramme"),tr("L'histogramme demandé n'est pas prévu pour l'export.")); break;
	}

}

void MainWindow::exportToPgm3D()
{
	int type = _ui->_comboExportPgm3dType->currentIndex();
	switch (type)
	{
		case 0: exportSegmentedKnotsOfCurrentSliceIntervalToPgm3d();	break;
		case 1: exportCurrentSegmentedKnotToPgm3d();	break;
		default: break;
	}
}

void MainWindow::exportToV3D()
{
	int type = _ui->_comboExportToV3DType->currentIndex();
	switch (type)
	{
		case 0: exportCurrentSegmentedKnotToV3D();	break;
		case 1: exportSegmentedKnotsOfCurrentSliceIntervalToV3D();	break;
		case 2: exportAllSegmentedKnotsOfBillonToV3D(); break;
		default: break;
	}
}

void MainWindow::exportToSdp()
{
	switch ( _ui->_comboExportToSdpType->currentIndex() )
	{
		case 0 : exportContourToSdp();	break;
		case 1 : exportCurrentSegmentedKnotToSdp();	break;
		case 2 : exportSegmentedKnotsOfCurrentSliceIntervalToSdp();	break;
		case 3 : exportAllSegmentedKnotsOfBillonToSdp(); break;
		default : break;
	}
}

/*******************************
 * Private functions
 *******************************/

void MainWindow::openNewBillon( const QString &fileName )
{
	if ( _billon != 0 )
	{
		delete _billon;
		_billon = 0;
	}
	if ( !fileName.isEmpty() )
	{
		_billon = DicomReader::read(fileName);
	}
	if ( _billon != 0 )
	{
		_mainPix = QImage(_billon->n_cols, _billon->n_rows,QImage::Format_ARGB32);
	}
	else
	{
		_mainPix = QImage(0,0,QImage::Format_ARGB32);
	}
}

void MainWindow::initComponentsValues() {
	_ui->_spanSliderIntensityThreshold->setMinimum(MINIMUM_INTENSITY);
	_ui->_spanSliderIntensityThreshold->setLowerValue(MINIMUM_INTENSITY);
	_ui->_spanSliderIntensityThreshold->setMaximum(MAXIMUM_INTENSITY);
	_ui->_spanSliderIntensityThreshold->setUpperValue(MAXIMUM_INTENSITY);

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

	_ui->_sliderSelectYSlice->setValue(0);
	_ui->_sliderSelectYSlice->setRange(0,0);

	_ui->_spanSliderZMotionThreshold->setMinimum(0);
	_ui->_spanSliderZMotionThreshold->setMaximum(1000);
	_ui->_spanSliderZMotionThreshold->setLowerValue(0);
	_ui->_spanSliderZMotionThreshold->setUpperValue(MAXIMUM_Z_MOTION);
	_ui->_spanSliderZMotionThreshold->setLowerValue(MINIMUM_Z_MOTION);

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

void MainWindow::updateUiComponentsValues()
{
	int minValue, maxValue, nbSlices, height;
	const bool existBillon = (_billon != 0);

	if ( existBillon )
	{
		minValue = _billon->minValue();
		maxValue = _billon->maxValue();
		nbSlices = _billon->n_slices-1;
		height = _billon->n_rows-1;
		_ui->_labelSliceNumber->setNum(0);
		_ui->_statusBar->showMessage( tr("Dimensions de voxels (largeur, hauteur, profondeur) : ( %1, %2, %3 )").arg(_billon->voxelWidth()).arg(_billon->voxelHeight())
									  .arg(_billon->voxelDepth()) );
	}
	else
	{
		minValue = maxValue = 0;
		nbSlices = height = 0;
		_ui->_labelSliceNumber->setText(tr("Aucune coupe présente."));
		_ui->_statusBar->clearMessage();
	}

	_ui->_spanSliderIntensityThreshold->setMinimum(minValue);
	_ui->_spanSliderIntensityThreshold->setLowerValue(MINIMUM_INTENSITY);
	_ui->_spanSliderIntensityThreshold->setMaximum(maxValue);
	_ui->_spanSliderIntensityThreshold->setUpperValue(MAXIMUM_INTENSITY);

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

	_ui->_sliderSelectYSlice->setValue(0);
	_ui->_sliderSelectYSlice->setRange(0,height);

	_ui->_checkRadiusAroundPith->setText( QString::number(_treeRadius) );

	enabledComponents();
}

void MainWindow::enabledComponents()
{
	const bool enable = (_billon != 0);
	_ui->_sliderSelectSlice->setEnabled(enable);
	_ui->_sliderSelectYSlice->setEnabled(enable);
	_ui->_spanSliderIntensityThreshold->setEnabled(enable);
	_ui->_buttonComputePith->setEnabled(enable);
	_ui->_buttonUpdateSliceHistogram->setEnabled(enable);
	_ui->_buttonExportHistograms->setEnabled(enable);
	_ui->_buttonMaxSlice->setEnabled(enable);
	_ui->_buttonMinSlice->setEnabled(enable);
	_ui->_buttonExportToDat->setEnabled(enable);
	_ui->_buttonExportToOfs->setEnabled(enable);
	_ui->_buttonNextMaximum->setEnabled(enable);
	_ui->_buttonPreviousMaximum->setEnabled(enable);
}


void MainWindow::updateSectorHistogram( const Interval<uint> &interval )
{
	_pieChart->setSectorsNumber(_ui->_spinSectorsNumber->value());
	_sectorHistogram->clear();

	if ( _billon != 0 )
	{
		_sectorHistogram->construct( *_billon, *_pieChart, interval, Interval<int>(_ui->_spinMinIntensity->value(),_ui->_spinMaxIntensity->value()),
									 Interval<int>(_ui->_spinMinZMotion->value(),_ui->_spinMaxZMotion->value()),
									 _treeRadius*_ui->_spinRestrictedAreaPercentage->value()/100., _ui->_spinSectorHistogramIntervalGap->value());
		_sectorHistogram->computeMaximumsAndIntervals( _ui->_spinSmoothingRadiusOfHistogram->value(), _ui->_spinMinimumHeightofMaximum->value(),
													   _ui->_spinMaximumsNeighborhood->value(), _ui->_spinDerivativePercentage->value(),
													   _ui->_spinHistogramIntervalMinimumWidth->value(), true );
	}

	_plotSectorHistogram->update(*_sectorHistogram, *_pieChart);
	_ui->_plotSectorHistogram->replot();
	_ui->_polarSectorHistogram->replot();
	drawSlice();
}

void MainWindow::exportPithToOfs()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .ofs"), "output.ofs", tr("Fichiers de données (*.ofs);;Tous les fichiers (*.*)"));
	if ( !fileName.isEmpty() )
	{
		OfsExport::process( *_billon, Interval<int>(_ui->_spinMinSlice->value(),_ui->_spinMaxSlice->value()), fileName,
							_ui->_spinExportNbEdges->value(), _ui->_spinExportRadius->value(), false );
		QMessageBox::information( this, tr("Export en .ofs"), tr("Terminé avec succés !"));
	}
}

void MainWindow::exportBillonRestrictedAreaToOfs()
{
	if ( _billon->hasPith() )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .ofs"), "output.ofs", tr("Fichiers de données (*.ofs);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			OfsExport::processOnRestrictedMesh( *_billon, Interval<uint>(_ui->_spinMinSlice->value(),_ui->_spinMaxSlice->value()),
												fileName, _ui->_spinRestrictedAreaResolution->value(), _ui->_spinRestrictedAreaThreshold->value(), false, _ui->_checkCloseBillon->isChecked() );
			QMessageBox::information( this, tr("Export en .ofs"), tr("Terminé avec succés !"));
		}
	}
	else QMessageBox::warning( this, tr("Export en .ofs"), tr("La moelle n'est pas calculée."));
}

void MainWindow::exportCurrentAngularSectorLargeAreaToOfs()
{
	uint index = _ui->_comboSelectSectorInterval->currentIndex();
	if ( _billon->hasPith() && index > 0 && index <= _sectorHistogram->nbIntervals() )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .ofs"), "output.ofs", tr("Fichiers de données (*.ofs);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			const Interval<uint> &sectorInterval = _sectorHistogram->interval(_ui->_comboSelectSectorInterval->currentIndex()-1);
			const Interval<uint> &slicesInterval = _sliceHistogram->interval(_ui->_comboSelectSliceInterval->currentIndex()-1);
			OfsExport::processOnSector( *_billon, slicesInterval, fileName, _pieChart->sector(sectorInterval.min()).rightAngle(),
										_pieChart->sector(sectorInterval.max()).leftAngle(), _ui->_spinExportNbEdges->value() );
			QMessageBox::information(this,tr("Export en .ofs"), tr("Terminé avec succés !"));
		}
	}
	else QMessageBox::warning(this,tr("Export en .ofs"), tr("Impossible  d'exporter car la moelle n'est pas calculée ou aucun secteur angulaire n'est sélectionné."));
}

void MainWindow::exportAllAngularSectorsOfAllSliceIntervalsLargeAreaToOfs()
{
	if ( _billon->hasPith() && _ui->_comboSelectSliceInterval->count() > 0 )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter en .ofs"), "output.ofs", tr("Fichiers de données (*.ofs);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QVector< QPair< Interval<uint>, QPair<qreal,qreal> > > intervals;
			for ( int i=0 ; i<_ui->_comboSelectSliceInterval->count()-1 ; i++ )
			{
				const Interval<uint> &slicesInterval = _sliceHistogram->interval(i);
				for ( int j=0 ; j<_ui->_comboSelectSectorInterval->count()-1 ; j++ )
				{
					const Interval<uint> &sectorInterval = _sectorHistogram->interval(j);
					const QPair<qreal,qreal> angles( _pieChart->sector(sectorInterval.min()).rightAngle(), _pieChart->sector(sectorInterval.max()).leftAngle() );
					intervals.append( QPair< Interval<uint>, QPair<qreal,qreal> >( slicesInterval, angles ) );
				}
			}
			OfsExport::processOnAllSectorInAllIntervals( *_billon, intervals, fileName, _ui->_spinExportNbEdges->value() );
			QMessageBox::information(this,tr("Export en .ofs"), tr("Terminé avec succés !"));
		}
	}
	else QMessageBox::warning(this,tr("Export en .ofs"), tr("Impossible car la moelle et/ou les interalles de coupes ne sont pas calculés."));
}

void MainWindow::exportSliceHistogramToSep()
{
	if ( _sliceHistogram->size() != 0 )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter l'histogramme de coupes en .sep"), "output.sep",
														tr("Fichiers séquences de point euclidiens (*.sep);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if ( file.open(QIODevice::WriteOnly) )
			{
				QTextStream stream(&file);
				stream << *_sliceHistogram;
				file.close();
				QMessageBox::information(this,tr("Exporter l'histogramme de coupes en .sep"), tr("Terminé avec succés !"));
			}
			else QMessageBox::warning(this,tr("Exporter l'histogramme de coupes en .sep"),tr("L'export a échoué : impossible de créer le fichier."));
		}
	}
	else QMessageBox::warning(this,tr("Exporter l'histogramme de coupes en .sep"),tr("L'export a échoué : l'histogramme de coupes n'est pas calculé."));
}

void MainWindow::exportSectorHistogramToSep()
{
	if ( _sectorHistogram->size() != 0 )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter l'histogramme de secteurs en .sep"), "output.sep",
														tr("Fichiers séquences de point euclidiens (*.sep);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if ( file.open(QIODevice::WriteOnly) )
			{
				QTextStream stream(&file);
				stream << *_sectorHistogram;
				file.close();
				QMessageBox::information(this,tr("Exporter l'histogramme de secteurs en .sep"), tr("Terminé avec succés !"));
			}
			else QMessageBox::warning(this,tr("Exporter l'histogramme de secteurs en .sep"),tr("L'export a échoué : impossible de créer le fichier."));
		}
	}
	else QMessageBox::warning(this,tr("Exporter l'histogramme de secteurs en .sep"),tr("L'export a échoué : l'histogramme de secteurs n'est pas calculé."));
}

void MainWindow::exportKnotHistogramToSep()
{
	if ( _nearestPointsHistogram->size() != 0 )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter l'histogramme de la zone de nœuds en .sep"), "output.sep",
														tr("Fichiers séquences de point euclidiens (*.sep);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if ( file.open(QIODevice::WriteOnly) )
			{
				QTextStream stream(&file);
				stream << *_nearestPointsHistogram;
				file.close();
				QMessageBox::information(this,tr("Exporter l'histogramme de la zone de nœuds en .sep"), tr("Terminé avec succés !"));
			}
			else QMessageBox::warning(this,tr("Exporter l'histogramme de la zone de nœuds en .sep"),tr("L'export a échoué : impossible de créer le fichier."));
		}
	}
	else QMessageBox::warning(this,tr("Exporter l'histogramme de la zone de nœuds en .sep"),tr("L'export a échoué : l'histogramme de la zone de nœuds n'est pas calculé."));
}

void MainWindow::exportSliceHistogramToImage()
{
	QString fileName;
	QwtPlotRenderer histoRenderer;
	QMessageBox::StandardButton button;
	QLabel label1;
	QPixmap image1;
	int sizeFact;
	bool sizeOk, abort;
	sizeOk = abort = false;

	if ( _sliceHistogram->size() != 0 )
	{
		while (!sizeOk && !abort)
		{
			sizeFact = QInputDialog::getInt(this,tr("Taille de l'image"), tr("Pourcentage"), 100, 10, 100, 1, &sizeOk);
			if ( sizeOk )
			{
				image1 = QPixmap( 1240*sizeFact/100 , 874*sizeFact/100 );
				image1.fill();

				_ui->_plotSliceHistogram->setAxisTitle(QwtPlot::xBottom,tr("Indice de la coupe"));
				_ui->_plotSliceHistogram->enableAxis(QwtPlot::yLeft);
				_ui->_plotSliceHistogram->setAxisTitle(QwtPlot::yLeft,tr("Cumul du z-mouvement"));

				histoRenderer.renderTo(_ui->_plotSliceHistogram,image1);

				image1 = image1.scaledToHeight(600,Qt::SmoothTransformation);
				label1.setPixmap(image1);
				label1.show();

				button = QMessageBox::question(&label1,tr("Taille correcte"),tr("La taille de l'image est-elle correcte ?"),QMessageBox::Abort|QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
				switch ( button )
				{
					case QMessageBox::Yes:
						fileName = QFileDialog::getSaveFileName(&label1, tr("Export de l'histogramme de coupes en image"), "output.pdf",
																tr("Fichiers PDF (*.pdf);;Fichiers PS (*.ps);;Fichiers PNG (*.png);;Fichiers SVG (*.svg);;Tous les fichiers (*.*)"));
						if ( !fileName.isEmpty() )
						{
							histoRenderer.renderDocument(_ui->_plotSliceHistogram,fileName,QSize(297*sizeFact/100,210*sizeFact/100),100);
							QMessageBox::information(this,tr("Export de l'histogramme de coupes en image"), tr("Terminé avec succés !"));
						}
						else QMessageBox::warning(this,tr("Export de l'histogramme de coupes en image"), tr("Impossible de créer le fichier."));
						sizeOk = true;
						break;
					case QMessageBox::No:
						sizeOk = false;
						break;
					case QMessageBox::Abort:
					default :
						abort = true;
						break;
				}

				_ui->_plotSliceHistogram->setAxisTitle(QwtPlot::xBottom,"");
				_ui->_plotSliceHistogram->setAxisTitle(QwtPlot::yLeft,"");
				_ui->_plotSliceHistogram->enableAxis(QwtPlot::yLeft,false);
			}
			else
			{
				abort = true;
				QMessageBox::warning(this,tr("Export de l'histogramme de coupes en image"), tr("Erreur lors de la saisie de la taille de l'image."));
			}
		}
	}
	else QMessageBox::warning(this,tr("Export de l'histogramme de coupes en image"),tr("L'export a échoué : l'histogramme de coupes n'est pas calculé."));
}

void MainWindow::exportSectorHistogramToImage()
{
	QString fileName, chemin, name;
	QwtPlotRenderer histoRenderer;
	QwtPolarRenderer diagramRenderer;
	QMessageBox::StandardButton button;
	QLabel label1, label2;
	QPixmap image1, image2;
	QSize imageSize;
	int sizeFact;
	bool sizeOk, abort;
	sizeOk = abort = false;

	if ( _sectorHistogram->size() != 0 )
	{
		while (!sizeOk && !abort)
		{
			sizeFact = QInputDialog::getInt(this,tr("Taille de l'image"), tr("Pourcentage"), 100, 10, 100, 1, &sizeOk);
			if ( sizeOk )
			{
				imageSize = QSize(297*sizeFact/100,210*sizeFact/100);
				image1 = QPixmap( 1240*sizeFact/100 , 874*sizeFact/100 );
				image1.fill();
				image2 = QPixmap( 1240*sizeFact/100 , 874*sizeFact/100 );
				image2.fill();

				_ui->_plotSectorHistogram->setAxisTitle(QwtPlot::xBottom,tr("Secteur angulaire en radians"));
				_ui->_plotSectorHistogram->enableAxis(QwtPlot::yLeft);
				_ui->_plotSectorHistogram->setAxisTitle(QwtPlot::yLeft,tr("Cumul du z-mouvement"));

				histoRenderer.renderTo(_ui->_plotSectorHistogram,image1);
				diagramRenderer.renderTo(_ui->_polarSectorHistogram,image2);

				image1 = image1.scaledToHeight(600,Qt::SmoothTransformation);
				label1.setPixmap(image1);
				label1.show();
				image2 = image2.scaledToHeight(600,Qt::SmoothTransformation);
				label2.setPixmap(image2);
				label2.show();

				button = QMessageBox::question(&label1,tr("Taille correcte"),tr("La taille de l'image est-elle correcte ?"),QMessageBox::Abort|QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
				switch ( button )
				{
					case QMessageBox::Yes:
						fileName = QFileDialog::getSaveFileName(&label1, tr("Export de l'histogramme de secteurs en image"), "output.pdf",
																tr("Fichiers PDF (*.pdf);;Fichiers PS (*.ps);;Fichiers PNG (*.png);;Fichiers SVG (*.svg);;Tous les fichiers (*.*)"));
						if ( !fileName.isEmpty() )
						{
							chemin = fileName.section(QDir::separator(),0,-2)+QDir::separator();
							name = fileName.section(QDir::separator(),-1);
							histoRenderer.renderDocument(_ui->_plotSectorHistogram, chemin+"histo_"+name, imageSize, 100);
							diagramRenderer.renderDocument(_ui->_polarSectorHistogram, chemin+"diag_"+name, imageSize, 100);
							QMessageBox::information(this,tr("Export de l'histogramme de secteurs en image"), tr("Terminé avec succés !"));
						}
						else QMessageBox::warning(this,tr("Export de l'histogramme de secteurs en image"), tr("Impossible de créer les fichiers."));
						sizeOk = true;
						break;
					case QMessageBox::No:
						sizeOk = false;
						break;
					case QMessageBox::Abort:
					default :
						abort = true;
						break;
				}

				_ui->_plotSectorHistogram->setAxisTitle(QwtPlot::xBottom,"");
				_ui->_plotSectorHistogram->setAxisTitle(QwtPlot::yLeft,"");
				_ui->_plotSectorHistogram->enableAxis(QwtPlot::yLeft,false);
			}
			else
			{
				abort = true;
				QMessageBox::warning(this,tr("Export de l'histogramme de secteurs en image"), tr("Erreur lors de la saisie de la taille de l'image."));
			}
		}
	}
	else QMessageBox::warning(this,tr("Export de l'histogramme de secteurs en image"),tr("L'export a échoué : l'histogramme de secteurs n'est pas calculé."));
}

void MainWindow::exportknotHistogramToImage()
{
	QString fileName;
	QwtPlotRenderer histoRenderer;
	QMessageBox::StandardButton button;
	QLabel label1;
	QPixmap image1;
	int sizeFact;
	bool sizeOk, abort;
	sizeOk = abort = false;

	if ( _nearestPointsHistogram->size() != 0 )
	{
		while (!sizeOk && !abort)
		{
			sizeFact = QInputDialog::getInt(this,tr("Taille de l'image"), tr("Pourcentage"), 100, 10, 100, 1, &sizeOk);
			if ( sizeOk )
			{
				image1 = QPixmap( 1240*sizeFact/100 , 874*sizeFact/100 );
				image1.fill();

				_ui->_plotNearestPointsHistogram->setAxisTitle(QwtPlot::xBottom,tr("Slice index"));
				_ui->_plotNearestPointsHistogram->setAxisTitle(QwtPlot::yLeft,tr("Distance to the pith"));

				histoRenderer.renderTo(_ui->_plotNearestPointsHistogram,image1);

				image1 = image1.scaledToHeight(600,Qt::SmoothTransformation);
				label1.setPixmap(image1);
				label1.show();

				button = QMessageBox::question(&label1,tr("Taille correcte"),tr("La taille de l'image est-elle correcte ?"),QMessageBox::Abort|QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes);
				switch ( button )
				{
					case QMessageBox::Yes:
						fileName = QFileDialog::getSaveFileName(&label1, tr("Export de l'histogramme de zone de nœuds en image"), "output.pdf",
																tr("Fichiers PDF (*.pdf);;Fichiers PS (*.ps);;Fichiers PNG (*.png);;Fichiers SVG (*.svg);;Tous les fichiers (*.*)"));
						if ( !fileName.isEmpty() )
						{
							histoRenderer.renderDocument(_ui->_plotNearestPointsHistogram,fileName,QSize(297*sizeFact/100,140*sizeFact/100),100);
							QMessageBox::information(this,tr("Export de l'histogramme de zone de nœuds en image"), tr("Terminé avec succés !"));
						}
						else QMessageBox::warning(this,tr("Export de l'histogramme de zone de nœuds en image"), tr("Impossible de créer le fichier."));
						sizeOk = true;
						break;
					case QMessageBox::No:
						sizeOk = false;
						break;
					case QMessageBox::Abort:
					default :
						abort = true;
						break;
				}

				_ui->_plotSliceHistogram->setAxisTitle(QwtPlot::xBottom,"");
				_ui->_plotSliceHistogram->setAxisTitle(QwtPlot::yLeft,"");
				_ui->_plotSliceHistogram->enableAxis(QwtPlot::yLeft,false);
			}
			else
			{
				abort = true;
				QMessageBox::warning(this,tr("Export de l'histogramme de zone de nœuds en image"), tr("Erreur lors de la saisie de la taille de l'image."));
			}
		}
	}
	else QMessageBox::warning(this,tr("Export de l'histogramme de zone de nœuds en image"),tr("L'export a échoué : l'histogramme de zone de nœuds n'est pas calculé."));
}

void MainWindow::exportCurrentSegmentedKnotToPgm3d()
{
	if ( _knotBillon )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter le nœud courant segmenté en PGM3D"), "output.pgm3d", tr("Fichiers PGM3D (*.pgm3d);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if( file.open(QIODevice::WriteOnly) )
			{
				QTextStream stream(&file);
				stream << "P3D" << endl;
				stream << "#!VoxelDim " << _knotBillon->voxelWidth() << ' ' << _knotBillon->voxelHeight() << ' ' << _knotBillon->voxelDepth() << endl;
				stream << _knotBillon->n_cols << " " << _knotBillon->n_rows << " " << _knotBillon->n_slices << endl;
				stream << 1 << endl;

				QDataStream dstream(&file);
				for ( uint k=_nearestPointsHistogram->interval(0).min() ; k<=_nearestPointsHistogram->interval(0).max() ; ++k )
				{
					SliceAlgorithm::writeInPgm3D( _knotBillon->slice(k) , dstream );
				}

				file.close();
				QMessageBox::information(this,"Exporter le nœud courant segmenté en PGM3D", "Export réussi !");
			}
			else QMessageBox::warning(this,tr("Exporter le nœud courant segmenté en PGM3D"),tr("L'export a échoué : impossible de créer le ficher %1.").arg(fileName));
		}
	}
	else QMessageBox::warning(this,tr("Exporter le nœud courant segmenté en PGM3D"),tr("L'export a échoué : le contour n'est pas calculé."));
}

void MainWindow::exportSegmentedKnotsOfCurrentSliceIntervalToPgm3d()
{
	if ( _billon != 0 && _billon->hasPith() && _ui->_comboSelectSliceInterval->currentIndex() > 0 )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter la zone de nœuds courante en PGM3D"), "output.pgm3d", tr("Fichiers de données (*.pgm3d);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if( file.open(QIODevice::WriteOnly) )
			{
				Billon billonToWrite( _billon->n_cols, _billon->n_rows, _sliceHistogram->interval(_ui->_comboSelectSliceInterval->currentIndex()-1).size()+1 );
				billonToWrite.fill(0);

				QTextStream stream(&file);
				stream << "P3D" << endl;
				stream << "#!VoxelDim " << _billon->voxelWidth() << ' ' << _billon->voxelHeight() << ' ' << _billon->voxelDepth() << endl;
				stream << billonToWrite.n_cols << " " << billonToWrite.n_rows << " " << billonToWrite.n_slices << endl;
				stream << _sectorHistogram->size() << endl;

				uint i, j, k;
				for ( int sectorIndex=1 ; sectorIndex< _ui->_comboSelectSectorInterval->count() ; ++sectorIndex )
				{
					selectSectorInterval(sectorIndex,false);
					if ( _knotBillon && _nearestPointsHistogram->intervals().size()>0 )
					{
						for ( k=_nearestPointsHistogram->interval(0).min() ; k<=_nearestPointsHistogram->interval(0).max() ; ++k )
						{
							const Slice &knotSlice = _knotBillon->slice(k);
							Slice &slice = billonToWrite.slice(k);
							for ( j=0 ; j<slice.n_rows ; ++j )
							{
								for ( i=0 ; i<slice.n_cols ; ++i )
								{
									if (knotSlice.at(j,i)) slice.at(j,i) = sectorIndex;
								}
							}
						}

					}
				}

				QDataStream dstream(&file);
				for ( k=0 ; k<billonToWrite.n_slices ; ++k )
				{
					Slice &slice = billonToWrite.slice(k);
					for ( j=0 ; j<slice.n_rows ; ++j )
					{
						for ( i=0 ; i<slice.n_cols ; ++i )
						{
							dstream << (qint16)(slice.at(j,i));
						}
					}
				}

				file.close();
				QMessageBox::information(this,"Exporter la zone de nœuds courante en PGM3D", "Export réussi !");
			}
			else QMessageBox::warning(this,tr("Exporter le nœud courant segmenté en PGM3D"),tr("L'export a échoué : impossible de créer le ficher %1.").arg(fileName));
		}
	}
	else QMessageBox::warning(this,tr("Exporter de la zone de nœuds courante en PGM3D"),tr("L'export a échoué : aucun intervalle angulaire sélectionné."));
}

void MainWindow::exportCurrentSegmentedKnotToV3D()
{
	if ( _knotBillon )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter le nœud courant segmenté en V3D"), "output.v3d", tr("Fichiers V3D (*.v3d);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if( file.open(QIODevice::WriteOnly) )
			{
				V3DExport::process( file, *_knotBillon, _nearestPointsHistogram->interval(0) );
				file.close();

				QMessageBox::information(this,"Exporter le nœud courant segmenté en V3D", "Export réussi !");
			}
			else QMessageBox::warning(this,tr("Exporter le nœud courant segmenté en V3D"),tr("L'export a échoué : impossible de créer le ficher %1.").arg(fileName));
		}
	}
	else QMessageBox::warning(this,tr("Exporter le nœud courant segmenté en V3D"),tr("L'export a échoué : le nœud n'est pas segmenté."));
}

void MainWindow::exportSegmentedKnotsOfCurrentSliceIntervalToV3D()
{
	if ( _billon != 0 && _billon->hasPith() && _ui->_comboSelectSliceInterval->currentIndex() > 0 )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter les nœuds segmentés de l'intervalle de coupe courant en V3D"), "output.v3d",
														tr("Fichiers V3D (*.v3d);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if ( file.open(QIODevice::WriteOnly) )
			{
				QXmlStreamWriter stream(&file);
				V3DExport::init(file,stream);

				V3DExport::appendTags(stream,*_billon );
				V3DExport::appendPith(stream,*_billon );

				int sectorIndex;

				V3DExport::startComponents(stream);
				for ( sectorIndex=1 ; sectorIndex< _ui->_comboSelectSectorInterval->count() ; ++sectorIndex )
				{
					selectSectorInterval(sectorIndex,false);
					if ( _knotBillon && _nearestPointsHistogram->intervals().size()>0 )
					{
						V3DExport::appendComponent( stream, *_knotBillon, _nearestPointsHistogram->interval(0), sectorIndex );
					}
				}
				V3DExport::endComponents(stream);

				V3DExport::close(stream);

				file.close();

				QMessageBox::information(this,"Exporter les nœuds segmentés de l'intervalle de coupe courant en V3D", "Export réussi !");
			}
			else QMessageBox::warning(this,tr("Exporter les nœuds segmentés de l'intervalle de coupe courant en V3D"),tr("L'export a échoué : impossible de créer le ficher %1.").arg(fileName));
		}
	}
	else QMessageBox::warning(this,tr("Exporter les nœuds segmentés de l'intervalle de coupe courant en V3D"),tr("L'export a échoué : aucun intervalle de coupes sélectionné."));
}

void MainWindow::exportAllSegmentedKnotsOfBillonToV3D()
{
	if ( _billon != 0 && _billon->hasPith() )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter tous les nœuds segmentés du billon en V3D"), "output.v3d", tr("Fichiers V3D (*.v3d);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if ( file.open(QIODevice::WriteOnly) )
			{
				QXmlStreamWriter stream(&file);
				V3DExport::init(file,stream);

				V3DExport::appendTags(stream,*_billon );
				V3DExport::appendPith(stream,*_billon );

				int intervalIndex, sectorIndex, counter;

				V3DExport::startComponents(stream);
				counter = 1;
				for ( intervalIndex=1 ; intervalIndex< _ui->_comboSelectSliceInterval->count() ; ++intervalIndex )
				{
					_ui->_comboSelectSliceInterval->setCurrentIndex(intervalIndex);
					for ( sectorIndex=1 ; sectorIndex< _ui->_comboSelectSectorInterval->count() ; ++sectorIndex )
					{
						selectSectorInterval(sectorIndex,false);
						if ( _knotBillon && _nearestPointsHistogram->intervals().size()>0 )
						{
							V3DExport::appendComponent( stream, *_knotBillon, _nearestPointsHistogram->interval(0), counter++ );
						}
					}
				}
				V3DExport::endComponents(stream);

				V3DExport::close(stream);

				file.close();

				QMessageBox::information(this,"Exporter tous les nœuds segmentés du billon en V3D", "Export réussi !");
			}
			else QMessageBox::warning(this,tr("Exporter tous les nœuds segmentés du billon en V3D"),tr("L'export a échoué : impossible de créer le ficher %1.").arg(fileName));
		}
	}
	else QMessageBox::warning(this,tr("Exporter tous les nœuds segmentés du billon en V3D"),tr("L'export a échoué : la moelle n'est pas calculée."));
}

void MainWindow::exportContourToSdp()
{
	if ( !_contourBillon->isEmpty() )
	{
		const Interval<uint> &sliceInterval = _sliceHistogram->interval(_ui->_comboSelectSliceInterval->currentIndex()-1);

		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter le contour de la coupe courante en SDP"), "output.ctr", tr("Fichiers de contours (*.sdp);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if ( file.open(QIODevice::WriteOnly) )
			{
				const Contour &contour = _contourBillon->contourSlice(_currentSlice-sliceInterval.min()).contour();

				QTextStream stream(&file);
				stream << contour.size() << endl;
				for ( int i=0 ; i<contour.size() ; ++i )
				{
					stream << contour.at(i).x << " " << contour.at(i).y << endl;
				}
				file.close();

				QMessageBox::information(this,"Exporter le contour de la coupe courante en SDP", "Export réussi !");
			}
			else QMessageBox::warning(this,tr("Exporter le contour de la coupe courante en SDP"),tr("L'export a échoué : impossible de créer le ficher %1.").arg(fileName));
		}
	}
	else QMessageBox::warning(this,tr("Exporter le contour de la coupe courante en SDP"),tr("L'export a échoué : le contour n'est pas calculé."));
}

void MainWindow::exportCurrentSegmentedKnotToSdp()
{
	if ( _knotBillon )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter le nœud courant segmenté en SDP"), "output.sdp", tr("Fichiers PGM3D (*.sdp);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if ( file.open(QIODevice::WriteOnly) )
			{
				QTextStream stream(&file);
				stream << "#SDP (Sequence of Discrete Points)" << endl;

				for ( uint k=_nearestPointsHistogram->interval(0).min() ; k<=_nearestPointsHistogram->interval(0).max() ; ++k )
				{
					SliceAlgorithm::writeInSDP( _knotBillon->slice(k) , stream, k, 0 );
				}

				file.close();

				QMessageBox::information(this,"Exporter le nœud courant segmenté en SDP", "Export réussi !");
			}
			else QMessageBox::warning(this,tr("Exporter le nœud courant segmenté en SDP"),tr("L'export a échoué : impossible de créer le ficher %1.").arg(fileName));
		}
	}
	else QMessageBox::warning(this,tr("Exporter le nœud courant segmenté en SDP"),tr("L'export a échoué : le nœud n'est pas segmenté."));
}

void MainWindow::exportSegmentedKnotsOfCurrentSliceIntervalToSdp()
{
	if ( _billon != 0 && _billon->hasPith() )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter les nœuds segmentés de l'intervalle de coupe courant en SDP"), "output.sdp",
														tr("Fichiers SDP (*.sdp);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if ( file.open(QIODevice::WriteOnly) )
			{
				QTextStream stream(&file);
				stream << "#SDP (Sequence of Discrete Points)" << endl;

				const bool useOldMethod = _ui->_checkExportSdpOldMethod->isChecked();
				int sectorIndex;
				uint k;

				for ( sectorIndex=1 ; sectorIndex< _ui->_comboSelectSectorInterval->count() ; ++sectorIndex )
				{
					_ui->_comboSelectSectorInterval->setCurrentIndex(sectorIndex);
					if ( _knotBillon && _nearestPointsHistogram->intervals().size()>0 )
					{
						for ( k=_nearestPointsHistogram->interval(0).min() ; k<=_nearestPointsHistogram->interval(0).max() ; ++k )
						{
							SliceAlgorithm::writeInSDP( useOldMethod?_componentBillon->slice(k):_knotBillon->slice(k) , stream, k, 0 );
						}
					}
				}

				file.close();

				QMessageBox::information(this,"Exporter les nœuds segmentés de l'intervalle de coupe courant en SDP", "Export réussi !");
			}
			else QMessageBox::warning(this,tr("Exporter les nœuds segmentés de l'intervalle de coupe courant en SDP"),tr("L'export a échoué : impossible de créer le ficher %1.").arg(fileName));
		}
	}
	else QMessageBox::warning(this,tr("Exporter les nœuds segmentés de l'intervalle de coupe courant en SDP"),tr("L'export a échoué : le nœud n'est pas segmenté."));
}

void MainWindow::exportAllSegmentedKnotsOfBillonToSdp()
{
	if ( _billon != 0 && _billon->hasPith() )
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Exporter tous les nœuds segmentés du billon en SDP"), "output.sdp", tr("Fichiers SDP (*.sdp);;Tous les fichiers (*.*)"));
		if ( !fileName.isEmpty() )
		{
			QFile file(fileName);
			if ( file.open(QIODevice::WriteOnly) )
			{
				QTextStream stream(&file);
				stream << "#SDP (Sequence of Discrete Points)" << endl;

				const bool useOldMethod = _ui->_checkExportSdpOldMethod->isChecked();
				int intervalIndex, sectorIndex;
				uint k;

				for ( intervalIndex=1 ; intervalIndex< _ui->_comboSelectSliceInterval->count() ; ++intervalIndex )
				{
					_ui->_comboSelectSliceInterval->setCurrentIndex(intervalIndex);
					for ( sectorIndex=1 ; sectorIndex< _ui->_comboSelectSectorInterval->count() ; ++sectorIndex )
					{
						_ui->_comboSelectSectorInterval->setCurrentIndex(sectorIndex);
						if ( _knotBillon && _nearestPointsHistogram->intervals().size() > 0 )
						{
							for ( k=_nearestPointsHistogram->interval(0).min() ; k<=_nearestPointsHistogram->interval(0).max() ; ++k )
							{
								SliceAlgorithm::writeInSDP( useOldMethod?_componentBillon->slice(k):_knotBillon->slice(k) , stream, k, 0 );
							}
						}
					}
				}

				file.close();

				QMessageBox::information(this,"Exporter tous les nœuds segmentés du billon en SDP", "Export réussi !");
			}
			else QMessageBox::warning(this,tr("Exporter tous les nœuds segmentés du billon en SDP"),tr("L'export a échoué : impossible de créer le ficher %1.").arg(fileName));
		}
	}
	else QMessageBox::warning(this,tr("Exporter tous les nœuds segmentés du billon en SDP"),tr("L'export a échoué : la moelle n'est pas calculée."));
}
