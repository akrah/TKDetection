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

	void moveCursor( const uint & contourIndex );

	void update( const CurvatureHistogram & histogram, const int &leftMainDominantPoint, const int &rightMainDominantPoint );

private:
	void updateDatas( const CurvatureHistogram & histogram );
	void updateDominantPoints( const CurvatureHistogram & histogram, const int &leftMainDominantPointIndex, const int &rightMainDominantPointIndex );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramMainDominantPoints;
	QwtPlotHistogram _histogramCursor;
};

#endif // PLOTCURVATUREHISTOGRAM_H
