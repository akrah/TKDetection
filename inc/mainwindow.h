#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>

#include "sliceview_def.h"
#include "slicezoomer.h"
#include "interval.h"
#include <qwt_plot_curve.h>

namespace Ui {
	class MainWindow;
}

#include "billon_def.h"
class Marrow;
class SliceView;
class SliceHistogram;
class PieChart;
class PieChartDiagrams;
class ContourCurve;

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
	void drawSlice( const int &sliceNumber );
	void setSlice( const int &sliceNumber );
	void setTypeOfView( const int &type );
	void setLowThreshold( const int &threshold );
	void setHighThreshold( const int &threshold );
	void updateSliceHistogram();
	void setMarrowAroundDiameter( const int &diameter );
	void setHistogramIntervalMinimumWidth( const int &width );
	void setHistogramSmoothingType( const int &type );
	void setHistogramMaximumsNeighborhood( const int &neighborhood );
	void setHistogramDerivativePercentage( const int &percentage );
	void highlightSliceHistogram( const int &slicePosition );
	void updateMarrow();
	void computeSectorsHistogramsForCurrentSliceIntervall();
	void highlightSectorHistogram( const int &sectorIdx );
	void setMinimumOfSliceInterval( const int &min );
	void setMinimumOfSliceIntervalToCurrentSlice();
	void setMaximumOfSliceInterval( const int &max );
	void setMaximumOfSliceIntervalToCurrentSlice();
	void previousMaximumInSliceHistogram();
	void nextMaximumInSliceHistogram();
	void zoomInSliceView( const qreal &zoomFactor, const QPoint &focalPoint );
	void dragInSliceView( const QPoint &movementVector );
	void setMovementThresholdMin( const int &threshold );
	void setMovementThresholdMax( const int &threshold );
	void useNextSliceInsteadOfCurrentSlice( const bool &enable );
	void flowApplied();
	void setRestrictedAreaResolution( const int &resolution );
	void setRestrictedAreaThreshold( const int &threshold );
	void setRestrictedAreaBeginRadius( const int &radius );
	void setEdgeDetectionType( const int &type );
	void setCannyRadiusOfGaussianMask( const int &radius );
	void setCannySigmaOfGaussianMask( const double &sigma );
	void setCannyMinimumGradient( const int &minimumGradient );
	void setCannyMinimumDeviation( const double &minimumDeviation );
	void exportToDat();
	void exportToOfs();
	void exportToOfsRestricted();
	void exportSectorToOfs();
	void exportAllSectorInAllIntervalsToOfs();
	void exportToV3D();
	void exportFlowToV3D();
	void exportHistogramToSep();
	void exportMovementsToV3D();
	void selectSliceInterval( const int &index );
	void selectCurrentSliceInterval();
	void selectSectorInterval( const int &index );
	void selectCurrentSectorInterval();
	void exportSectorToPgm3D();
	void exportConnexComponentToPgm3D();
	void exportSliceHistogram();
	void exportSectorDiagramAndHistogram();
	void exportContours();
	void setContourCurveSmoothingRadius( const int &radius );
	void exportContourComponentToPgm3D();

private:
	void openNewBillon(const QString &folderName = "");
	void initComponentsValues();
	void updateUiComponentsValues();
	void enabledComponents();
	void computeSectorsHistogramForInterval( const Interval &interval );

private:
	Ui::MainWindow *_ui;
	QImage _pix;
	QButtonGroup _groupSliceView;

	Billon *_billon;
	Billon *_sectorBillon;
	Billon *_componentBillon;
	Marrow *_marrow;
	SliceView *_sliceView;
	SliceHistogram *_sliceHistogram;
	QwtPlotCurve _histogramCursor;

	PieChart *_pieChart;
	PieChartDiagrams *_pieChartDiagrams;

	ContourCurve *_contourCurve;

	SliceZoomer _sliceZoomer;
	Interval _slicesInterval;
	Interval _intensityInterval;

	int _currentSlice;
	int _currentMaximum;
	int _currentSector;
};

#endif // MAINWINDOW_H
