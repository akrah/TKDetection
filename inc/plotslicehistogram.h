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

	void update( const SliceHistogram & histogram );

private:
	void computeValues( const SliceHistogram & histogram );
	void computeMaximums( const SliceHistogram & histogram );
	void computeIntervals( const SliceHistogram & histogram );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramMaximums;
	QwtPlotHistogram _histogramIntervals;

	QwtPlotCurve _curvePercentage;
};

#endif // PLOTSLICEHISTOGRAM_H
