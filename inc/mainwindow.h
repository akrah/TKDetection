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
class PieChartHistograms;

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
	void adaptGraphicsComponentsToSliceType( const SliceType::SliceType &type );
	void computeNewMarrow();
	void redrawSliceHistogram();
	void highlightSliceHistogram( const int &slicePosition );
	void updateSectorsHistograms();
	void updateSectorsHistogramsView();
	void drawCurrentSector();

private:
	void updateGraphicsComponentsValues();
	void openNewBillon(const QString &folderName = "");

private:
	Ui::MainWindow *_ui;
	QButtonGroup _groupSliceView;

	Billon *_billon;
	Marrow *_marrow;
	SliceView *_sliceView;
	SliceHistogram *_sliceHistogram;
	QwtPlotCurve _histogramCursor;

	PieChart *_pieChart;
	PieChartHistograms *_pieChartHistograms;
	QList<QwtPlot *> _pieChartPlots;
};

#endif // MAINWINDOW_H
