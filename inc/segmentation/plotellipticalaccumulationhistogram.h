#ifndef PLOTELLIPTICALACCUMULATIONHISTOGRAM_H
#define PLOTELLIPTICALACCUMULATIONHISTOGRAM_H

#include <qwt_plot_histogram.h>

class EllipticalAccumulationHistogram;
class QwtPlot;

class PlotEllipticalAccumulationHistogram
{
public:
	PlotEllipticalAccumulationHistogram();
	~PlotEllipticalAccumulationHistogram();

	void attach( QwtPlot * const plot );
	void clear();

	void moveCursor( const uint & radiusIndex );

	void update( const EllipticalAccumulationHistogram & histogram );

private:
	void updateDatas( const EllipticalAccumulationHistogram & histogram );
	void updateMaximums( const EllipticalAccumulationHistogram & histogram );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramMaximums;
	QwtPlotHistogram _histogramCursor;
};

#endif // PLOTELLIPTICALACCUMULATIONHISTOGRAM_H
