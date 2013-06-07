#include "inc/plotcontourdistanceshistogram.h"

#include "inc/contourdistanceshistogram.h"

PlotContourDistancesHistogram::PlotContourDistancesHistogram()
{
	_histogramCursor.setBrush(Qt::red);
	_histogramCursor.setPen(QPen(Qt::red));

	_histogramMainDominantPoints.setBrush(Qt::green);
	_histogramMainDominantPoints.setPen(QPen(Qt::green));
}

PlotContourDistancesHistogram::~PlotContourDistancesHistogram()
{
}

void PlotContourDistancesHistogram::attach( QwtPlot * const plot )
{
	if ( plot )
	{
		_histogramData.attach(plot);
		_histogramMainDominantPoints.attach(plot);
		_histogramCursor.attach(plot);
	}
}

void PlotContourDistancesHistogram::clear()
{
	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
	_histogramMainDominantPoints.setSamples(emptyData);
	_histogramCursor.setSamples(emptyData);
}

void PlotContourDistancesHistogram::moveCursor( const uint &contourIndex )
{
	static QVector<QwtIntervalSample> datasCursor(1);
	static QwtIntervalSample &datasCursorInterval = datasCursor[0];
	datasCursorInterval.interval.setInterval(contourIndex,contourIndex+1);
	datasCursorInterval.value = _histogramData.sample(contourIndex).value;
	_histogramCursor.setSamples(datasCursor);
}

void PlotContourDistancesHistogram::update( const ContourDistancesHistogram &histogram, const int &leftMainDominantPointIndex , const int &rightMainDominantPointIndex )
{
	updateDatas( histogram );
	updateDominantPoints( histogram, leftMainDominantPointIndex, rightMainDominantPointIndex );
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


void PlotContourDistancesHistogram::updateDominantPoints( const ContourDistancesHistogram &histogram, const int &leftMainDominantPointIndex, const int &rightMainDominantPointIndex )
{
	QVector<QwtIntervalSample> dominantPointHistogram(0);
	if ( histogram.size() > 0 )
	{
		if ( leftMainDominantPointIndex != -1 )
			dominantPointHistogram.append( QwtIntervalSample(histogram[leftMainDominantPointIndex],leftMainDominantPointIndex,leftMainDominantPointIndex+1) );
		if ( rightMainDominantPointIndex != -1 )
			dominantPointHistogram.append( QwtIntervalSample(histogram[rightMainDominantPointIndex],rightMainDominantPointIndex,rightMainDominantPointIndex+1) );
	}
	_histogramMainDominantPoints.setSamples(dominantPointHistogram);
}
