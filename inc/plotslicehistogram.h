#ifndef PLOTSLICEHISTOGRAM_H
#define PLOTSLICEHISTOGRAM_H

#include <qwt_plot_histogram.h>
#include <qwt_plot_curve.h>

class SliceHistogram;
class QwtPlot;

class PlotSliceHistogram
{
public:
	PlotSliceHistogram();
	~PlotSliceHistogram();

	void attach( QwtPlot * const plot );
	void clear();

	void moveCursor( const uint & sliceIndex );
	void updatePercentageCurve( const uint & thresholdOfMaximums );

	void update( const SliceHistogram & histogram );

private:
	void updateDatas( const SliceHistogram & histogram );
	void updateMaximums( const SliceHistogram & histogram );
	void updateIntervals( const SliceHistogram & histogram );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramMaximums;
	QwtPlotHistogram _histogramIntervals;
	QwtPlotHistogram _histogramCursor;

	QwtPlotCurve _curvePercentage;
};

#endif // PLOTSLICEHISTOGRAM_H
