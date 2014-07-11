#include "inc/plotknotpithprofile.h"

#include "inc/knotpithprofile.h"
#include "inc/define.h"

PlotKnotPithProfile::PlotKnotPithProfile()
{
	_histogramCursor.setBrush(Qt::red);
	_histogramCursor.setPen(QPen(Qt::red));
}

PlotKnotPithProfile::~PlotKnotPithProfile()
{
}

void PlotKnotPithProfile::attach( QwtPlot * const plot )
{
	if ( plot )
	{
		_histogramData.attach(plot);
		_histogramCursor.attach(plot);
	}
}

void PlotKnotPithProfile::clear()
{
	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
	_histogramCursor.setSamples(emptyData);
}

void PlotKnotPithProfile::moveCursor( const uint & sliceIndex )
{
	static QVector<QwtIntervalSample> datasCursor(1);
	static QwtIntervalSample &datasCursorInterval = datasCursor[0];
	datasCursorInterval.interval.setInterval(sliceIndex,sliceIndex+1);
	datasCursorInterval.value = sliceIndex<_histogramData.dataSize() ? _histogramData.sample(sliceIndex).value : 0;
	_histogramCursor.setSamples(datasCursor);
}

void PlotKnotPithProfile::update( const KnotPithProfile & histogram )
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
			datasHistogram.append(QwtIntervalSample(qAcos(*begin++)*RAD_TO_DEG_FACT,i,i+1));
			++i;
		}
	}
	_histogramData.setSamples(datasHistogram);
}
