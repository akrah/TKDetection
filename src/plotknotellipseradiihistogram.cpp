#include "inc/plotknotellipseradiihistogram.h"

#include "inc/knotellipseradiihistogram.h"

PlotKnotEllipseRadiiHistogram::PlotKnotEllipseRadiiHistogram()
{
	_histogramCursor.setBrush(Qt::red);
	_histogramCursor.setPen(QPen(Qt::red));
}

PlotKnotEllipseRadiiHistogram::~PlotKnotEllipseRadiiHistogram()
{
}

void PlotKnotEllipseRadiiHistogram::attach( QwtPlot * const plot )
{
	if ( plot )
	{
		_histogramData.attach(plot);
		_histogramCursor.attach(plot);
	}
}

void PlotKnotEllipseRadiiHistogram::clear()
{
	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
	_histogramCursor.setSamples(emptyData);
}

void PlotKnotEllipseRadiiHistogram::moveCursor( const uint & radiusIndex )
{
	static QVector<QwtIntervalSample> datasCursor(1);
	static QwtIntervalSample &datasCursorInterval = datasCursor[0];
	datasCursorInterval.interval.setInterval(radiusIndex,radiusIndex+1);
	datasCursorInterval.value = radiusIndex<_histogramData.dataSize() ? _histogramData.sample(radiusIndex).value : 0;
	_histogramCursor.setSamples(datasCursor);
}

void PlotKnotEllipseRadiiHistogram::update( const KnotEllipseRadiiHistogram & histogram )
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
