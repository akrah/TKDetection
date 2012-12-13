#ifndef PLOTINTENSITYDISTRIBUTIONHISTOGRAM_H
#define PLOTINTENSITYDISTRIBUTIONHISTOGRAM_H

#include <qwt_plot_histogram.h>

class IntensityDistributionHistogram;
template<typename T> class Interval;

class PlotIntensityDistributionHistogram
{
public:
	PlotIntensityDistributionHistogram();
	~PlotIntensityDistributionHistogram();

	void attach( QwtPlot * const plot );
	void clear();

	void update(const IntensityDistributionHistogram & histogram , const Interval<int> &intensityInterval );

private:
	void updateDatas( const IntensityDistributionHistogram & histogram, const Interval<int> &intensityInterval );

private:
	QwtPlotHistogram _histogramData;
};

#endif // PLOTINTENSITYDISTRIBUTIONHISTOGRAM_H
