#ifndef PLOTNEARESTPOINTSHISTOGRAM_H
#define PLOTNEARESTPOINTSHISTOGRAM_H

#include <qwt_plot_histogram.h>

class QwtPlot;
class NearestPointsHistogram;

class PlotNearestPointsHistogram
{
public:
	PlotNearestPointsHistogram();
	~PlotNearestPointsHistogram();

	void attach( QwtPlot * const plot );
	void clear();

	void moveCursor( const uint & sliceIndex );

	void update( const NearestPointsHistogram & histogram );

private:
	void updateDatas( const NearestPointsHistogram & histogram );
	void updateMaximums( const NearestPointsHistogram & histogram );
	void updateIntervals( const NearestPointsHistogram & histogram );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramMaximums;
	QwtPlotHistogram _histogramIntervals;
	QwtPlotHistogram _histogramCursor;
};

#endif // PLOTNEARESTPOINTSHISTOGRAM_H
