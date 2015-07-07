#include "inc/plotellipticalaccumulationhistogram.h"

#include "inc/ellipticalaccumulationhistogram.h"

PlotEllipticalAccumulationHistogram::PlotEllipticalAccumulationHistogram()
{
	_histogramCursor.setBrush(Qt::red);
	_histogramCursor.setPen(QPen(Qt::red));

	_histogramMaximums.setBrush(Qt::green);
	_histogramMaximums.setPen(QPen(Qt::green));
}

PlotEllipticalAccumulationHistogram::~PlotEllipticalAccumulationHistogram()
{
}

void PlotEllipticalAccumulationHistogram::attach( QwtPlot * const plot )
{
	if ( plot )
	{
		_histogramData.attach(plot);
		_histogramMaximums.attach(plot);
		_histogramCursor.attach(plot);
	}
}

void PlotEllipticalAccumulationHistogram::clear()
{
	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
	_histogramMaximums.setSamples(emptyData);
	_histogramCursor.setSamples(emptyData);
}

void PlotEllipticalAccumulationHistogram::moveCursor( const uint & radiusIndex )
{
	static QVector<QwtIntervalSample> datasCursor(1);
	static QwtIntervalSample &datasCursorInterval = datasCursor[0];
	datasCursorInterval.interval.setInterval(radiusIndex,radiusIndex+1);
	datasCursorInterval.value = radiusIndex<_histogramData.dataSize() ? _histogramData.sample(radiusIndex).value : 0;
	_histogramCursor.setSamples(datasCursor);
}

void PlotEllipticalAccumulationHistogram::update( const EllipticalAccumulationHistogram & histogram )
{
	updateDatas( histogram );
	updateMaximums( histogram );
}

void PlotEllipticalAccumulationHistogram::updateDatas( const EllipticalAccumulationHistogram & histogram )
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

void PlotEllipticalAccumulationHistogram::updateMaximums( const EllipticalAccumulationHistogram & histogram )
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
