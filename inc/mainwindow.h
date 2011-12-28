#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>

#include "sliceview_def.h"
#include "marrowextractor_def.h"

namespace Ui {
	class MainWindow;
}

class Billon;
class SliceView;
class SliceHistogram;

#include <qwt_plot_curve.h>

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
	void redrawHistogram();
	void highlightHistogramSlice( const int &slicePosition );

private:
	void updateGraphicsComponentsValues();
	void openNewBillon(const QString &folderName = "");

private:
	Ui::MainWindow *_ui;
	QButtonGroup _groupSliceView;

	Billon *_billon;
	SliceView *_sliceView;
	SliceHistogram *_sliceHistogram;
	Marrow *_marrow;
	QwtPlotCurve _histogramCursor;
};

#endif // MAINWINDOW_H
