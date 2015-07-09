#ifndef PLOTKNOTELLIPSERADIIHISTOGRAM_H
#define PLOTKNOTELLIPSERADIIHISTOGRAM_H

#include <qwt_plot_histogram.h>
#include <qwt_plot_curve.h>

class QwtPlot;
class EllipseRadiiHistogram;

class PlotEllipseRadiiHistogram
{
public:
	PlotEllipseRadiiHistogram();
	~PlotEllipseRadiiHistogram();

	void attach( QwtPlot * const plot );
	void clear();

	void moveCursor( const uint & radiusIndex );

	void update( const EllipseRadiiHistogram & histogram );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotCurve _histogramSmoothed;
	QwtPlotHistogram _histogramCursor;
};

#endif // PLOTKNOTELLIPSERADIIHISTOGRAM_H
