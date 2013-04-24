#include "inc/plotzmotiondistributionhistogram.h"

#include "inc/zmotiondistributionhistogram.h"

PlotZMotionDistributionHistogram::PlotZMotionDistributionHistogram()
{
}

PlotZMotionDistributionHistogram::~PlotZMotionDistributionHistogram()
{
}

void PlotZMotionDistributionHistogram::attach( QwtPlot * const plot )
{
	if ( plot )
	{
		_histogramData.attach(plot);
	}
}

void PlotZMotionDistributionHistogram::clear()
{
	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
}

void PlotZMotionDistributionHistogram::update( const ZMotionDistributionHistogram & histogram, const Interval<uint> &zMotionInterval )
{
	updateDatas( histogram, zMotionInterval );
}

void PlotZMotionDistributionHistogram::updateDatas( const ZMotionDistributionHistogram &histogram, const Interval<uint> &zMotionInterval )
{
	QVector<QwtIntervalSample> datasHistogram(0);
	if ( histogram.size() > 0 )
	{
		datasHistogram.reserve(histogram.size());
		int i=zMotionInterval.min();
		QVector<qreal>::ConstIterator begin = histogram.begin();
		const QVector<qreal>::ConstIterator end = histogram.end();
		while ( begin != end )
		{
			datasHistogram.append(QwtIntervalSample(*begin++,i,i+1));
			++i;
		}
	}
	_histogramData.setSamples(datasHistogram);
}
