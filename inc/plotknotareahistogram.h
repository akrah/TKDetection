#ifndef PLOTKNOTAREAHISTOGRAM_H
#define PLOTKNOTAREAHISTOGRAM_H

#include <qwt_plot_histogram.h>

class QwtPlot;
class KnotAreaHistogram;

class PlotKnotAreaHistogram
{
public:
	PlotKnotAreaHistogram();
	~PlotKnotAreaHistogram();

	void attach( QwtPlot * const plot );
	void clear();

	void moveCursor( const uint & sliceIndex );

	void update( const KnotAreaHistogram & histogram );

private:
	void updateDatas( const KnotAreaHistogram & histogram );
	void updateMaximums( const KnotAreaHistogram & histogram );
	void updateIntervals( const KnotAreaHistogram & histogram );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramMaximums;
	QwtPlotHistogram _histogramIntervals;
	QwtPlotHistogram _histogramCursor;
};

#endif // PLOTKNOTAREAHISTOGRAM_H
