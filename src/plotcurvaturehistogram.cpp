#include "inc/plotcurvaturehistogram.h"

#include "inc/curvaturehistogram.h"

PlotCurvatureHistogram::PlotCurvatureHistogram()
{
	_histogramCursor.setBrush(Qt::red);
	_histogramCursor.setPen(QPen(Qt::red));

	_histogramMainDominantPoints.setBrush(Qt::green);
	_histogramMainDominantPoints.setPen(QPen(Qt::green));
}

PlotCurvatureHistogram::~PlotCurvatureHistogram()
{
}

void PlotCurvatureHistogram::attach( QwtPlot * const plot )
{
	if ( plot )
	{
		_histogramData.attach(plot);
		_histogramMainDominantPoints.attach(plot);
		_histogramCursor.attach(plot);
	}
}

void PlotCurvatureHistogram::clear()
{
	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
	_histogramMainDominantPoints.setSamples(emptyData);
	_histogramCursor.setSamples(emptyData);
}

void PlotCurvatureHistogram::moveCursor(const uint &contourIndex )
{
	static QVector<QwtIntervalSample> datasCursor(1);
	static QwtIntervalSample &datasCursorInterval = datasCursor[0];
	datasCursorInterval.interval.setInterval(contourIndex,contourIndex+1);
	datasCursorInterval.value = _histogramData.sample(contourIndex).value;
	_histogramCursor.setSamples(datasCursor);
}

void PlotCurvatureHistogram::update(const CurvatureHistogram & histogram, const int &leftMainDominantPoint , const int &rightMainDominantPoint )
{
	updateDatas( histogram );
	updateDominantPoints( histogram, leftMainDominantPoint, rightMainDominantPoint );
}

void PlotCurvatureHistogram::updateDatas( const CurvatureHistogram &histogram )
{
	QVector<QwtIntervalSample> datasHistogram(0);
	if ( histogram.size() > 0 )
	{
		datasHistogram.reserve(histogram.size());
		int i=0;
		foreach (const qreal &curvature, histogram)
		{
			datasHistogram.append(QwtIntervalSample(curvature,i,i+1));
			++i;
		}
	}
	_histogramData.setSamples(datasHistogram);
}


void PlotCurvatureHistogram::updateDominantPoints(const CurvatureHistogram &histogram, const int &leftMainDominantPointIndex, const int &rightMainDominantPointIndex )
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
