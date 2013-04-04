#include "inc/plotcurvaturehistogram.h"

#include "inc/curvaturehistogram.h"

PlotCurvatureHistogram::PlotCurvatureHistogram()
{
	_histogramCursor.setBrush(Qt::red);
	_histogramCursor.setPen(QPen(Qt::red));

	_histogramDominantPointsFromLeft.setBrush(Qt::green);
	_histogramDominantPointsFromLeft.setPen(QPen(Qt::green));

	_histogramDominantPointsFromRight.setBrush(Qt::yellow);
	_histogramDominantPointsFromRight.setPen(QPen(Qt::yellow));
}

PlotCurvatureHistogram::~PlotCurvatureHistogram()
{
}

void PlotCurvatureHistogram::attach( QwtPlot * const plot )
{
	if ( plot != 0 )
	{
		_histogramData.attach(plot);
		_histogramDominantPointsFromLeft.attach(plot);
		_histogramDominantPointsFromRight.attach(plot);
		_histogramCursor.attach(plot);
	}
}

void PlotCurvatureHistogram::clear()
{
	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
	_histogramDominantPointsFromLeft.setSamples(emptyData);
	_histogramDominantPointsFromRight.setSamples(emptyData);
	_histogramCursor.setSamples(emptyData);
}

void PlotCurvatureHistogram::moveCursor( const uint &sliceIndex )
{
	static QVector<QwtIntervalSample> datasCursor(1);
	static QwtIntervalSample &datasCursorInterval = datasCursor[0];
	datasCursorInterval.interval.setInterval(sliceIndex,sliceIndex+1);
	datasCursorInterval.value = _histogramData.sample(sliceIndex).value;
	_histogramCursor.setSamples(datasCursor);
}

void PlotCurvatureHistogram::update( const CurvatureHistogram & histogram, const QVector<int> &dominantPointsIndexFromLeft , const QVector<int> &dominantPointsIndexFromRight )
{
	updateDatas( histogram );
	updateDominantPoints( histogram, dominantPointsIndexFromLeft, dominantPointsIndexFromRight );
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


void PlotCurvatureHistogram::updateDominantPoints( const CurvatureHistogram &histogram, const QVector<int> &dominantPointsIndexFromLeft, const QVector<int> &dominantPointsIndexFromRight )
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
