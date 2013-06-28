#ifndef PLOTCONTOURDISTANCESHISTOGRAM_H
#define PLOTCONTOURDISTANCESHISTOGRAM_H

#include <qwt_plot_histogram.h>

class ContourDistancesHistogram;

class PlotContourDistancesHistogram
{
public:
	PlotContourDistancesHistogram();
	~PlotContourDistancesHistogram();

	void attach( QwtPlot * const plot );
	void clear();

	void moveCursor(const uint & contourIndex );

	void update( const ContourDistancesHistogram & histogram, const int &leftMainDominantPointIndex, const int &rightMainDominantPointIndex );

private:
	void updateDatas( const ContourDistancesHistogram & histogram );
	void updateDominantPoints(const ContourDistancesHistogram & histogram, const int &leftMainDominantPointIndex, const int &rightMainDominantPointIndex );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramMainDominantPoints;
	QwtPlotHistogram _histogramCursor;
};

#endif // PLOTCONTOURDISTANCESHISTOGRAM_H
