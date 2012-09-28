#include "inc/piechartdiagrams.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/piechart.h"
#include "inc/piepart.h"
#include "inc/interval.h"
#include "inc/pointpolarseriesdata.h"
#include "inc/intervalscomputer.h"

#include <qwt_plot_histogram.h>
#include <qwt_polar_plot.h>
#include <qwt_plot.h>

#include <QPainter>

PieChartDiagrams::PieChartDiagrams() : Histogram(), _datasCurve(new PointPolarSeriesData()), _datasCurveMaximums(new PointPolarSeriesData()),
	_datasCurveIntervals(new PointPolarSeriesData()),  _highlightCurveDatas(new PointPolarSeriesData()), _pieChart(PieChart(0.,360))
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

const QVector<Interval> &PieChartDiagrams::branchesSectors() const
{
	return _intervals;
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
		_histogram.attach(plot);
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
	_histogram.setSamples(empty);

	_datasCurveMaximums->clear();
	_histogramMaximums.setSamples(empty);
	_maximums.clear();

	_datasCurveIntervals->clear();
	_histogramIntervals.setSamples(empty);

	_highlightCurveDatas->clear();
	_highlightCurveHistogram.setSamples(empty);

	_curvePercentage.setSamples(QVector<QPointF>(0));
}

void PieChartDiagrams::compute( const Billon &billon, const Marrow *marrow, const PieChart &pieChart, const Interval &slicesInterval, const Interval &intensity, const Interval &motionInterval,
								const int &smoothingRadius, const int &minimumHeightPercentageOfMaximum, const int &neighborhoodOfMaximums,
								const int &derivativesPercentage, const int &minimumWidthOfIntervals, const int &radiusAroundPith )
{
	if ( slicesInterval.isValid() && slicesInterval.width() > 0 )
	{
		_pieChart = pieChart;

		const uint width = billon.n_cols;
		const uint height = billon.n_rows;
		const int minValue = intensity.minValue();
		const int maxValue = intensity.maxValue();
		const int nbSectors = _pieChart.nbSectors();
		const uint minOfInterval = slicesInterval.minValue();
		const uint maxOfInterval = slicesInterval.maxValue();
		const int radiusMax = radiusAroundPith+1;
		const qreal squareRadius = qPow(radiusAroundPith,2);

		// Calcul des diagrammes en parcourant les tranches du billon comprises dans l'intervalle
		_datas.fill(0.,nbSectors);

		QList<int> circleLines;
		if ( marrow != 0 )
		{
			circleLines.reserve(2*radiusAroundPith+1);
			for ( int lineIndex=-radiusAroundPith ; lineIndex<radiusMax ; ++lineIndex )
			{
				circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
			}
		}

		int i, j, iRadius, iRadiusMax, sectorIdx;
		uint k, marrowX, marrowY, xPos, yPos;
		__billon_type__ currentSliceValue, previousSliceValue, diff;
		marrowX = width/2;
		marrowY = height/2;
		if ( marrow != 0 )
		{
			for ( k=minOfInterval ; k<maxOfInterval ; ++k )
			{
				const arma::Slice &currentSlice = billon.slice(k);
				const arma::Slice &prevSlice = billon.slice(k>0?k-1:k+1);
				marrowX = marrow->at(k).x;
				marrowY = marrow->at(k).y;
				for ( j=-radiusAroundPith ; j<radiusMax ; ++j )
				{
					iRadius = circleLines[j+radiusAroundPith];
					iRadiusMax = iRadius+1;
					for ( i=-iRadius ; i<iRadiusMax ; ++i )
					{
						xPos = marrowX+i;
						yPos = marrowY+j;
						if ( xPos < width && yPos < height )
						{
							sectorIdx = _pieChart.partOfAngle( TWO_PI-ANGLE(marrowX,marrowY,xPos,yPos) );
							currentSliceValue = currentSlice.at(yPos,xPos);
							previousSliceValue = prevSlice.at(yPos,xPos);
							if ( (currentSliceValue > minValue) && (previousSliceValue > minValue) )
							{
								diff = qAbs(RESTRICT_TO(minValue,currentSliceValue,maxValue) - RESTRICT_TO(minValue,previousSliceValue,maxValue));
								if ( motionInterval.containsClosed(diff) )
								{
									_datas[sectorIdx] += (diff-motionInterval.minValue());
								}
							}
						}
					}
				}
			}
		}
		else
		{
			for ( k=minOfInterval ; k<maxOfInterval ; ++k )
			{
				const arma::Slice &currentSlice = billon.slice(k);
				const arma::Slice &prevSlice = billon.slice(k>0?k-1:k+1);
				for ( j=0 ; j<static_cast<int>(height) ; ++j )
				{
					for ( i=0 ; i<static_cast<int>(width) ; ++i )
					{
						sectorIdx = _pieChart.partOfAngle( TWO_PI-ANGLE(marrowX,marrowY,i,j) );
						currentSliceValue = currentSlice.at(j,i);
						previousSliceValue = prevSlice.at(j,i);
						if ( (currentSliceValue > minValue) && (previousSliceValue > minValue) )
						{
							diff = qAbs(RESTRICT_TO(minValue,currentSliceValue,maxValue) - RESTRICT_TO(minValue,previousSliceValue,maxValue));
							if ( motionInterval.containsClosed(diff) )
							{
								_datas[sectorIdx] += (diff-motionInterval.maxValue());
							}
						}
					}
				}
			}
		}

		computeAll( smoothingRadius, minimumHeightPercentageOfMaximum, neighborhoodOfMaximums, derivativesPercentage, minimumWidthOfIntervals, true );

		computeValues();
		computeMaximums();
		computeIntervals();

		const qreal derivativeThreshold = thresholdOfMaximums( minimumHeightPercentageOfMaximum );
		const qreal x[] = { 0., TWO_PI };
		const qreal y[] = { derivativeThreshold, derivativeThreshold };
		_curvePercentage.setSamples(x,y,2);
	}
}

void PieChartDiagrams::highlightCurve( const int &index )
{
	QVector<QwtIntervalSample> highlightCurveHistogramDatas(1);
	if ( index > -1 && index < _datas.size() )
	{
		_highlightCurveDatas->resize(4);
		const qreal rightAngle = _pieChart.sector(index).rightAngle();
		const qreal leftAngle = _pieChart.sector(index).leftAngle();
		const qreal valueOfCurve = _datas[index];

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
void PieChartDiagrams::computeValues()
{
	const int nbSectors = _datas.size();
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
			const PiePart &part = _pieChart.sector(i);
			value = _datas[i];
			left = part.leftAngle();
			right = part.rightAngle();
			(*beginCurve).setAzimuth(right);
			(*beginCurve++).setRadius(value);
			(*beginCurve).setAzimuth(left);
			(*beginCurve++).setRadius(value);
			(*beginHist).value = value;
			(*beginHist++).interval.setInterval(right,left);
		}
		(*beginCurve).setAzimuth(_pieChart.sector(0).rightAngle());
		(*beginCurve).setRadius(_datas[0]);
	}
	else
	{
		_datasCurve->resize(0);
	}
	_histogram.setSamples(curveHistogramDatas);
}

void PieChartDiagrams::computeMaximums()
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
			left = _pieChart.sector(sector).leftAngle();
			right = _pieChart.sector(sector).rightAngle();
			value = _datas[sector];
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

void PieChartDiagrams::computeIntervals()
{
	_datasCurveIntervals->clear();
	QVector<QwtIntervalSample> curveHistogramIntervalsDatas;

	int nbMaximums = _maximums.size();
	if ( nbMaximums > 0 )
	{
		int min, max, i;
		Interval currentInterval;
		QVector<Interval>::ConstIterator begin = _intervals.begin();
		const QVector<Interval>::ConstIterator end = _intervals.end();
		const int nbSectors = _datas.size();
		qDebug() << "Intervalles d'angles' :";
		while ( begin != end )
		{
			currentInterval = *begin++;
			min = currentInterval.minValue();
			max = currentInterval.maxValue();
			if ( currentInterval.isValid() )
			{
				_datasCurveIntervals->append(QwtPointPolar( _pieChart.sector(min).rightAngle(), 0. ));
				for ( i=min ; i<max ; ++i )
				{
					const PiePart &currentSector = _pieChart.sector(i);
					_datasCurveIntervals->append(QwtPointPolar(currentSector.rightAngle(),_datas[i]));
					_datasCurveIntervals->append(QwtPointPolar(currentSector.leftAngle(),_datas[i]));
					curveHistogramIntervalsDatas.append(QwtIntervalSample(_datas[i],currentSector.rightAngle(),currentSector.leftAngle()));
				}
				_datasCurveIntervals->append(QwtPointPolar( _pieChart.sector(max).leftAngle(), 0. ));
			}
			else
			{
				_datasCurveIntervals->append(QwtPointPolar( _pieChart.sector(min).rightAngle(), 0. ));
				for ( i=min ; i<nbSectors ; ++i )
				{
					const PiePart &currentSector = _pieChart.sector(i);
					_datasCurveIntervals->append(QwtPointPolar(currentSector.rightAngle(),_datas[i]));
					_datasCurveIntervals->append(QwtPointPolar(currentSector.leftAngle(),_datas[i]));
					curveHistogramIntervalsDatas.append(QwtIntervalSample(_datas[i],currentSector.rightAngle(),currentSector.leftAngle()));
				}
				for ( i=0 ; i<max ; ++i )
				{
					const PiePart &currentSector = _pieChart.sector(i);
					_datasCurveIntervals->append(QwtPointPolar(currentSector.rightAngle(),_datas[i]));
					_datasCurveIntervals->append(QwtPointPolar(currentSector.leftAngle(),_datas[i]));
					curveHistogramIntervalsDatas.append(QwtIntervalSample(_datas[i],currentSector.rightAngle(),currentSector.leftAngle()));
				}
				_datasCurveIntervals->append(QwtPointPolar( _pieChart.sector(max>=0?max:nbSectors-1).leftAngle(), 0. ));
			}
			qDebug() << "  [ " << min << ", " << max << " ] => [" << _pieChart.sector(min).rightAngle()*RAD_TO_DEG_FACT << ", " << _pieChart.sector(max).leftAngle()*RAD_TO_DEG_FACT << "] avec largeur = " << max-min;
		}
	}
	_histogramIntervals.setSamples(curveHistogramIntervalsDatas);
}

void PieChartDiagrams::draw( QImage &image, const iCoord2D &center ) const
{
	if ( !_maximums.isEmpty() )
	{
		const int width = image.width();
		const int height = image.height();
		const int centerX = center.x;
		const int centerY = center.y;
		QColor colors[] = { Qt::blue, Qt::cyan, Qt::magenta };
//		colors[0].setAlpha(20);
//		colors[1].setAlpha(20);
//		colors[2].setAlpha(20);
		qreal angle, x1,y1,x2,y2; //,oldx,oldy;
		int i, nbBoucle;

		// Liste qui va contenir les angles des deux côté du secteur à dessiner
		// Permet de factoriser le code de calcul des coordonnées juste en dessous
		QList<qreal> twoSides;
		for ( i=0 ; i<_intervals.size() ; ++i )
		{
			const Interval &interval = _intervals[i];
			twoSides.append( TWO_PI-_pieChart.sector(interval.minValue()).orientation()-0.01 );
			twoSides.append( TWO_PI-_pieChart.sector(interval.maxValue()).orientation() );
		}

		// Dessin des deux côtés du secteur
		QPainter painter(&image);
		i = twoSides.size()-1;
		while ( i > 0 )
		{
			painter.setPen(colors[i%3]);
			painter.setBrush(colors[i%3]);
			nbBoucle = 2;
			while ( nbBoucle-- > 0 )
			{
				// Calcul des coordonnées du segment à tracer
				angle = twoSides[i--];
				x1 = x2 = centerX;
				y1 = y2 = centerY;
				if ( qFuzzyCompare(angle,PI_ON_TWO) ) y2 = height;
				else if ( qFuzzyCompare(angle,THREE_PI_ON_TWO) ) y2 = 0;
				else
				{
					const qreal a = tan(angle);
					const qreal b = centerY - (a*centerX);
					if ( angle < PI_ON_TWO || angle > THREE_PI_ON_TWO )
					{
						x2 = width;
						y2 = a*width+b;
					}
					else
					{
						x2 = 0;
						y2 = b;
					}
				}
				// Tracé du segment droit
				painter.drawLine(x1,y1,x2,y2);
//				if ( nbBoucle == 0 )
//				{
//					QPolygon polygon;
//					polygon.append(QPoint(x1,y1));
//					polygon.append(QPoint(x2,y2));
//					polygon.append(QPoint(oldx,oldy));
//					painter.drawPolygon(polygon);
//				}
//				else
//					{
//					oldx = x2;
//					oldy = y2;
//				}
			}
		}
	}
}
