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
class KnotAreaHistogram;
class PieChart;
class PlotCurvatureHistogram;
class PlotKnotAreaHistogram;
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
	void moveKnotAreaCursor( const int &position );
	void moveContourCursor( const int &position );
	void setTypeOfView( const int &type );
	void updateSliceHistogram();
	void updateCurvatureHistogram();
	void updatePith();
	void setMinimumOfSliceIntervalToCurrentSlice();
	void setMaximumOfSliceIntervalToCurrentSlice();
	void previousMaximumInSliceHistogram();
	void nextMaximumInSliceHistogram();
	void zoomInSliceView( const qreal &zoomFactor, const QPoint &focalPoint );
	void dragInSliceView( const QPoint &movementVector );
	void updateOpticalFlowalFlow();
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

	void exportCurrentKnotAreaToPgm3d();
	void exportCurrentSegmentedKnotToPgm3d();

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

	QImage _mainPix;
	SliceZoomer _sliceZoomer;

	SliceView *_sliceView;

	SliceHistogram *_sliceHistogram;
	PlotSliceHistogram *_plotSliceHistogram;

	PieChart *_pieChart;
	SectorHistogram *_sectorHistogram;
	PlotSectorHistogram * _plotSectorHistogram;

	KnotAreaHistogram *_knotAreaHistogram;
	PlotKnotAreaHistogram *_plotKnotAreaHistogram;

	CurvatureHistogram *_curvatureHistogram;
	PlotCurvatureHistogram *_plotCurvatureHistogram;

	ContourBillon *_contourBillon;

	uint _currentSlice;
	int _currentMaximum;
	uint _currentSector;
};

#endif // MAINWINDOW_H
