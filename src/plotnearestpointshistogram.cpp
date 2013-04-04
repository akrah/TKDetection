#include "inc/plotnearestpointshistogram.h"

#include "inc/nearestpointshistogram.h"

PlotNearestPointsHistogram::PlotNearestPointsHistogram()
{
	_histogramMaximums.setBrush(Qt::green);
	_histogramMaximums.setPen(QPen(Qt::green));

	_histogramIntervals.setBrush(Qt::blue);
	_histogramIntervals.setPen(QPen(Qt::blue));

	_histogramCursor.setBrush(Qt::red);
	_histogramCursor.setPen(QPen(Qt::red));
}

PlotNearestPointsHistogram::~PlotNearestPointsHistogram()
{
}

/*******************************
 * Public setters
 *******************************/

void PlotNearestPointsHistogram::attach( QwtPlot * const plot )
{
	if ( plot != 0 )
	{
		_histogramData.attach(plot);
		_histogramIntervals.attach(plot);
		_histogramMaximums.attach(plot);
		_histogramCursor.attach(plot);
	}
}

void PlotNearestPointsHistogram::clear()
{
	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
	_histogramMaximums.setSamples(emptyData);
	_histogramIntervals.setSamples(emptyData);
	_histogramCursor.setSamples(emptyData);
}

void PlotNearestPointsHistogram::moveCursor( const uint &sliceIndex )
{
	static QVector<QwtIntervalSample> datasCursor(1);
	static QwtIntervalSample &datasCursorInterval = datasCursor[0];
	datasCursorInterval.interval.setInterval(sliceIndex,sliceIndex+1);
	datasCursorInterval.value = _histogramData.sample(sliceIndex).value;
	_histogramCursor.setSamples(datasCursor);
}

void PlotNearestPointsHistogram::update( const NearestPointsHistogram & histogram )
{
	updateDatas( histogram );
	updateMaximums( histogram );
	updateIntervals( histogram );
}

void PlotNearestPointsHistogram::updateDatas( const NearestPointsHistogram &histogram )
{
	QVector<QwtIntervalSample> datasHistogram(0);
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

void PlotNearestPointsHistogram::updateMaximums( const NearestPointsHistogram & histogram )
{
	QVector<QwtIntervalSample> datasMaximums(0);
	if ( histogram.nbMaximums() > 0 )
	{
		datasMaximums.reserve(histogram.nbMaximums());
		uint slice;
		QVector<uint>::ConstIterator begin = histogram.maximums().begin();
		const QVector<uint>::ConstIterator end = histogram.maximums().end();
		while ( begin != end )
		{
			slice = *begin++;
			datasMaximums.append(QwtIntervalSample(histogram[slice],slice,slice+1));
		}
	}
	_histogramMaximums.setSamples(datasMaximums);
}

void PlotNearestPointsHistogram::updateIntervals( const NearestPointsHistogram & histogram )
{
	QVector<QwtIntervalSample> dataIntervals(0);
	if ( histogram.nbIntervals() > 0 )
	{
		dataIntervals.reserve(histogram.nbIntervals());
		uint i, min, max;
		QVector< Interval<uint> >::ConstIterator begin = histogram.intervals().begin();
		const QVector< Interval<uint> >::ConstIterator end = histogram.intervals().end();
		qDebug() << "Intervalles de nœuds :";
		while ( begin != end )
		{
			min = (*begin).min();
			max = (*begin++).max();
			for ( i=min ; i<=max ; ++i )
			{
				dataIntervals.append(QwtIntervalSample(histogram[i],i,i+1));
			}
			qDebug() << "  [ " << min << ", " << max << " ] avec largeur = " << max-min;
		}
	}
	_histogramIntervals.setSamples(dataIntervals);
}
