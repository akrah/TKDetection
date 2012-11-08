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

	void update( const CurvatureHistogram & histogram );

private:
	void updateDatas( const CurvatureHistogram & histogram );
	void updateDominantPoints( const CurvatureHistogram & histogram );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramDominantPoints;
	QwtPlotHistogram _histogramCursor;
};

#endif // PLOTCURVATUREHISTOGRAM_H
