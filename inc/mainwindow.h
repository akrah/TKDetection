#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>

#include "sliceview_def.h"
#include <qwt_plot_curve.h>

namespace Ui {
	class MainWindow;
}

class Billon;
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
	void drawSlice( const int &sliceNumber );
	void setTypeOfView( const int &type );
	void setLowThreshold( const int &threshold );
	void setHighThreshold( const int &threshold );
	void updateSliceHistogram();
	void highlightSliceHistogram( const int &slicePosition );
	void updateMarrow();
	void updateSectorsHistograms();
	void selectSectorHistogram( const int &sectorIdx );
	void setMinimumSectorsIntervalToCurrentSlice();
	void setMaximumSectorsIntervalToCurrentSlice();
	void updateMinimumSectorsIntervalExtremum( const int &value );
	void updateMaximumSectorsIntervalExtremum( const int &value );
	void setMinimalDifferenceForSectors( const int &minimalDifference );

private:
	void openNewBillon(const QString &folderName = "");
	void drawSlice();
	void updateComponentsValues();
	void enabledComponents();

private:
	Ui::MainWindow *_ui;
	QPixmap _pix;
	QButtonGroup _groupSliceView;

	Billon *_billon;
	Marrow *_marrow;
	SliceView *_sliceView;
	SliceHistogram *_sliceHistogram;
	QwtPlotCurve _histogramCursor;

	PieChart *_pieChart;
	PieChartDiagrams *_pieChartDiagrams;
	QList<QwtPlot *> _pieChartPlots;
};

#endif // MAINWINDOW_H
