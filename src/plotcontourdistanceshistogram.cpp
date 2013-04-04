#include "inc/plotcontourdistanceshistogram.h"

#include "inc/contourdistanceshistogram.h"

PlotContourDistancesHistogram::PlotContourDistancesHistogram()
{
	_histogramCursor.setBrush(Qt::red);
	_histogramCursor.setPen(QPen(Qt::red));

	_histogramDominantPointsFromLeft.setBrush(Qt::green);
	_histogramDominantPointsFromLeft.setPen(QPen(Qt::green));

	_histogramDominantPointsFromRight.setBrush(Qt::yellow);
	_histogramDominantPointsFromRight.setPen(QPen(Qt::yellow));
}

PlotContourDistancesHistogram::~PlotContourDistancesHistogram()
{
}

void PlotContourDistancesHistogram::attach( QwtPlot * const plot )
{
	if ( plot != 0 )
	{
		_histogramData.attach(plot);
		_histogramDominantPointsFromLeft.attach(plot);
		_histogramDominantPointsFromRight.attach(plot);
		_histogramCursor.attach(plot);
	}
}

void PlotContourDistancesHistogram::clear()
{
	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
	_histogramDominantPointsFromLeft.setSamples(emptyData);
	_histogramDominantPointsFromRight.setSamples(emptyData);
	_histogramCursor.setSamples(emptyData);
}

void PlotContourDistancesHistogram::moveCursor( const uint &sliceIndex )
{
	static QVector<QwtIntervalSample> datasCursor(1);
	static QwtIntervalSample &datasCursorInterval = datasCursor[0];
	datasCursorInterval.interval.setInterval(sliceIndex,sliceIndex+1);
	datasCursorInterval.value = _histogramData.sample(sliceIndex).value;
	_histogramCursor.setSamples(datasCursor);
}

void PlotContourDistancesHistogram::update(const ContourDistancesHistogram &histogram, const QVector<int> &dominantPointsIndexFromLeft , const QVector<int> &dominantPointsIndexFromRight )
{
	updateDatas( histogram );
	updateDominantPoints( histogram, dominantPointsIndexFromLeft, dominantPointsIndexFromRight );
}

void PlotContourDistancesHistogram::updateDatas( const ContourDistancesHistogram &histogram )
{
	QVector<QwtIntervalSample> datasHistogram(0);
	if ( histogram.size() > 0 )
	{
		datasHistogram.reserve(histogram.size());
		int i=0;
		foreach (const qreal &distance, histogram)
		{
			datasHistogram.append(QwtIntervalSample(distance,i,i+1));
			++i;
		}
	}
	_histogramData.setSamples(datasHistogram);
}


void PlotContourDistancesHistogram::updateDominantPoints( const ContourDistancesHistogram &histogram, const QVector<int> &dominantPointsIndexFromLeft, const QVector<int> &dominantPointsIndexFromRight )
{
	QVector<QwtIntervalSample> dominantPointHistogram(0);
	if ( histogram.size() > 0 && dominantPointsIndexFromLeft.size() > 0 )
	{
		dominantPointHistogram.reserve(dominantPointsIndexFromLeft.size());
		foreach (const int &dpIndex, dominantPointsIndexFromLeft) {
			dominantPointHistogram.append(QwtIntervalSample(histogram[dpIndex],dpIndex,dpIndex+1));
		}
	}
	_histogramDominantPointsFromLeft.setSamples(dominantPointHistogram);
	dominantPointHistogram.clear();
	if ( histogram.size() > 0 && dominantPointsIndexFromRight.size() > 0 )
	{
		dominantPointHistogram.reserve(dominantPointsIndexFromRight.size());
		foreach (const int &dpIndex, dominantPointsIndexFromRight) {
			dominantPointHistogram.append(QwtIntervalSample(histogram[dpIndex],dpIndex,dpIndex+1));
		}
	}
	_histogramDominantPointsFromRight.setSamples(dominantPointHistogram);
}
