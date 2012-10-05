#include "inc/plotsectorhistogram.h"

#include "inc/coordinate.h"
#include "inc/define.h"
#include "inc/piechart.h"
#include "inc/piepart.h"
#include "inc/pointpolarseriesdata.h"
#include "inc/sectorhistogram.h"

#include <qwt_polar_plot.h>
#include <qwt_plot.h>

#include <QPainter>

PlotSectorHistogram::PlotSectorHistogram() : _datasCurve(new PointPolarSeriesData()), _datasCurveMaximums(new PointPolarSeriesData()),
	_datasCurveIntervals(new PointPolarSeriesData()),  _datasHighlightCurve(new PointPolarSeriesData())
{
	_highlightCurve.setPen(QPen(Qt::red));
	_histogramHighlightCurve.setBrush(Qt::red);
	_histogramHighlightCurve.setPen(QPen(Qt::red));

	_curveMaximums.setPen(QPen(Qt::green));
	_histogramMaximums.setBrush(Qt::green);
	_histogramMaximums.setPen(QPen(Qt::green));

	_curveIntervals.setPen(QPen(Qt::blue));
	_histogramIntervals.setBrush(Qt::blue);
	_histogramIntervals.setPen(QPen(Qt::blue));

	_curvePercentage.setPen(QPen(Qt::red));

	_curveData.setData(_datasCurve);
	_curveMaximums.setData(_datasCurveMaximums);
	_highlightCurve.setData(_datasHighlightCurve);
	_curveIntervals.setData(_datasCurveIntervals);
}

PlotSectorHistogram::~PlotSectorHistogram()
{
}

/*******************************
 * Public setters
 *******************************/

void PlotSectorHistogram::attach( QwtPolarPlot * const polarPlot )
{
	if ( polarPlot != 0 )
	{
		_curveData.attach(polarPlot);
		_curveIntervals.attach(polarPlot);
		_curveMaximums.attach(polarPlot);
		_highlightCurve.attach(polarPlot);
	}
}

void PlotSectorHistogram::attach( QwtPlot * const plot )
{
	if ( plot != 0 )
	{
		_histogramData.attach(plot);
		_histogramIntervals.attach(plot);
		_histogramMaximums.attach(plot);
		_histogramHighlightCurve.attach(plot);
		_curvePercentage.attach(plot);
	}
}

void PlotSectorHistogram::clear()
{
	const QVector<QwtIntervalSample> empty(0);
	_histogramData.setSamples(empty);
	_histogramMaximums.setSamples(empty);
	_histogramIntervals.setSamples(empty);
	_histogramHighlightCurve.setSamples(empty);

	_datasCurve->clear();
	_datasCurveMaximums->clear();
	_datasCurveIntervals->clear();
	_datasHighlightCurve->clear();

	_curvePercentage.setSamples(QVector<QPointF>(0));
}

void PlotSectorHistogram::highlightCurve( const uint &index )
{
	QVector<QwtIntervalSample> highlightCurveHistogramDatas(1);
	if ( index < _histogramData.dataSize() )
	{
		const qreal rightAngle = _histogramData.sample(index).interval.maxValue();
		const qreal leftAngle = _histogramData.sample(index).interval.minValue();
		const qreal valueOfCurve = _histogramData.sample(index).value;

		_datasHighlightCurve->resize(4);
		_datasHighlightCurve->at(0).setAzimuth(rightAngle);
		_datasHighlightCurve->at(1).setAzimuth(rightAngle);
		_datasHighlightCurve->at(1).setRadius(valueOfCurve);
		_datasHighlightCurve->at(2).setAzimuth(leftAngle);
		_datasHighlightCurve->at(2).setRadius(valueOfCurve);
		_datasHighlightCurve->at(3).setAzimuth(leftAngle);

		highlightCurveHistogramDatas[0] = _histogramData.sample(index);
	}
	else
	{
		_datasHighlightCurve->resize(0);
	}
	_histogramHighlightCurve.setSamples(highlightCurveHistogramDatas);
	_highlightCurve.plot()->replot();
	_histogramHighlightCurve.plot()->replot();
}

void PlotSectorHistogram::update( const SectorHistogram & histogram, const PieChart & pieChart )
{
	computeValues(histogram,pieChart);
	computeMaximums(histogram,pieChart);
	computeIntervals(histogram,pieChart);

//	const qreal derivativeThreshold = thresholdOfMaximums( minimumHeightPercentageOfMaximum );
//	const qreal x[] = { 0., TWO_PI };
//	const qreal y[] = { derivativeThreshold, derivativeThreshold };
//	_curvePercentage.setSamples(x,y,2);
}


/*******************************
 * Private functions
 *******************************/
void PlotSectorHistogram::computeValues( const SectorHistogram &histogram, const PieChart &pieChart )
{
	const int nbSectors = histogram.size();
	QVector<QwtIntervalSample> datasHistogram(0);
	if ( nbSectors > 0 )
	{
		datasHistogram.fill(QwtIntervalSample(),nbSectors);
		_datasCurve->resize(2*nbSectors+1);
		QVector<QwtPointPolar>::Iterator beginCurve = _datasCurve->begin();
		QVector<QwtIntervalSample>::Iterator beginHist = datasHistogram.begin();
		int i, value;
		qreal left, right;
		for ( i=0 ; i<nbSectors ; ++i )
		{
			const PiePart &part = pieChart.sector(i);
			value = histogram[i];
			left = part.leftAngle();
			right = part.rightAngle();
			(*beginCurve).setAzimuth(right);
			(*beginCurve++).setRadius(value);
			(*beginCurve).setAzimuth(left);
			(*beginCurve++).setRadius(value);
			(*beginHist).value = value;
			(*beginHist++).interval.setInterval(right,left);
		}
		(*beginCurve).setAzimuth(pieChart.sector(0).rightAngle());
		(*beginCurve).setRadius(histogram[0]);
	}
	else
	{
		_datasCurve->resize(0);
	}
	_histogramData.setSamples(datasHistogram);
}

void PlotSectorHistogram::computeMaximums( const SectorHistogram &histogram, const PieChart &pieChart )
{
	const int nbMaximums = histogram.nbMaximums();
	QVector<QwtIntervalSample> curveHistogramMaximumsDatas(nbMaximums);
	_datasCurveMaximums->resize(4*nbMaximums);
	if ( nbMaximums > 0 )
	{
		int sector;
		qreal left, right, value;
		QVector<int>::ConstIterator begin = histogram.maximums().begin();
		const QVector<int>::ConstIterator end = histogram.maximums().end();
		QVector<QwtPointPolar>::Iterator beginCurve = _datasCurveMaximums->begin();
		QVector<QwtIntervalSample>::Iterator beginHist = curveHistogramMaximumsDatas.begin();
		while ( begin != end )
		{
			sector = *begin++;
			left = pieChart.sector(sector).leftAngle();
			right = pieChart.sector(sector).rightAngle();
			value = histogram[sector];
			(*beginCurve++).setAzimuth(right);
			(*beginCurve).setAzimuth(right);
			(*beginCurve++).setRadius(value);
			(*beginCurve).setAzimuth(left);
			(*beginCurve++).setRadius(value);
			(*beginCurve++).setAzimuth(left);
			(*beginHist).interval.setMinValue(right);
			(*beginHist).interval.setMaxValue(left);
			(*beginHist++).value = value;
		}
	}
	_histogramMaximums.setSamples(curveHistogramMaximumsDatas);
}

void PlotSectorHistogram::computeIntervals( const SectorHistogram &histogram, const PieChart &pieChart )
{
	_datasCurveIntervals->clear();
	QVector<QwtIntervalSample> curveHistogramIntervalsDatas;

	if ( histogram.nbIntervals() > 0 )
	{
		int min, max, i;
		Interval<int> currentInterval;
		QVector< Interval<int> >::ConstIterator begin = histogram.intervals().begin();
		const QVector< Interval<int> >::ConstIterator end = histogram.intervals().end();
		const int nbSectors = histogram.nbIntervals();
		qDebug() << "Intervalles d'angles' :";
		while ( begin != end )
		{
			currentInterval = *begin++;
			min = currentInterval.min();
			max = currentInterval.max();
			if ( currentInterval.isValid() )
			{
				_datasCurveIntervals->append(QwtPointPolar( pieChart.sector(min).rightAngle(), 0. ));
				for ( i=min ; i<max ; ++i )
				{
					const PiePart &currentSector = pieChart.sector(i);
					_datasCurveIntervals->append(QwtPointPolar(currentSector.rightAngle(),histogram[i]));
					_datasCurveIntervals->append(QwtPointPolar(currentSector.leftAngle(), histogram[i]));
					curveHistogramIntervalsDatas.append(QwtIntervalSample( histogram[i],currentSector.rightAngle(),currentSector.leftAngle()));
				}
				_datasCurveIntervals->append(QwtPointPolar( pieChart.sector(max).leftAngle(), 0. ));
			}
			else
			{
				_datasCurveIntervals->append(QwtPointPolar( pieChart.sector(min).rightAngle(), 0. ));
				for ( i=min ; i<nbSectors ; ++i )
				{
					const PiePart &currentSector = pieChart.sector(i);
					_datasCurveIntervals->append(QwtPointPolar(currentSector.rightAngle(), histogram[i]));
					_datasCurveIntervals->append(QwtPointPolar(currentSector.leftAngle(), histogram[i]));
					curveHistogramIntervalsDatas.append(QwtIntervalSample( histogram[i],currentSector.rightAngle(),currentSector.leftAngle()));
				}
				for ( i=0 ; i<max ; ++i )
				{
					const PiePart &currentSector = pieChart.sector(i);
					_datasCurveIntervals->append(QwtPointPolar(currentSector.rightAngle(), histogram[i]));
					_datasCurveIntervals->append(QwtPointPolar(currentSector.leftAngle(), histogram[i]));
					curveHistogramIntervalsDatas.append(QwtIntervalSample( histogram[i],currentSector.rightAngle(),currentSector.leftAngle()));
				}
				_datasCurveIntervals->append(QwtPointPolar( pieChart.sector(max>=0?max:nbSectors-1).leftAngle(), 0. ));
			}
			qDebug() << "  [ " << min << ", " << max << " ] => [" << pieChart.sector(min).rightAngle()*RAD_TO_DEG_FACT << ", " << pieChart.sector(max).leftAngle()*RAD_TO_DEG_FACT << "] avec largeur = " << max-min;
		}
	}
	_histogramIntervals.setSamples(curveHistogramIntervalsDatas);
}
