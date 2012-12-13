#ifndef PLOTZMOTIONDISTRIBUTIONHISTOGRAM_H
#define PLOTZMOTIONDISTRIBUTIONHISTOGRAM_H

#include <qwt_plot_histogram.h>
class ZMotionDistributionHistogram;
template<typename T> class Interval;

class PlotZMotionDistributionHistogram
{
public:
	PlotZMotionDistributionHistogram();
	~PlotZMotionDistributionHistogram();

	void attach( QwtPlot * const plot );
	void clear();

	void update(const ZMotionDistributionHistogram & histogram , const Interval<uint> &zMotionInterval );

private:
	void updateDatas( const ZMotionDistributionHistogram & histogram, const Interval<uint> &zMotionInterval );

private:
	QwtPlotHistogram _histogramData;
};

#endif // PLOTZMOTIONDISTRIBUTIONHISTOGRAM_H
