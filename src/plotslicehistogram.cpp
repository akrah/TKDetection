#include "inc/plotslicehistogram.h"
#include "inc/slicehistogram.h"

PlotSliceHistogram::PlotSliceHistogram()
{
	_histogramMaximums.setBrush(Qt::green);
	_histogramMaximums.setPen(QPen(Qt::green));

	_histogramIntervals.setBrush(Qt::blue);
	_histogramIntervals.setPen(QPen(Qt::blue));

	_histogramCursor.setBrush(Qt::red);
	_histogramCursor.setPen(QPen(Qt::red));

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
	if ( plot )
	{
		_histogramData.attach(plot);
		_histogramIntervals.attach(plot);
		_histogramMaximums.attach(plot);
		_histogramCursor.attach(plot);
		_curvePercentage.attach(plot);
	}
}

void PlotSliceHistogram::clear()
{
	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
	_histogramMaximums.setSamples(emptyData);
	_histogramIntervals.setSamples(emptyData);
	_histogramCursor.setSamples(emptyData);

	_curvePercentage.setSamples(QVector<QPointF>(0));
}

void PlotSliceHistogram::moveCursor( const uint &sliceIndex )
{
	static QVector<QwtIntervalSample> datasCursor(1);
	static QwtIntervalSample &datasCursorInterval = datasCursor[0];
	datasCursorInterval.interval.setInterval(sliceIndex,sliceIndex+1);
	datasCursorInterval.value = sliceIndex<_histogramData.dataSize() ? _histogramData.sample(sliceIndex).value : 0;
	_histogramCursor.setSamples(datasCursor);
}

void PlotSliceHistogram::updatePercentageCurve( const uint & thresholdOfMaximums )
{
	const qreal x[] = { 0., static_cast<qreal>(_histogramData.dataSize()) };
	const qreal y[] = { static_cast<qreal>(thresholdOfMaximums), static_cast<qreal>(thresholdOfMaximums) };
	_curvePercentage.setSamples(x,y,2);
}

void PlotSliceHistogram::update( const SliceHistogram & histogram )
{
	updateDatas( histogram );
	updateMaximums( histogram );
	updateIntervals( histogram );
}

void PlotSliceHistogram::updateDatas( const SliceHistogram &histogram )
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

void PlotSliceHistogram::updateMaximums( const SliceHistogram & histogram )
{
	QVector<QwtIntervalSample> datasMaximums(0);
	if ( histogram.nbMaximums() > 0 )
	{
		datasMaximums.reserve(histogram.nbMaximums());
		int slice;
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

void PlotSliceHistogram::updateIntervals( const SliceHistogram & histogram )
{
	QVector<QwtIntervalSample> dataIntervals(0);
	if ( histogram.nbIntervals() > 0 )
	{
		dataIntervals.reserve(histogram.nbIntervals());
		uint i, min, max;
		QVector< Interval<uint> >::ConstIterator begin = histogram.intervals().begin();
		const QVector< Interval<uint> >::ConstIterator end = histogram.intervals().end();
//		qDebug() << "Intervalles de nœuds :";
		while ( begin != end )
		{
			min = (*begin).min();
			max = (*begin).max();
			for ( i=min ; i<=max ; ++i )
			{
				dataIntervals.append(QwtIntervalSample(histogram[i],i,i+1));
			}
			begin++;
//			qDebug() << "  [ " << min << ", " << max << " ] avec largeur = " << max-min;
		}
	}
	_histogramIntervals.setSamples(dataIntervals);
}
