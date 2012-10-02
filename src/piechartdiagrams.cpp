#include "inc/piechartdiagrams.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/piechart.h"
#include "inc/piepart.h"
#include "inc/interval.h"
#include "inc/pointpolarseriesdata.h"

#include <qwt_plot_histogram.h>
#include <qwt_polar_plot.h>
#include <qwt_plot.h>

#include <QPainter>

PieChartDiagrams::PieChartDiagrams() : Histogram(), _datasCurve(new PointPolarSeriesData()), _datasCurveMaximums(new PointPolarSeriesData()),
	_datasCurveIntervals(new PointPolarSeriesData()),  _highlightCurveDatas(new PointPolarSeriesData())
{
	_highlightCurve.setPen(QPen(Qt::red));
	_highlightCurveHistogram.setBrush(Qt::red);
	_highlightCurveHistogram.setPen(QPen(Qt::red));

	_curveMaximums.setPen(QPen(Qt::green));
	_histogramMaximums.setBrush(Qt::green);
	_histogramMaximums.setPen(QPen(Qt::green));

	_curveIntervals.setPen(QPen(Qt::blue));
	_histogramIntervals.setBrush(Qt::blue);
	_histogramIntervals.setPen(QPen(Qt::blue));

	_curvePercentage.setPen(QPen(Qt::red));

	_curve.setData(_datasCurve);
	_curveMaximums.setData(_datasCurveMaximums);
	_highlightCurve.setData(_highlightCurveDatas);
	_curveIntervals.setData(_datasCurveIntervals);
}

PieChartDiagrams::~PieChartDiagrams()
{
}

/*******************************
 * Public getters
 *******************************/

const QVector< Interval<int> > &PieChartDiagrams::knotIntervals() const
{
	return intervals();
}

/*******************************
 * Public setters
 *******************************/

void PieChartDiagrams::attach( QwtPolarPlot * const polarPlot )
{
	if ( polarPlot != 0 )
	{
		_curve.attach(polarPlot);
		_curveIntervals.attach(polarPlot);
		_curveMaximums.attach(polarPlot);
		_highlightCurve.attach(polarPlot);
	}
}

void PieChartDiagrams::attach( QwtPlot * const plot )
{
	if ( plot != 0 )
	{
		_plotHistogram.attach(plot);
		_histogramIntervals.attach(plot);
		_histogramMaximums.attach(plot);
		_highlightCurveHistogram.attach(plot);
		_curvePercentage.attach(plot);
	}
}

void PieChartDiagrams::clear()
{
	QVector<QwtIntervalSample> empty(0);

	_datasCurve->clear();
	_plotHistogram.setSamples(empty);

	_datasCurveMaximums->clear();
	_histogramMaximums.setSamples(empty);
	_maximums.clear();

	_datasCurveIntervals->clear();
	_histogramIntervals.setSamples(empty);

	_highlightCurveDatas->clear();
	_highlightCurveHistogram.setSamples(empty);

	_curvePercentage.setSamples(QVector<QPointF>(0));
}

void PieChartDiagrams::compute( const Billon &billon, const Marrow *marrow, const PieChart &pieChart, const Interval<int> &slicesInterval, const Interval<int> &intensity, const Interval<int> &motionInterval,
								const int &smoothingRadius, const int &minimumHeightPercentageOfMaximum, const int &neighborhoodOfMaximums,
								const int &derivativesPercentage, const int &minimumWidthOfIntervals, const int &radiusAroundPith )
{
	if ( slicesInterval.isValid() && slicesInterval.width() > 0 )
	{
		const int nbSectors = pieChart.nbSectors();
		const int radiusMax = radiusAroundPith+1;
		const qreal squareRadius = qPow(radiusAroundPith,2);

		fill(0.,nbSectors);

		QList<int> circleLines;
		if ( marrow != 0 )
		{
			circleLines.reserve(2*radiusAroundPith+1);
			for ( int lineIndex=-radiusAroundPith ; lineIndex<radiusMax ; ++lineIndex )
			{
				circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
			}
		}

		const int width = billon.n_cols;
		const int height = billon.n_rows;
		const uint minOfInterval = slicesInterval.min();
		const uint maxOfInterval = slicesInterval.max();
		int i, j, iRadius, iRadiusMax, sectorIdx;
		__billon_type__ currentSliceValue, previousSliceValue, diff;
		iCoord2D currentPos, marrowCoord(width/2,height/2);
		uint k;

		// Calcul du diagramme en parcourant les tranches du billon comprises dans l'intervalle
		for ( k=minOfInterval ; k<maxOfInterval ; ++k )
		{
			const arma::Slice &currentSlice = billon.slice(k);
			const arma::Slice &prevSlice = billon.slice(k>0?k-1:k+1);
			if ( marrow != 0 ) marrowCoord = marrow->at(k);
			for ( j=-radiusAroundPith ; j<radiusMax ; ++j )
			{
				iRadius = circleLines[j+radiusAroundPith];
				iRadiusMax = iRadius+1;
				for ( i=-iRadius ; i<iRadiusMax ; ++i )
				{
					currentPos.x = marrowCoord.x+i;
					currentPos.y = marrowCoord.y+j;
					if ( currentPos.x < width && currentPos.y < height )
					{
						sectorIdx = pieChart.sectorIndexOfAngle( marrowCoord.angle(currentPos) );
						currentSliceValue = currentSlice.at(currentPos.y,currentPos.x);
						previousSliceValue = prevSlice.at(currentPos.y,currentPos.x);
						if ( intensity.containsOpen(currentSliceValue) && intensity.containsOpen(previousSliceValue) )
						{
							diff = qAbs(currentSliceValue - previousSliceValue);
							if ( motionInterval.containsClosed(diff) )
							{
								(*this)[sectorIdx] += (diff-motionInterval.min());
							}
						}
					}
				}
			}
		}

		computeAll( smoothingRadius, minimumHeightPercentageOfMaximum, neighborhoodOfMaximums, derivativesPercentage, minimumWidthOfIntervals, true );

		computeValues(pieChart);
		computeMaximums(pieChart);
		computeIntervals(pieChart);

		const qreal derivativeThreshold = thresholdOfMaximums( minimumHeightPercentageOfMaximum );
		const qreal x[] = { 0., TWO_PI };
		const qreal y[] = { derivativeThreshold, derivativeThreshold };
		_curvePercentage.setSamples(x,y,2);
	}
}

void PieChartDiagrams::highlightCurve( const int &index, const PieChart &pieChart )
{
	QVector<QwtIntervalSample> highlightCurveHistogramDatas(1);
	if ( index > -1 && index < this->size() )
	{
		_highlightCurveDatas->resize(4);
		const qreal rightAngle = pieChart.sector(index).rightAngle();
		const qreal leftAngle = pieChart.sector(index).leftAngle();
		const qreal valueOfCurve = this->at(index);

		_highlightCurveDatas->at(0).setAzimuth(rightAngle);
		_highlightCurveDatas->at(1).setAzimuth(rightAngle);
		_highlightCurveDatas->at(1).setRadius(valueOfCurve);
		_highlightCurveDatas->at(2).setAzimuth(leftAngle);
		_highlightCurveDatas->at(2).setRadius(valueOfCurve);
		_highlightCurveDatas->at(3).setAzimuth(leftAngle);

		highlightCurveHistogramDatas[0].value = valueOfCurve;
		highlightCurveHistogramDatas[0].interval.setInterval(rightAngle,leftAngle);
	}
	else
	{
		_highlightCurveDatas->resize(0);
	}
	_highlightCurveHistogram.setSamples(highlightCurveHistogramDatas);
	_highlightCurve.plot()->replot();
	_highlightCurveHistogram.plot()->replot();
}

/*******************************
 * Private functions
 *******************************/
void PieChartDiagrams::computeValues( const PieChart &pieChart )
{
	const int nbSectors = this->size();
	QVector<QwtIntervalSample> curveHistogramDatas(0);
	if ( nbSectors > 0 )
	{
		curveHistogramDatas.fill(QwtIntervalSample(),nbSectors);
		_datasCurve->resize(2*nbSectors+1);
		QVector<QwtPointPolar>::Iterator beginCurve = _datasCurve->begin();
		QVector<QwtIntervalSample>::Iterator beginHist = curveHistogramDatas.begin();
		int i, value;
		qreal left, right;
		for ( i=0 ; i<nbSectors ; ++i )
		{
			const PiePart &part = pieChart.sector(i);
			value = this->at(i);
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
		(*beginCurve).setRadius(this->at(0));
	}
	else
	{
		_datasCurve->resize(0);
	}
	_plotHistogram.setSamples(curveHistogramDatas);
}

void PieChartDiagrams::computeMaximums( const PieChart &pieChart )
{
	const int nbMaximums = _maximums.size();
	QVector<QwtIntervalSample> curveHistogramMaximumsDatas(nbMaximums);
	_datasCurveMaximums->resize(4*nbMaximums);
	if ( nbMaximums > 0 )
	{
		int sector;
		qreal left, right, value;
		QVector<int>::ConstIterator begin = _maximums.begin();
		const QVector<int>::ConstIterator end = _maximums.end();
		QVector<QwtPointPolar>::Iterator beginCurve = _datasCurveMaximums->begin();
		QVector<QwtIntervalSample>::Iterator beginHist = curveHistogramMaximumsDatas.begin();
		while ( begin != end )
		{
			sector = *begin++;
			left = pieChart.sector(sector).leftAngle();
			right = pieChart.sector(sector).rightAngle();
			value = at(sector);
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

void PieChartDiagrams::computeIntervals( const PieChart & pieChart )
{
	_datasCurveIntervals->clear();
	QVector<QwtIntervalSample> curveHistogramIntervalsDatas;

	int nbMaximums = _maximums.size();
	if ( nbMaximums > 0 )
	{
		int min, max, i;
		Interval<int> currentInterval;
		QVector< Interval<int> >::ConstIterator begin = _intervals.begin();
		const QVector< Interval<int> >::ConstIterator end = _intervals.end();
		const int nbSectors = this->size();
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
					_datasCurveIntervals->append(QwtPointPolar(currentSector.rightAngle(),this->at(i)));
					_datasCurveIntervals->append(QwtPointPolar(currentSector.leftAngle(), this->at(i)));
					curveHistogramIntervalsDatas.append(QwtIntervalSample( this->at(i),currentSector.rightAngle(),currentSector.leftAngle()));
				}
				_datasCurveIntervals->append(QwtPointPolar( pieChart.sector(max).leftAngle(), 0. ));
			}
			else
			{
				_datasCurveIntervals->append(QwtPointPolar( pieChart.sector(min).rightAngle(), 0. ));
				for ( i=min ; i<nbSectors ; ++i )
				{
					const PiePart &currentSector = pieChart.sector(i);
					_datasCurveIntervals->append(QwtPointPolar(currentSector.rightAngle(), this->at(i)));
					_datasCurveIntervals->append(QwtPointPolar(currentSector.leftAngle(), this->at(i)));
					curveHistogramIntervalsDatas.append(QwtIntervalSample( this->at(i),currentSector.rightAngle(),currentSector.leftAngle()));
				}
				for ( i=0 ; i<max ; ++i )
				{
					const PiePart &currentSector = pieChart.sector(i);
					_datasCurveIntervals->append(QwtPointPolar(currentSector.rightAngle(), this->at(i)));
					_datasCurveIntervals->append(QwtPointPolar(currentSector.leftAngle(), this->at(i)));
					curveHistogramIntervalsDatas.append(QwtIntervalSample( this->at(i),currentSector.rightAngle(),currentSector.leftAngle()));
				}
				_datasCurveIntervals->append(QwtPointPolar( pieChart.sector(max>=0?max:nbSectors-1).leftAngle(), 0. ));
			}
			qDebug() << "  [ " << min << ", " << max << " ] => [" << pieChart.sector(min).rightAngle()*RAD_TO_DEG_FACT << ", " << pieChart.sector(max).leftAngle()*RAD_TO_DEG_FACT << "] avec largeur = " << max-min;
		}
	}
	_histogramIntervals.setSamples(curveHistogramIntervalsDatas);
}

void PieChartDiagrams::draw( QImage &image, const iCoord2D &center, const PieChart & pieChart ) const
{
	if ( !_maximums.isEmpty() )
	{
		// Liste qui va contenir les angles des deux côté du secteur à dessiner
		// Permet de factoriser le code de calcul des coordonnées juste en dessous
		QList<qreal> twoSides;
		QVector< Interval<int> >::const_iterator interval;
		for ( interval = _intervals.constBegin() ; interval < _intervals.constEnd() ; ++interval )
		{
			twoSides.append( pieChart.sector((*interval).min()).orientation()-0.01 );
			twoSides.append( pieChart.sector((*interval).max()).orientation() );
		}

		// Dessin des deux côtés du secteur
		const int width = image.width();
		const int height = image.height();
		const QColor colors[] = { Qt::blue, Qt::cyan, Qt::magenta };
		QColor color;
		iCoord2D end;
		int colorIndex = 0;

		QPainter painter(&image);
		QList<qreal>::const_iterator angle;
		for ( angle = twoSides.constBegin() ; angle < twoSides.constEnd() ; ++angle )
		{
			// Calcul des coordonnées du segment à tracer
			color = colors[(colorIndex++/2)%3];
			painter.setPen(color);
			painter.setBrush(color);
			end = center;
			if ( qFuzzyCompare(*angle,PI_ON_TWO) ) end.y = height;
			else if ( qFuzzyCompare(*angle,THREE_PI_ON_TWO) ) end.y = 0;
			else
			{
				const qreal a = qTan(*angle);
				const qreal b = center.y - (a*center.x);
				if ( (*angle) < PI_ON_TWO || (*angle) > THREE_PI_ON_TWO ) end = iCoord2D(width,a*width+b);
				else end = iCoord2D(0,b);
			}
			// Tracé du segment
			painter.drawLine(center.x,center.y,end.x,end.y);
		}
	}
}
