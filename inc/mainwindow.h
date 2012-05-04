#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>

#include "sliceview_def.h"
#include "slicezoomer.h"
#include "slicesinterval.h"
#include "intensityinterval.h"
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
	void setTypeOfView( const int &type );
	void setLowThreshold( const int &threshold );
	void setHighThreshold( const int &threshold );
	void updateSliceHistogram();
	void setMarrowAroundDiameter( const int &diameter );
	void setHistogramIntervalType( const int &type );
	void setHistogramIntervalMinimumWidth( const int &width );
	void enableHistogramSmoothing( const bool &enable );
	void setHistogramMaximumsNeighborhood( const int &neighborhood );
	void highlightSliceHistogram( const int &slicePosition );
	void updateMarrow();
	void computeSectorsHistogramsForCurrentSliceIntervall();
	void highlightSectorHistogram( const int &sectorIdx );
	void setMinimumOfSlicesIntervalToCurrentSlice();
	void setMaximumOfSlicesIntervalToCurrentSlice();
	void previousMaximumInSliceHistogram();
	void nextMaximumInSliceHistogram();
	void zoomInSliceView( const qreal &zoomFactor, const QPoint &focalPoint );
	void dragInSliceView( const QPoint &movementVector );
	void setMovementThresholdMin( const int &threshold );
	void setMovementThresholdMax( const int &threshold );
	void enableMovementWithBackground( const bool &enable );
	void useNextSliceInsteadOfCurrentSlice( const bool &enable );
	void flowApplied();
	void setRestrictedAreaResolution( const int &resolution );
	void setRestrictedAreaThreshold( const int &threshold );
	void enableRestrictedAreaCircle( const bool &enable ) ;
	void setRestrictedAreaBeginRadius( const int &radius );
	void computeRestrictedBillon();
	void changeBillonUsed();
	void exportToDat();
	void exportToOfs();
		void exportToOfsRestricted();
	void exportToV3D();
	void exportFlowToV3D();
	void exportHistogramToV3D();
	void exportMovementsToV3D();
	void selectSliceInterval( const int &index );
	void selectCurrentSliceInterval();
	void selectSectorInterval( const int &index );
	void selectCurrentSectorInterval();
	void exportSectorToPgm3D();
	void exportConnexComponentToPgm3D();
	void exportSliceHistogram();
	void exportSectorDiagramAndHistogram();

private:
	void openNewBillon(const QString &folderName = "");
	void updateComponentsValues();
	void enabledComponents();
	void computeSectorsHistogramForInterval( const SlicesInterval &interval );

private:
	Ui::MainWindow *_ui;
	QImage _pix;
	QButtonGroup _groupSliceView;

	Billon *_billon;
	Billon *_unusedBillon;
	Billon *_sectorBillon;
	Billon *_componentBillon;
	Marrow *_marrow;
	SliceView *_sliceView;
	SliceHistogram *_sliceHistogram;
	QwtPlotCurve _histogramCursor;

	PieChart *_pieChart;
	PieChartDiagrams *_pieChartDiagrams;

	SliceZoomer _sliceZoomer;
	SlicesInterval _slicesInterval;
	IntensityInterval _intensityInterval;

	int _currentSlice;
	int _currentMaximum;
	bool _isUsedOriginalBillon;
	int _currentSector;
};

#endif // MAINWINDOW_H
