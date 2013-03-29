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

	void moveCursor( const uint & sliceIndex );

	void update( const ContourDistancesHistogram & histogram, const QVector<int> &dominantPointsIndexFromLeft, const QVector<int> &dominantPointsIndexFromRight );

private:
	void updateDatas( const ContourDistancesHistogram & histogram );
	void updateDominantPoints(const ContourDistancesHistogram & histogram, const QVector<int> &dominantPointsIndexFromLeft, const QVector<int> &dominantPointsIndexFromRight );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramDominantPointsFromLeft;
	QwtPlotHistogram _histogramDominantPointsFromRight;
	QwtPlotHistogram _histogramCursor;
};

#endif // PLOTCONTOURDISTANCESHISTOGRAM_H
