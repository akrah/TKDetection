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

class EllipticalAccumulationHistogram;
class KnotPithProfile;
class PieChart;
class PlotEllipticalAccumulationHistogram;
class PlotKnotPithProfile;
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
	void zoomInSliceView( const qreal &zoomFactor, const qreal &zoomCoefficient );
	void dragInSliceView( const QPoint &movementVector );
	void zoomInTangentialView( const qreal &zoomFactor, const qreal &zoomCoefficient );
	void dragInTangentialView( const QPoint &movementVector );
	void previousMaximumInSliceHistogram();
	void nextMaximumInSliceHistogram();

	void setSlice( const int &sliceIndex );
	void setYSlice( const int &yPosition );
	void setTangentialSlice( const int &sliceIndex );
	void setSectorNumber( const int &value );

	void selectSliceInterval( const int &index );
	void selectCurrentSliceInterval();
	void selectSectorInterval( const int &index, const bool &draw = true );
	void selectCurrentSectorInterval();

	void updateBillonPith();
	void updateSliceHistogram();
	void updateSectorHistogram( const Interval<uint> &interval );
	void updateKnotPithProfile();
	void updateEllipticalAccumulationHistogram();

	void resetHistogramDefaultValuesZMotion();
	void resetHistogramDefaultValuesZMotionAngular();

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

	void exportPithOfCurrentKnotAreaToSdp();
//	void exportCurrentSegmentedKnotToSdp();
//	void exportSegmentedKnotsOfCurrentSliceIntervalToSdp( bool keepBillonSliceNumber = false );
//	void exportSegmentedKnotsOfCurrentSliceIntervalToSdpOldAlgo( bool keepBillonSliceNumber = false );
//	void exportAllSegmentedKnotsOfBillonToSdp();

private:
	Ui::MainWindow *_ui;
	QLabel *_labelSliceView;
	QLabel *_labelTangentialView;

	Billon *_billon;
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

	KnotPithProfile *_knotPithProfile;
	PlotKnotPithProfile *_plotKnotPithProfile;

	EllipticalAccumulationHistogram * _ellipticalAccumulationHistogram;
	PlotEllipticalAccumulationHistogram * _plotEllipticalAccumulationHistogram;

	uint _currentSlice;
	uint _currentYSlice;
	int _currentMaximum;
	uint _currentSector;
	qreal _treeRadius;
};

#endif // MAINWINDOW_H
