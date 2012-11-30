#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "def/def_billon.h"
#include "def/def_coordinate.h"
#include "slicezoomer.h"

namespace Ui
{
	class MainWindow;
}

class ContourBillon;
class CurvatureHistogram;
class NearestPointsHistogram;
class PieChart;
class PlotContourDistancesHistogram;
class PlotCurvatureHistogram;
class PlotNearestPointsHistogram;
class PlotSectorHistogram;
class PlotSliceHistogram;
class SectorHistogram;
class SliceHistogram;
class SliceView;

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
	void closeImage();
	void drawSlice();
	void setSlice( const int &sliceNumber );
	void setXSlice( const int &xPosition );
	void setYSlice( const int &yPosition );
	void moveNearestPointsCursor( const int &position );
	void moveContourCursor( const int &position );
	void setTypeOfView( const int &type );
	void updateSliceHistogram();
	void updateContourHistograms( const int &histogramIndex );
	void updatePith();
	void setMinimumOfSliceIntervalToCurrentSlice();
	void setMaximumOfSliceIntervalToCurrentSlice();
	void previousMaximumInSliceHistogram();
	void nextMaximumInSliceHistogram();
	void zoomInSliceView( const qreal &zoomFactor, const QPoint &focalPoint );
	void dragInSliceView( const QPoint &movementVector );
	void updateOpticalFlow();
	void setRestrictedAreaResolution( const int &resolution );
	void setRestrictedAreaThreshold( const int &threshold );
	void setRestrictedAreaBeginRadius( const int &radius );
	void setEdgeDetectionType( const int &type );
	void setCannyRadiusOfGaussianMask( const int &radius );
	void setCannySigmaOfGaussianMask( const double &sigma );
	void setCannyMinimumGradient( const int &minimumGradient );
	void setCannyMinimumDeviation( const double &minimumDeviation );
	void selectSliceInterval( const int &index );
	void selectCurrentSliceInterval();
	void selectSectorInterval( const int &index, const bool &draw = true );
	void selectCurrentSectorInterval();
	void exportToDat();
	void exportToOfs();
	void exportHistograms();
	void exportToPgm3D();
	void exportToV3D();
	void exportToSdp();

private:
	void openNewBillon(const QString &fileName);
	void initComponentsValues();
	void updateUiComponentsValues();
	void enabledComponents();
	void updateSectorHistogram( const Interval<uint> &interval );

	void exportPithToOfs();
	void exportBillonRestrictedAreaToOfs();
	void exportCurrentAngularSectorLargeAreaToOfs();
	void exportAllAngularSectorsOfAllSliceIntervalsLargeAreaToOfs();

	void exportSliceHistogramToSep();
	void exportSectorHistogramToSep();
	void exportKnotHistogramToSep();
	void exportSliceHistogramToImage();
	void exportSectorHistogramToImage();
	void exportknotHistogramToImage();

	void exportCurrentSegmentedKnotToPgm3d();
	void exportSegmentedKnotsOfCurrentSliceIntervalToPgm3d();

	void exportCurrentSegmentedKnotToV3D();
	void exportSegmentedKnotsOfCurrentSliceIntervalToV3D();
	void exportAllSegmentedKnotsOfBillonToV3D();

	void exportContourToSdp();
	void exportCurrentSegmentedKnotToSdp();
	void exportSegmentedKnotsOfCurrentSliceIntervalToSdp();
	void exportAllSegmentedKnotsOfBillonToSdp();

private:
	Ui::MainWindow *_ui;

	Billon *_billon;
	Billon *_componentBillon;
	Billon *_knotBillon;

	QImage _mainPix;
	SliceZoomer _sliceZoomer;

	SliceView *_sliceView;

	SliceHistogram *_sliceHistogram;
	PlotSliceHistogram *_plotSliceHistogram;

	PieChart *_pieChart;
	SectorHistogram *_sectorHistogram;
	PlotSectorHistogram * _plotSectorHistogram;

	NearestPointsHistogram *_nearestPointsHistogram;
	PlotNearestPointsHistogram *_plotNearestPointsHistogram;

	PlotCurvatureHistogram *_plotCurvatureHistogram;
	PlotContourDistancesHistogram *_plotContourDistancesHistogram;

	ContourBillon *_contourBillon;

	uint _currentSlice;
	uint _currentXSlice;
	uint _currentYSlice;
	int _currentMaximum;
	uint _currentSector;
	qreal _treeRadius;
};

#endif // MAINWINDOW_H
