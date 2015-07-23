#ifndef PLOTKNOTELLIPSERADIIHISTOGRAM_H
#define PLOTKNOTELLIPSERADIIHISTOGRAM_H

#include "inc/plothistogram.h"

#include <qwt_plot_histogram.h>
#include <qwt_plot_curve.h>

class PlotEllipseRadiiHistogram : public PlotHistogram
{
public:
	PlotEllipseRadiiHistogram();
	~PlotEllipseRadiiHistogram();

	void attach( QwtPlot * const plot );
	void clear();

	void update( const Histogram<qreal> & histogram );

	void moveCursor( const uint & radiusIndex );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotCurve _histogramSmoothed;
	QwtPlotHistogram _histogramCursor;
};

#endif // PLOTKNOTELLIPSERADIIHISTOGRAM_H
