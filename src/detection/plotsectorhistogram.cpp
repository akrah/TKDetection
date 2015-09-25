#include "inc/detection/plotsectorhistogram.h"

#include "inc/coordinate.h"
#include "inc/define.h"
#include "inc/piechart.h"
#include "inc/piepart.h"
#include "inc/pointpolarseriesdata.h"
#include "inc/detection/sectorhistogram.h"

#include <qwt_polar_plot.h>
#include <qwt_plot.h>

#include <QPainter>

PlotSectorHistogram::PlotSectorHistogram() : _datasCurve(new PointPolarSeriesData()), _datasCurveMaximums(new PointPolarSeriesData()),
	_datasCurveIntervals(new PointPolarSeriesData()),  _datasCurveCursor(new PointPolarSeriesData())
{
	_curveMaximums.setPen(QPen(Qt::green));
	_histogramMaximums.setBrush(Qt::green);
	_histogramMaximums.setPen(QPen(Qt::green));

	_curveIntervals.setPen(QPen(Qt::blue));
	_histogramIntervals.setBrush(Qt::blue);
	_histogramIntervals.setPen(QPen(Qt::blue));

	_curveCursor.setPen(QPen(Qt::red));
	_histogramCursor.setBrush(Qt::red);
	_histogramCursor.setPen(QPen(Qt::red));

	_curvePercentage.setPen(QPen(Qt::red));

	_curveData.setData(_datasCurve);
	_curveMaximums.setData(_datasCurveMaximums);
	_curveCursor.setData(_datasCurveCursor);
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
	if ( polarPlot )
	{
		_curveData.attach(polarPlot);
		_curveIntervals.attach(polarPlot);
		_curveMaximums.attach(polarPlot);
		_curveCursor.attach(polarPlot);
	}
}

void PlotSectorHistogram::attach( QwtPlot * const plot )
{
	if ( plot )
	{
		_histogramData.attach(plot);
		_histogramIntervals.attach(plot);
		_histogramMaximums.attach(plot);
		_histogramCursor.attach(plot);
		_curvePercentage.attach(plot);
	}
}

void PlotSectorHistogram::clear()
{
	const QVector<QwtIntervalSample> emptyDatas(0);
	_histogramData.setSamples(emptyDatas);
	_histogramMaximums.setSamples(emptyDatas);
	_histogramIntervals.setSamples(emptyDatas);
	_histogramCursor.setSamples(emptyDatas);

	_datasCurve->clear();
	_datasCurveMaximums->clear();
	_datasCurveIntervals->clear();
	_datasCurveCursor->clear();

	_curvePercentage.setSamples(QVector<QPointF>(0));
}

void PlotSectorHistogram::update( const Histogram<qreal> &histogram )
{
	const SectorHistogram &sectHist = dynamic_cast<const SectorHistogram &>(histogram);
	updateDatas(sectHist);
	updateMaximums(sectHist);
	updateIntervals(sectHist);
}

void PlotSectorHistogram::moveCursor( const uint &index )
{
	static QVector<QwtIntervalSample> datasCursor(1);
	if ( index < _histogramData.dataSize() )
	{
		const QwtIntervalSample &sample = _histogramData.sample(index);
		_datasCurveCursor->resize(4);
		(*_datasCurveCursor)[0].setAzimuth(sample.interval.maxValue()*TWO_PI/_histogramData.data()->size());
		(*_datasCurveCursor)[1].setAzimuth(sample.interval.minValue()*TWO_PI/_histogramData.data()->size());
		(*_datasCurveCursor)[1].setRadius(sample.value);
		(*_datasCurveCursor)[2].setAzimuth(sample.interval.minValue()*TWO_PI/_histogramData.data()->size());
		(*_datasCurveCursor)[2].setRadius(sample.value);
		(*_datasCurveCursor)[3].setAzimuth(sample.interval.minValue()*TWO_PI/_histogramData.data()->size());
		datasCursor[0] = sample;
	}
	else
	{
		_datasCurveCursor->resize(0);
	}
	_histogramCursor.setSamples(datasCursor);
}


/*******************************
 * Private functions
 *******************************/
void PlotSectorHistogram::updateDatas( const SectorHistogram &histogram )
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
			const PiePart &part = static_cast<SectorHistogram>(histogram).pieChart().sector(i);
			value = histogram[i];
			left = part.maxAngle();
			right = part.minAngle();
			(*beginCurve).setAzimuth(right);
			(*beginCurve++).setRadius(value);
			(*beginCurve).setAzimuth(left);
			(*beginCurve++).setRadius(value);
			(*beginHist).value = value;
			(*beginHist++).interval.setInterval(i,i+1);
		}
		(*beginCurve).setAzimuth(histogram.pieChart().sector(0).minAngle());
		(*beginCurve).setRadius(histogram[0]);
	}
	else
	{
		_datasCurve->resize(0);
	}
	_histogramData.setSamples(datasHistogram);
}

void PlotSectorHistogram::updateMaximums(const SectorHistogram &histogram )
{
	const int nbMaximums = histogram.nbMaximums();
	QVector<QwtIntervalSample> curveHistogramMaximumsDatas(nbMaximums);
	_datasCurveMaximums->resize(4*nbMaximums);
	if ( nbMaximums > 0 )
	{
		int sector;
		qreal left, right, value;
		const PieChart &pieChart = histogram.pieChart();
		QVector<uint>::ConstIterator begin = histogram.maximums().begin();
		const QVector<uint>::ConstIterator end = histogram.maximums().end();
		QVector<QwtPointPolar>::Iterator beginCurve = _datasCurveMaximums->begin();
		QVector<QwtIntervalSample>::Iterator beginHist = curveHistogramMaximumsDatas.begin();
		while ( begin != end )
		{
			sector = *begin++;
			left = pieChart.sector(sector).maxAngle();
			right = pieChart.sector(sector).minAngle();
			value = histogram[sector];
			(*beginCurve++).setAzimuth(right);
			(*beginCurve).setAzimuth(right);
			(*beginCurve++).setRadius(value);
			(*beginCurve).setAzimuth(left);
			(*beginCurve++).setRadius(value);
			(*beginCurve++).setAzimuth(left);
			(*beginHist).interval.setMinValue(sector);
			(*beginHist).interval.setMaxValue(sector+1);
			(*beginHist++).value = value;
		}
	}
	_histogramMaximums.setSamples(curveHistogramMaximumsDatas);
}

void PlotSectorHistogram::updateIntervals( const SectorHistogram &histogram )
{
	_datasCurveIntervals->clear();
	QVector<QwtIntervalSample> curveHistogramIntervalsDatas;
	const int nbIntervals = histogram.nbIntervals();
	if ( nbIntervals > 0 )
	{
		const PieChart &pieChart = histogram.pieChart();
		QVector< Interval<uint> >::ConstIterator begin = histogram.intervals().begin();
		const QVector< Interval<uint> >::ConstIterator end = histogram.intervals().end();
		const uint nbSectors = pieChart.nbSectors();
		uint min, max, i;
//		qDebug() << "Intervalles d'angles' :";
		while ( begin != end )
		{
			const Interval<uint> &currentInterval = *begin++;
			min = currentInterval.min();
			max = currentInterval.max();
			_datasCurveIntervals->append(QwtPointPolar( pieChart.sector(min).minAngle(), 0. ));
			if ( max<pieChart.nbSectors() )
			{
				for ( i=min ; i<=max ; ++i )
				{
					const PiePart &currentSector = pieChart.sector(i);
					_datasCurveIntervals->append(QwtPointPolar(currentSector.minAngle(),histogram[i]));
					_datasCurveIntervals->append(QwtPointPolar(currentSector.maxAngle(), histogram[i]));
					curveHistogramIntervalsDatas.append(QwtIntervalSample( histogram[i],i,i+1));
				}
			}
			else
			{
				max = max-nbSectors;
				for ( i=min ; i<nbSectors ; ++i )
				{
					const PiePart &currentSector = pieChart.sector(i);
					_datasCurveIntervals->append(QwtPointPolar(currentSector.minAngle(), histogram[i]));
					_datasCurveIntervals->append(QwtPointPolar(currentSector.maxAngle(), histogram[i]));
					curveHistogramIntervalsDatas.append(QwtIntervalSample( histogram[i],i,i+1));
				}
				for ( i=0 ; i<=max ; ++i )
				{
					const PiePart &currentSector = pieChart.sector(i);
					_datasCurveIntervals->append(QwtPointPolar(currentSector.minAngle(), histogram[i]));
					_datasCurveIntervals->append(QwtPointPolar(currentSector.maxAngle(), histogram[i]));
					curveHistogramIntervalsDatas.append(QwtIntervalSample( histogram[i],i,i+1));
				}
			}
			_datasCurveIntervals->append(QwtPointPolar( pieChart.sector(max).maxAngle(), 0. ));
//			qDebug() << "  [ " << min << ", " << max << " ] => [" << pieChart.sector(min).minAngle()*RAD_TO_DEG_FACT << ", " << pieChart.sector(max).maxAngle()*RAD_TO_DEG_FACT << "] avec largeur = " << (max>min?max-min:nbSectors-min+max);
		}
	}
	_histogramIntervals.setSamples(curveHistogramIntervalsDatas);
}
