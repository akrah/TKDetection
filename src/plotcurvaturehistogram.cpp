#include "inc/plotcurvaturehistogram.h"

#include "inc/curvaturehistogram.h"

PlotCurvatureHistogram::PlotCurvatureHistogram()
{
}

PlotCurvatureHistogram::~PlotCurvatureHistogram()
{
}

void PlotCurvatureHistogram::attach( QwtPlot * const plot )
{
	if ( plot != 0 )
	{
		_histogramData.attach(plot);
	}
}

void PlotCurvatureHistogram::clear()
{
	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
}

void PlotCurvatureHistogram::update( const CurvatureHistogram & histogram )
{
	updateDatas( histogram );
}

void PlotCurvatureHistogram::updateDatas( const CurvatureHistogram &histogram )
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
