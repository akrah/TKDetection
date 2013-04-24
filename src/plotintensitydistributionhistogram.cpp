#include "inc/plotintensitydistributionhistogram.h"

#include "inc/intensitydistributionhistogram.h"

PlotIntensityDistributionHistogram::PlotIntensityDistributionHistogram()
{
}

PlotIntensityDistributionHistogram::~PlotIntensityDistributionHistogram()
{
}

void PlotIntensityDistributionHistogram::attach( QwtPlot * const plot )
{
	if ( plot )
	{
		_histogramData.attach(plot);
	}
}

void PlotIntensityDistributionHistogram::clear()
{
	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
}

void PlotIntensityDistributionHistogram::update( const IntensityDistributionHistogram & histogram, const Interval<int> &intensityInterval )
{
	updateDatas( histogram, intensityInterval );
}

void PlotIntensityDistributionHistogram::updateDatas( const IntensityDistributionHistogram &histogram, const Interval<int> &intensityInterval )
{
	QVector<QwtIntervalSample> datasHistogram(0);
	if ( histogram.size() > 0 )
	{
		datasHistogram.reserve(histogram.size());
		int i=intensityInterval.min();
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
