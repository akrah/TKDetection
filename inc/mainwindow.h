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

class ConcavityPointSerieCurve;
class ContourBillon;
class CurvatureHistogram;
class IntensityDistributionHistogram;
class NearestPointsHistogram;
class PieChart;
class PlotConcavityPointSerieCurve;
class PlotContourDistancesHistogram;
class PlotCurvatureHistogram;
class PlotIntensityDistributionHistogram;
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
	void openTiff();
	void closeImage();

	void drawSlice();
	void drawTangentialView();
	void setTypeOfView( const int &type );
	void zoomInSliceView( const qreal &zoomFactor, const qreal &zoomCoefficient );
	void dragInSliceView( const QPoint &movementVector );
	void previousMaximumInSliceHistogram();
	void nextMaximumInSliceHistogram();

	void setSlice( const int &sliceNumber );
	void setYSlice( const int &yPosition );
	void setSectorNumber( const int &value );

	void selectSliceInterval( const int &index );
	void selectCurrentSliceInterval();
	void selectSectorInterval( const int &index, const bool &draw = true );
	void selectCurrentSectorInterval();

	void updatePith();
	void updateSliceHistogram();
	void updateSectorHistogram( const Interval<uint> &interval );
	void updateContourHistograms( const int &sliceIndex );
	void updateConcavityPointSerieCurve();
	void updateIntensityDistributionHistogram();
	void updateIntensityDistributionHistogramOnKnotArea();

	void updateCurvatureThreshold( const int &value );
	void updateCurvatureThreshold( const double &value );
	void resetHistogramDefaultValuesZMotion();
	void resetHistogramDefaultValuesZMotionAngular();
	void resetHistogramDefaultValuesNearestDistance();
	void moveNearestPointsCursor( const int &position );
	void moveContourCursor( const int &position );

	void exportToOfs();
	void exportHistograms();
	void exportToPgm3D();
	void exportToV3D();
	void exportToSdp();

private:
	void openNewBillon( const QString &fileName );
	void initComponentsValues();
	void updateUiComponentsValues();
	void enabledComponents();

	void exportPithToOfs( const bool &onCurrentSliceInterval );
	void exportCompleteBillonToOfs();
	void exportCurrentKnotAreaToOfs();
	void exportAllKnotAreasOfCurrentSliceIntervalToOfs();
	void exportAllKnotAreasToOfs();

	void exportSliceHistogramToSep();
	void exportSectorHistogramToSep();
	void exportKnotHistogramToSep();
	void exportSliceHistogramToImage();
	void exportSectorHistogramToImage();
	void exportknotHistogramToImage();

	void exportImageSliceIntervalToPgm3d();
	void exportImageCartesianSliceIntervalToPgm3d();
	void exportCurrentSegmentedKnotToPgm3d();
	void exportSegmentedKnotsOfCurrentSliceIntervalToPgm3d();

	void exportCurrentSegmentedKnotToV3D();
	void exportSegmentedKnotsOfCurrentSliceIntervalToV3D();
	void exportAllSegmentedKnotsOfBillonToV3D();

	void exportContourToSdp();
	void exportCurrentSegmentedKnotToSdp();
	void exportSegmentedKnotsOfCurrentSliceIntervalToSdp( bool keepBillonSliceNumber = false );
	void exportSegmentedKnotsOfCurrentSliceIntervalToSdpOldAlgo( bool keepBillonSliceNumber = false );
	void exportAllSegmentedKnotsOfBillonToSdp();

private:
	Ui::MainWindow *_ui;
	QLabel *_labelSliceView;
	QLabel *_labelTangentialView;

	Billon *_billon;
	Billon *_componentBillon;
	Billon *_knotBillon;
	Billon *_tangentialBillon;

	QImage _mainPix;
	SliceZoomer _sliceZoomer;

	QImage _tangentialPix;
	SliceZoomer _tangentialZoomer;

	SliceView *_sliceView;

	SliceHistogram *_sliceHistogram;
	PlotSliceHistogram *_plotSliceHistogram;

	SectorHistogram *_sectorHistogram;
	PlotSectorHistogram * _plotSectorHistogram;

	NearestPointsHistogram *_nearestPointsHistogram;
	PlotNearestPointsHistogram *_plotNearestPointsHistogram;

	PlotCurvatureHistogram *_plotCurvatureHistogram;
	PlotContourDistancesHistogram *_plotContourDistancesHistogram;

	IntensityDistributionHistogram *_intensityDistributionHistogram;
	PlotIntensityDistributionHistogram *_plotIntensityDistributionHistogram;

	IntensityDistributionHistogram *_intensityDistributionHistogramOnKnotArea;
	PlotIntensityDistributionHistogram *_plotIntensityDistributionHistogramOnKnotArea;

	ConcavityPointSerieCurve * _concavityPointSerieCurve;
	PlotConcavityPointSerieCurve * _plotConcavityPointSerieCurve;

	ContourBillon *_contourBillon;

	uint _currentSlice;
	uint _currentYSlice;
	int _currentMaximum;
	uint _currentSector;
	qreal _treeRadius;
};

#endif // MAINWINDOW_H
