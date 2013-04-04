#ifndef PLOTCURVATUREHISTOGRAM_H
#define PLOTCURVATUREHISTOGRAM_H

#include <qwt_plot_histogram.h>

class CurvatureHistogram;

class PlotCurvatureHistogram
{
public:
	PlotCurvatureHistogram();
	~PlotCurvatureHistogram();

	void attach( QwtPlot * const plot );
	void clear();

	void moveCursor( const uint & sliceIndex );

	void update( const CurvatureHistogram & histogram, const QVector<int> &dominantPointsIndexFromLeft, const QVector<int> &dominantPointsIndexFromRight );

private:
	void updateDatas( const CurvatureHistogram & histogram );
	void updateDominantPoints(const CurvatureHistogram & histogram, const QVector<int> &dominantPointsIndexFromLeft, const QVector<int> &dominantPointsIndexFromRight );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramDominantPointsFromLeft;
	QwtPlotHistogram _histogramDominantPointsFromRight;
	QwtPlotHistogram _histogramCursor;
};

#endif // PLOTCURVATUREHISTOGRAM_H
