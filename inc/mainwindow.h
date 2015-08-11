#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextStream>

#include <qwt_plot.h>

#include "def/def_billon.h"
#include "def/def_coordinate.h"
#include "inc/detection/knotbywhorldetector.h"
#include "inc/detection/knotbyzmotionmapdetector.h"
#include "inc/detection/plotslicehistogram.h"
#include "inc/detection/plotsectorhistogram.h"
#include "inc/pithextractorboukadida.h"
#include "inc/plothistogram.h"
#include "inc/segmentation/tangentialgenerator.h"
#include "inc/segmentation/pithprofile.h"
#include "inc/segmentation/ellipseradiihistogram.h"
#include "inc/sliceui.h"
#include "slicezoomer.h"

namespace Ui
{
	class MainWindow;
}

class PlotEllipticalAccumulationHistogram;
class PlotEllipseRadiiHistogram;
class PlotPithProfile;
class SlicePainter;
class QComboBox;

template <typename T> class Interval;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow( QWidget *parent = 0 );
	~MainWindow();

	bool eventFilter( QObject *obj, QEvent *event );

private slots:
	void openDicom();
	void openTiff();
	void closeImage();

	void drawSlice();
	void drawTangentialSlice();
	void drawZMotionMap();

	void setSlice( const int &sliceIndex );
	void setYSlice( const int &yPosition );
	void setTangentialSlice( const int &sliceIndex );
	void setTangentialYSlice( const int &yPosition );
	void setSectorNumber( const int &value );
	void setMapSectorNumber( const int &value );

	void computeBillonPith( const bool &draw = true );
	void computeSliceAndSectorHistograms( const bool &draw = true );

	void selectSliceInterval( const int &index );
	void selectSectorInterval( const int &index, const bool &draw = true );
	void selectKnotArea( const int &index, const bool &draw = true );
	void selectCurrentKnotArea();

	void computeKnotPithProfile( const Billon &billon, const bool &draw = true );
	void computeKnotEllipseRadiiHistogram( const Billon &billon, const bool &draw = true );
	void updateEllipticalAccumulationHistogram();

	void computeZMotionMapKnotAreas( const bool &draw = true );

	void resetHistogramDefaultValuesZMotion();
	void resetHistogramDefaultValuesZMotionAngular();

	void exportToOfs();
	void exportHistograms();
	void exportToPgm3D();
	void exportToV3D();
	void exportToSdp();
	void exportToPng();
	void export2DZMotion();
	void export2DKnotAreaCoordinates();

private:
	void openNewBillon( const QString &fileName );
	void postOpenFile( const QString &filename, Billon *billon );
	void initComponentsValues();
	void updateUiComponentsValues();
	void enabledComponents();

	void updatePlotHistogram( PlotHistogram &plotHistogram, QwtPlot &uiPlotHistogram, const Histogram<qreal> &histogram, const int &cursorPosition );
	void updatePlotSliceHistogram();
	void updatePlotSectorHistogram();
	void resetPlotSectorHistogram();
	void resetComboBox( QComboBox &comboBox );
	void updateComboBox( QComboBox &comboBox, const Histogram<qreal> &histogram );

	void exportPithToOfs( const bool &onCurrentSliceInterval );
	void exportCompleteBillonToOfs();
	void exportCurrentKnotAreaToOfs();
	void exportAllKnotAreasToOfs();

	void exportSliceHistogramToSep();
	void exportSectorHistogramToSep();
	void exportSliceHistogramToImage();
	void exportSectorHistogramToImage();
//	void exportknotHistogramToImage();

	void exportImageSliceIntervalToPgm3d();
	void exportImageCartesianSliceIntervalToPgm3d();
//	void exportCurrentSegmentedKnotToPgm3d();
//	void exportSegmentedKnotsOfCurrentSliceIntervalToPgm3d();

//	void exportCurrentSegmentedKnotToV3D();
//	void exportSegmentedKnotsOfCurrentSliceIntervalToV3D();
//	void exportAllSegmentedKnotsOfBillonToV3D();

	void exportPithOfCurrentKnotToSdp();
	void exportPithOfCurrentKnotAreaToSdp();
	void exportPithOfAllKnotAreaToSdp();
	void exportPithOfAKnotAreaToSdp( QTextStream &stream, const TangentialGenerator &tangentialTransform, const uint &numSliceInterval, const uint &numAngularInterval, const uint &knotID );
	void exportPithOfBillonToSdp();

	void exportCurrentSegmentedKnotToSdp( const bool &useSliceIntervalCoordinates = false );
	void exportSegmentedKnotsOfCurrentSliceIntervalToSdp( const bool &useSliceIntervalCoordinates = false );
	void exportAllSegmentedKnotsOfBillonToSdp();
	void exportSegmentedKnotToSdp( QTextStream &stream, const TangentialGenerator &tangentialTransform, const bool &useSliceIntervalCoordinates , const int & knotId = 1 );

//	void exportSegmentedKnotsOfCurrentSliceIntervalToSdpOldAlgo( bool keepBillonSliceNumber = false );

private:
	Ui::MainWindow *_ui;

	SliceUI _billonSliceUI;
	SliceUI _tangentialSliceUI;
	SliceUI _zMotionMapSliceUI;
	SliceUI _tangentialZMotionMapSliceUI;

	Billon *_billon;
	Billon *_tangentialBillon;
	Billon *_tangentialZMotionMapBillon;

	SlicePainter *_slicePainter;

	KnotByWhorlDetector _knotByWhorlDetector;
	PlotSliceHistogram _plotSliceHistogram;
	PlotSectorHistogram _plotSectorHistogram;

	KnotByZMotionMapDetector _knotByZMotionMapDetector;

	PithProfile _knotPithProfile;
	PlotPithProfile *_plotKnotPithProfile;

	EllipseRadiiHistogram  _knotEllipseRadiiHistogram;
	PlotEllipseRadiiHistogram * _plotKnotEllipseRadiiHistogram;
	PlotEllipticalAccumulationHistogram *_plotEllipticalAccumulationHistogram;

	PithExtractorBoukadida _billonPithExtractor;
	PithExtractorBoukadida _knotPithExtractor;

	TangentialGenerator _tangentialGenerator;


	uint _currentSliceInterval;
	uint _currentSectorInterval;
	uint _currentKnotArea;
	uint _currentSector;
	qreal _treeRadius;
};

#endif // MAINWINDOW_H
