#ifndef PLOTSLICEHISTOGRAM_H
#define PLOTSLICEHISTOGRAM_H

#include "inc/plothistogram.h"

#include <qwt_plot_histogram.h>
#include <qwt_plot_curve.h>

class PlotSliceHistogram : public PlotHistogram
{
public:
	PlotSliceHistogram();
	~PlotSliceHistogram();

	void attach( QwtPlot * const plot );
	void clear();

	void update( const Histogram<qreal> &histogram );

	void moveCursor( const uint &sliceIndex );
	void updatePercentageCurve( const uint &thresholdOfMaximums );


private:
	void updateDatas( const Histogram<qreal> &histogram );
	void updateMaximums( const Histogram<qreal> &histogram );
	void updateIntervals( const Histogram<qreal> &histogram );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramMaximums;
	QwtPlotHistogram _histogramIntervals;
	QwtPlotHistogram _histogramCursor;

	QwtPlotCurve _curvePercentage;
};

#endif // PLOTSLICEHISTOGRAM_H
