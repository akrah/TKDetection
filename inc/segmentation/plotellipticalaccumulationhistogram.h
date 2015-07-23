#ifndef PLOTELLIPTICALACCUMULATIONHISTOGRAM_H
#define PLOTELLIPTICALACCUMULATIONHISTOGRAM_H

#include "inc/plothistogram.h"

#include <qwt_plot_histogram.h>

class PlotEllipticalAccumulationHistogram : public PlotHistogram
{
public:
	PlotEllipticalAccumulationHistogram();
	~PlotEllipticalAccumulationHistogram();

	void attach( QwtPlot * const plot );
	void clear();

	void update( const Histogram<qreal> &histogram );

	void moveCursor( const uint & radiusIndex );

private:
	void updateDatas( const Histogram<qreal> &histogram );
	void updateMaximums( const Histogram<qreal> &histogram );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramMaximums;
	QwtPlotHistogram _histogramCursor;
};

#endif // PLOTELLIPTICALACCUMULATIONHISTOGRAM_H
