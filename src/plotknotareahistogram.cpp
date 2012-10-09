#include "inc/plotknotareahistogram.h"

#include "inc/knotareahistogram.h"

PlotKnotAreaHistogram::PlotKnotAreaHistogram()
{
	_histogramMaximums.setBrush(Qt::green);
	_histogramMaximums.setPen(QPen(Qt::green));

	_histogramIntervals.setBrush(Qt::blue);
	_histogramIntervals.setPen(QPen(Qt::blue));

	_histogramCursor.setBrush(Qt::red);
	_histogramCursor.setPen(QPen(Qt::red));
}

PlotKnotAreaHistogram::~PlotKnotAreaHistogram()
{
}

/*******************************
 * Public setters
 *******************************/

void PlotKnotAreaHistogram::attach( QwtPlot * const plot )
{
	if ( plot != 0 )
	{
		_histogramData.attach(plot);
		_histogramIntervals.attach(plot);
		_histogramMaximums.attach(plot);
		_histogramCursor.attach(plot);
	}
}

void PlotKnotAreaHistogram::clear()
{
	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
	_histogramMaximums.setSamples(emptyData);
	_histogramIntervals.setSamples(emptyData);
	_histogramCursor.setSamples(emptyData);
}

void PlotKnotAreaHistogram::moveCursor( const uint &sliceIndex )
{
	QVector<QwtIntervalSample> datasCursor(1);
	datasCursor[0].interval.setInterval(sliceIndex,sliceIndex+1);
	datasCursor[0].value = _histogramData.sample(sliceIndex).value;
	_histogramCursor.setSamples(datasCursor);
}

void PlotKnotAreaHistogram::update( const KnotAreaHistogram & histogram )
{
	updateDatas( histogram );
	updateMaximums( histogram );
	updateIntervals( histogram );
}

void PlotKnotAreaHistogram::updateDatas( const KnotAreaHistogram &histogram )
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

void PlotKnotAreaHistogram::updateMaximums( const KnotAreaHistogram & histogram )
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

void PlotKnotAreaHistogram::updateIntervals( const KnotAreaHistogram & histogram )
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
			for ( i=min ; i<max ; ++i )
			{
				dataIntervals.append(QwtIntervalSample(histogram[i],i,i+1));
			}
			qDebug() << "  [ " << min << ", " << max << " ] avec largeur = " << max-min;
		}
	}
	_histogramIntervals.setSamples(dataIntervals);
}
