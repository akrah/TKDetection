#include "inc/plotslicehistogram.h"

#include "inc/slicehistogram.h"

PlotSliceHistogram::PlotSliceHistogram()
{
	_histogramMaximums.setBrush(Qt::green);
	_histogramMaximums.setPen(QPen(Qt::green));

	_histogramIntervals.setBrush(Qt::blue);
	_histogramIntervals.setPen(QPen(Qt::blue));

	_curvePercentage.setPen(QPen(Qt::red));
}


PlotSliceHistogram::~PlotSliceHistogram()
{
}

/*******************************
 * Public setters
 *******************************/

void PlotSliceHistogram::attach( QwtPlot * const plot )
{
	if ( plot != 0 )
	{
		_histogramData.attach(plot);
		_histogramIntervals.attach(plot);
		_histogramMaximums.attach(plot);
		_curvePercentage.attach(plot);
	}
}

void PlotSliceHistogram::detach()
{
	_histogramData.detach();
	_histogramIntervals.detach();
	_histogramMaximums.detach();
	_curvePercentage.detach();
}

void PlotSliceHistogram::clear()
{
	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
	_histogramMaximums.setSamples(emptyData);
	_histogramIntervals.setSamples(emptyData);
	_curvePercentage.setSamples(QVector<QPointF>(0));
}

void PlotSliceHistogram::update( const SliceHistogram & histogram )
{
	computeValues( histogram );
	computeMaximums( histogram );
	computeIntervals( histogram );

//	const qreal thresholdOfMaximums = histogram.thresholdOfMaximums( minimumHeightPercentageOfMaximum );
//	const qreal x[] = { 0., depth };
//	const qreal y[] = { thresholdOfMaximums, thresholdOfMaximums };
//	_curvePercentage.setSamples(x,y,2);
}

void PlotSliceHistogram::computeValues( const SliceHistogram &histogram )
{
	QVector<QwtIntervalSample> datasHistogram;
	if ( histogram.size() > 0 )
	{
		datasHistogram.reserve(histogram.size());
		int i=0;
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

void PlotSliceHistogram::computeMaximums( const SliceHistogram & histogram )
{
	QVector<QwtIntervalSample> datasMaximums;
	if ( histogram.maximums().size() > 0 )
	{
		int slice;
		QVector<int>::ConstIterator begin = histogram.maximums().begin();
		const QVector<int>::ConstIterator end = histogram.maximums().end();
		while ( begin != end )
		{
			slice = *begin++;
			datasMaximums.append(QwtIntervalSample(histogram[slice],slice,slice+1));
		}
	}
	_histogramMaximums.setSamples(datasMaximums);
}

void PlotSliceHistogram::computeIntervals( const SliceHistogram & histogram )
{
	QVector<QwtIntervalSample> dataIntervals;
	if ( histogram.maximums().size() > 0 )
	{
		int i, min, max;
		QVector< Interval<int> >::ConstIterator begin = histogram.intervals().begin();
		const QVector< Interval<int> >::ConstIterator end = histogram.intervals().end();
		qDebug() << "Intervalles de nœuds :";
		while ( begin != end )
		{
			min = (*begin).min();
			max = (*begin++).max();
			for ( i=min ; i<max ; ++i )
			{
				dataIntervals.append(QwtIntervalSample(histogram[i],i,i+1));
			}
			qDebug() << "  [ " << min << ", " << max << " ] avec largeur = " << max-min;
		}
	}
	_histogramIntervals.setSamples(dataIntervals);
}
