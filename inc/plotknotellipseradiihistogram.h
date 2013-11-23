#ifndef PLOTKNOTELLIPSERADIIHISTOGRAM_H
#define PLOTKNOTELLIPSERADIIHISTOGRAM_H

#include <qwt_plot_histogram.h>

class QwtPlot;
class KnotEllipseRadiiHistogram;

class PlotKnotEllipseRadiiHistogram
{
public:
	PlotKnotEllipseRadiiHistogram();
	~PlotKnotEllipseRadiiHistogram();

	void attach( QwtPlot * const plot );
	void clear();

	void moveCursor( const uint & radiusIndex );

	void update( const KnotEllipseRadiiHistogram & histogram );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramCursor;
};

#endif // PLOTKNOTELLIPSERADIIHISTOGRAM_H
