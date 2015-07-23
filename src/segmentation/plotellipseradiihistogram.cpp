#include "inc/segmentation/plotellipseradiihistogram.h"

#include "inc/segmentation/ellipseradiihistogram.h"

PlotEllipseRadiiHistogram::PlotEllipseRadiiHistogram()
{
	_histogramCursor.setBrush(Qt::red);
	_histogramCursor.setPen(QPen(Qt::red));

	_histogramSmoothed.setPen(QPen(Qt::blue));
}

PlotEllipseRadiiHistogram::~PlotEllipseRadiiHistogram()
{
}

void PlotEllipseRadiiHistogram::attach( QwtPlot * const plot )
{
	if ( plot )
	{
		_histogramData.attach(plot);
		_histogramSmoothed.attach(plot);
		_histogramCursor.attach(plot);
	}
}

void PlotEllipseRadiiHistogram::clear()
{
	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
	_histogramCursor.setSamples(emptyData);

	const QVector<QPointF> emptyCurve(0);
	_histogramSmoothed.setSamples(emptyCurve);
}

void PlotEllipseRadiiHistogram::update( const Histogram<qreal> &histogram )
{
	QVector<QwtIntervalSample> datasHistogram(0);
	QVector<QPointF> datasHistogramSmoothed(0);
	if ( histogram.size() > 0 )
	{
		datasHistogram.reserve(histogram.size());
		datasHistogramSmoothed.reserve(histogram.size());
		int i=0;
		QVector<qreal>::ConstIterator begin = histogram.begin();
		QVector<qreal>::ConstIterator beginSmoothed = histogram.begin();
		const QVector<qreal>::ConstIterator end = histogram.end();
		while ( begin != end )
		{
			datasHistogram.append(QwtIntervalSample(*begin++,i,i+1));
			datasHistogramSmoothed.append(QPointF(i,*beginSmoothed++));
			++i;
		}
	}
	_histogramData.setSamples(datasHistogram);
	_histogramSmoothed.setSamples(datasHistogramSmoothed);
}

void PlotEllipseRadiiHistogram::moveCursor( const uint & radiusIndex )
{
	static QVector<QwtIntervalSample> datasCursor(1);
	static QwtIntervalSample &datasCursorInterval = datasCursor[0];
	datasCursorInterval.interval.setInterval(radiusIndex,radiusIndex+1);
	datasCursorInterval.value = radiusIndex<_histogramData.dataSize() ? _histogramData.sample(radiusIndex).value : 0;
	_histogramCursor.setSamples(datasCursor);
}
