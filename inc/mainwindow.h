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

class QLabel;

class ContourBillon;
class CurvatureHistogram;
class IntensityDistributionHistogram;
class NearestPointsHistogram;
class PieChart;
class PlotContourDistancesHistogram;
class PlotCurvatureHistogram;
class PlotIntensityDistributionHistogram;
class PlotNearestPointsHistogram;
class PlotSectorHistogram;
class PlotSliceHistogram;
class PlotZMotionDistributionHistogram;
class SectorHistogram;
class SliceHistogram;
class SliceView;
class ZMotionDistributionHistogram;

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
	void setSlice( const int &sliceNumber );
	void setYSlice( const int &yPosition );
	void moveNearestPointsCursor( const int &position );
	void moveContourCursor( const int &position );
	void setTypeOfView( const int &type );
	void resetHistogramDefaultValuesZMotion();
	void resetHistogramDefaultValuesNearestDistance();
	void updateSliceHistogram();
	void updateContourHistograms( const int &histogramIndex );
	void updateIntensityDistributionHistogram();
	void updateZMotionDistributionHistogram();
	void updatePith();
	void setMinimumOfSliceIntervalToCurrentSlice();
	void setMaximumOfSliceIntervalToCurrentSlice();
	void previousMaximumInSliceHistogram();
	void nextMaximumInSliceHistogram();
	void zoomInSliceView( const qreal &zoomFactor, const qreal &zoomCoefficient );
	void dragInSliceView( const QPoint &movementVector );
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

	void exportImgeSliceIntervalToPgm3d();
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
	QLabel *_labelSliceView;

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

	IntensityDistributionHistogram *_intensityDistributionHistogram;
	PlotIntensityDistributionHistogram *_plotIntensityDistributionHistogram;

	ZMotionDistributionHistogram *_zMotionDistributionHistogram;
	PlotZMotionDistributionHistogram * _plotZMotionDistributionHistogram;

	ContourBillon *_contourBillon;

	uint _currentSlice;
	uint _currentYSlice;
	int _currentMaximum;
	uint _currentSector;
	qreal _treeRadius;
};

#endif // MAINWINDOW_H
