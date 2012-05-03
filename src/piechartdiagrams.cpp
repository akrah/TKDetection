#include "inc/piechartdiagrams.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/piechart.h"
#include "inc/piepart.h"
#include "inc/slicesinterval.h"
#include "inc/intensityinterval.h"
#include "inc/pointpolarseriesdata.h"

#include <qwt_plot_histogram.h>
#include <qwt_polar_plot.h>
#include <qwt_plot.h>

#include <QPainter>

PieChartDiagrams::PieChartDiagrams() : _curveDatas(new PointPolarSeriesData()), _curveMaximumsDatas(new PointPolarSeriesData()), _curveIntervalsDatas(new PointPolarSeriesData()), _highlightCurveDatas(new PointPolarSeriesData()),
	_pieChart(PieChart(0.,100)), _movementsThresholdMin(200), _movementsThresholdMax(500), _marrowAroundDiameter(100), _intervalType(HistogramIntervalType::FROM_MIDDLE_OF_MEANS_AND_MEDIAN), _smoothing(true)
{
	_highlightCurve.setPen(QPen(Qt::red));
	_highlightCurveHistogram.setBrush(Qt::red);
	_highlightCurveHistogram.setPen(QPen(Qt::red));

	_curveMaximums.setPen(QPen(Qt::green));
	_curveHistogramMaximums.setBrush(Qt::green);
	_curveHistogramMaximums.setPen(QPen(Qt::green));

	_curveIntervals.setPen(QPen(Qt::blue));
	_curveHistogramIntervals.setBrush(Qt::blue);
	_curveHistogramIntervals.setPen(QPen(Qt::blue));

	_curveMeans.setPen(QPen(Qt::red));
	_curveMedian.setPen(QPen(Qt::red));
	_curveMeansMedian.setPen(QPen(Qt::red));

	_curve.setData(_curveDatas);
	_curveMaximums.setData(_curveMaximumsDatas);
	_highlightCurve.setData(_highlightCurveDatas);
	_curveIntervals.setData(_curveIntervalsDatas);
}

PieChartDiagrams::~PieChartDiagrams() {
}

/*******************************
 * Public getters
 *******************************/

int PieChartDiagrams::count() const {
	return _curveHistogramDatas.size();
}

const QVector<QwtInterval> &PieChartDiagrams::branchesSectors() const {
	return _curveHistogramIntervalsRealDatas;
}

/*******************************
 * Public setters
 *******************************/

void PieChartDiagrams::attach( QwtPolarPlot * const polarPlot ) {
	if ( polarPlot != 0 ) {
		_curve.attach(polarPlot);
		_curveIntervals.attach(polarPlot);
		_curveMaximums.attach(polarPlot);
		_highlightCurve.attach(polarPlot);
	}
}

void PieChartDiagrams::attach( QwtPlot * const plot ) {
	if ( plot != 0 ) {
		_curveHistogram.attach(plot);
		_curveHistogramIntervals.attach(plot);
		_curveHistogramMaximums.attach(plot);
		_highlightCurveHistogram.attach(plot);
		_curveMeans.detach();
		_curveMedian.detach();
		_curveMeansMedian.detach();
		if ( _intervalType == HistogramIntervalType::FROM_MEANS ) {
			_curveMeans.attach(_curveHistogram.plot());
		}
		else if ( _intervalType == HistogramIntervalType::FROM_MEDIAN ) {
			_curveMedian.attach(_curveHistogram.plot());
		}
		else if ( _intervalType == HistogramIntervalType::FROM_MIDDLE_OF_MEANS_AND_MEDIAN ) {
			_curveMeansMedian.attach(_curveHistogram.plot());
		}
	}
}

void PieChartDiagrams::clearAll() {
	_curveDatas->clear();
	_curveHistogramDatas.clear();
	_curveHistogram.setSamples(_curveHistogramDatas);

	_curveMaximumsDatas->clear();
	_curveHistogramMaximumsDatas.clear();
	_curveHistogramMaximums.setSamples(_curveHistogramMaximumsDatas);
	_maximumsIndex.clear();

	_curveIntervalsDatas->clear();
	_curveHistogramIntervalsDatas.clear();
	_curveHistogramIntervals.setSamples(_curveHistogramIntervalsDatas);
	_curveHistogramIntervalsRealDatas.clear();

	_highlightCurveDatas->clear();
	_highlightCurveHistogramDatas.clear();
	_highlightCurveHistogram.setSamples(_highlightCurveHistogramDatas);

	_curveMeans.detach();
	_curveMedian.detach();
	_curveMeansMedian.detach();
}

void PieChartDiagrams::setMovementsThresholdMin( const int &threshold ) {
	_movementsThresholdMin = threshold;
}

void PieChartDiagrams::setMovementsThresholdMax( const int &threshold ) {
	_movementsThresholdMax = threshold;
}

void PieChartDiagrams::useNextSliceInsteadOfCurrentSlice( const bool &enable ) {
	_useNextSlice = enable;
}

void PieChartDiagrams::setMarrowAroundDiameter( const int &diameter ) {
	_marrowAroundDiameter = diameter;
}

void PieChartDiagrams::setIntervalType( const HistogramIntervalType::HistogramIntervalType &type ) {
	if ( type > HistogramIntervalType::_HIST_INTERVAL_TYPE_MIN_ && type < HistogramIntervalType::_HIST_INTERVAL_TYPE_MAX__ ) {
		_intervalType = type;
		_curveMeans.detach();
		_curveMedian.detach();
		_curveMeansMedian.detach();
		if ( _intervalType == HistogramIntervalType::FROM_MEANS ) {
			_curveMeans.attach(_curveHistogram.plot());
		}
		else if ( _intervalType == HistogramIntervalType::FROM_MEDIAN ) {
			_curveMedian.attach(_curveHistogram.plot());
		}
		else if ( _intervalType == HistogramIntervalType::FROM_MIDDLE_OF_MEANS_AND_MEDIAN ) {
			_curveMeansMedian.attach(_curveHistogram.plot());
		}
	}
}

void PieChartDiagrams::enableSmoothing( const bool &enable ) {
	_smoothing = enable;
}

void PieChartDiagrams::compute( const Billon &billon, const Marrow *marrow, const PieChart &pieChart, const SlicesInterval &slicesInterval, const IntensityInterval &intensity ) {
	if ( slicesInterval.isValid() ) {
		_pieChart = pieChart;

		const uint width = billon.n_cols;
		const uint height = billon.n_rows;
		const int minValue = intensity.min();
		const int maxValue = intensity.max();
		const int nbSectors = _pieChart.nbSectors();
		const uint minOfInterval = slicesInterval.min();
		const uint maxOfInterval = slicesInterval.max();
		const int diameter = _marrowAroundDiameter;
		const int radius = diameter/(2.*billon.voxelWidth());
		const int radiusMax = radius+1;
		const qreal squareRadius = qPow(radius,2);

		// Calcul des diagrammes en parcourant les tranches du billon comprises dans l'intervalle
		QVector<qreal> sectorsSum(nbSectors,0.);

		QList<int> circleLines;
		if ( marrow != 0 ) {
			circleLines.reserve(2*radius+1);
			for ( int lineIndex=-radius ; lineIndex<radiusMax ; ++lineIndex ) {
				circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
			}
		}

		int i, j, iRadius, iRadiusMax, sectorIdx;
		uint k, marrowX, marrowY, xPos, yPos;
		__billon_type__ currentSliceValue, previousSliceValue, diff;
		marrowX = width/2;
		marrowY = height/2;
		for ( k=minOfInterval==0?1:minOfInterval ; k<maxOfInterval ; ++k ) {
			const arma::Slice &slice = _useNextSlice?billon.slice(k+1):billon.slice(k);
			const arma::Slice &prevSlice = billon.slice(k-1);
			if ( marrow != 0 ) {
				marrowX = marrow->at(k).x;
				marrowY = marrow->at(k).y;
				for ( j=-radius ; j<radiusMax ; ++j ) {
					iRadius = circleLines[j+radius];
					iRadiusMax = iRadius+1;
					for ( i=-iRadius ; i<iRadiusMax ; ++i ) {
						xPos = marrowX+i;
						yPos = marrowY+j;
						if ( xPos < width && yPos < height ) {
							sectorIdx = _pieChart.partOfAngle( TWO_PI-ANGLE(marrowX,marrowY,xPos,yPos) );
							currentSliceValue = slice.at(yPos,xPos);
							previousSliceValue = prevSlice.at(yPos,xPos);
							if ( (currentSliceValue > minValue) && (previousSliceValue > minValue) ) {
								diff = qAbs(qBound(minValue,currentSliceValue,maxValue) - qBound(minValue,previousSliceValue,maxValue));
								if ( (diff >= _movementsThresholdMin) && (diff <= _movementsThresholdMax) ) {
									sectorsSum[sectorIdx] += (diff-_movementsThresholdMin);
								}
							}
						}
					}
				}
			}
			else {
				for ( j=0 ; j<static_cast<int>(height) ; ++j ) {
					for ( i=0 ; i<static_cast<int>(width) ; ++i ) {
						sectorIdx = _pieChart.partOfAngle( TWO_PI-ANGLE(marrowX,marrowY,i,j) );
						currentSliceValue = slice.at(j,i);
						previousSliceValue = prevSlice.at(j,i);
						if ( (currentSliceValue > minValue) && (previousSliceValue > minValue) ) {
							diff = qAbs(qBound(minValue,currentSliceValue,maxValue) - qBound(minValue,previousSliceValue,maxValue));
							if ( diff >= _movementsThresholdMin && diff <= _movementsThresholdMax ) {
								sectorsSum[sectorIdx] += (diff-_movementsThresholdMin);
							}
						}
					}
				}
			}
		}

		if ( _smoothing ) smoothHistogram( sectorsSum );
		createDiagrams( sectorsSum );
		computeMeansAndMedian( sectorsSum );
		computeMaximums( sectorsSum );
		computeIntervals( sectorsSum );
	}
}

void PieChartDiagrams::highlightCurve( const int &index ) {
	_highlightCurveDatas->clear();
	_highlightCurveHistogramDatas.clear();
	if ( index > -1 && index < count() ) {
		const qreal rightAngle = _pieChart.sector(index).rightAngle();
		const qreal leftAngle = _pieChart.sector(index).leftAngle();
		const qreal valueOfCurve = _curveHistogramDatas[index].value;

		_highlightCurveDatas->append(QwtPointPolar(rightAngle,0.));
		_highlightCurveDatas->append(QwtPointPolar(rightAngle,valueOfCurve));
		_highlightCurveDatas->append(QwtPointPolar(leftAngle,valueOfCurve));
		_highlightCurveDatas->append(QwtPointPolar(leftAngle,0.));

		_highlightCurveHistogramDatas.append(QwtIntervalSample(valueOfCurve,rightAngle,leftAngle));
	}
	_highlightCurveHistogram.setSamples(_highlightCurveHistogramDatas);
	_highlightCurve.plot()->replot();
	_highlightCurveHistogram.plot()->replot();
}

/*******************************
 * Private functions
 *******************************/
void PieChartDiagrams::createDiagrams( const QVector<qreal> &sectorsSum ) {
	const int nbSectors = sectorsSum.size();
	_curveDatas->clear();
	_curveHistogramDatas.clear();
	if ( nbSectors > 0 ) {
		int i, value;
		_curveHistogramDatas.fill(QwtIntervalSample(),nbSectors);
		for ( i=0 ; i<nbSectors ; ++i ) {
			const PiePart &part = _pieChart.sector(i);
			value = sectorsSum[i];
			_curveDatas->append(QwtPointPolar(part.rightAngle(),value));
			_curveDatas->append(QwtPointPolar(part.leftAngle(),value));
			_curveHistogramDatas[i].value = value;
			_curveHistogramDatas[i].interval.setInterval(part.rightAngle(),part.leftAngle());
		}
		_curveDatas->append(QwtPointPolar(_pieChart.sector(0).rightAngle(),sectorsSum[0]));
	}
	_curveHistogram.setSamples(_curveHistogramDatas);
}

void PieChartDiagrams::smoothHistogram( QVector<qreal> &sectorsSum ) {
	int i = 0;
	qreal veryOldValue, oldValue, currentValue, firstValue, secondValue;
	const int nbSectors = sectorsSum.size()-2;
	veryOldValue = sectorsSum[nbSectors];
	oldValue = sectorsSum[nbSectors+1];
	firstValue = sectorsSum[0];
	secondValue = sectorsSum[1];
	for ( i=0 ; i<nbSectors ; ++i ) {
		currentValue = sectorsSum[i];
		sectorsSum[i] = (veryOldValue + oldValue + currentValue + sectorsSum[i+1] + sectorsSum[i+2])/5.;
		veryOldValue = oldValue;
		oldValue = currentValue;
	}
	currentValue = sectorsSum[i];
	sectorsSum[i] = (veryOldValue + oldValue + currentValue + sectorsSum[i+1] + firstValue)/5.;
	veryOldValue = oldValue;
	oldValue = currentValue;
	i++;
	currentValue = sectorsSum[i];
	sectorsSum[i] = (veryOldValue + oldValue + currentValue + firstValue + secondValue)/5.;
}

void PieChartDiagrams::computeMeansAndMedian( const QVector<qreal> &sectorsSum ) {
	const int nbSectors = sectorsSum.size();
	qreal xMeans[2] = { 0., TWO_PI };
	qreal yMeans[2] = { 0., 0. };
	qreal xMedian[2] = { 0., TWO_PI };
	qreal yMedian[2] = { 0., 0. };
	qreal xMeansMedian[2] = { 0., TWO_PI };
	qreal yMeansMedian[2] = { 0., 0. };
	_dataMeans = 0.;
	_dataMedian = 0.;
	_dataMeansMedian = 0.;
	if ( nbSectors > 0 ) {
		// Moyenne
		for ( int i=0 ; i<nbSectors ; ++i ) {
			_dataMeans += sectorsSum[i];
		}
		_dataMeans /= static_cast<qreal>(nbSectors);
		// Mediane
		QVector<qreal> sortedList(sectorsSum);
		qSort(sortedList);
		if ( nbSectors % 2 == 0 ) _dataMedian = (sortedList[(nbSectors/2)-1]+sortedList[nbSectors/2])/2.;
		else _dataMedian = sortedList[(nbSectors+1)/2-1];
		// Milieu Moyenne-Mediane
		_dataMeansMedian = (_dataMeans+_dataMedian)/2.;

		yMeans[0] = yMeans[1] = _dataMeans;
		yMedian[0] = yMedian[1] = _dataMedian;
		yMeansMedian[0] = yMeansMedian[1] = _dataMeansMedian;
	}
	_curveMeans.setSamples(xMeans,yMeans,2);
	_curveMedian.setSamples(xMedian,yMedian,2);
	_curveMeansMedian.setSamples(xMeansMedian,yMeansMedian,2);
}

void PieChartDiagrams::computeMaximums( const QVector<qreal> &sectorsSum ) {
	const int nbSectors = sectorsSum.size();
	_curveMaximumsDatas->clear();
	_curveHistogramMaximumsDatas.clear();
	_maximumsIndex.clear();
	if ( nbSectors > 0 ) {
		double value;
		int i, cursor;
		bool isMax;
//		qreal limit;
//		TODO : Ce seuil ne doit pas être identique à celui de l'histogramme de coupes car on perd trop de branches.
//				Il y a donc un seuil à 100 en attendant.
//		limit = 100;
//		if ( _intervalType == HistogramIntervalType::FROM_EDGE ) limit = 0;
//		else limit = _intervalType==HistogramIntervalType::FROM_MEANS?_dataMeans:_intervalType==HistogramIntervalType::FROM_MEDIAN?_dataMedian:_dataMeansMedian;
		qDebug() << "Pics angulaires :";
		for ( i=0 ; i<nbSectors ; ++i ) {
			value = sectorsSum[i];
			//if ( value > limit ) {
			if ( value > 0 ) {
				cursor = 1;
				do {
					isMax = ( (value > sectorsSum[i-cursor>=0?i-cursor:nbSectors+i-cursor]) && (value > sectorsSum[i+cursor<nbSectors?i+cursor:i+cursor-nbSectors]) );
					cursor++;
				}
				while ( isMax && cursor<10 );  //TOUJOURS VRAI SI CURSOR COMMENCE A 1
				//if ( (value > sectorsSum[i>=1?i-1:nbSectors+i-1]) && (value > sectorsSum[i+1<nbSectors?i+1:i+1-nbSectors]) ) {
				if ( isMax ) {
					const PiePart &part = _pieChart.sector(i);
					_curveMaximumsDatas->append(QwtPointPolar(part.rightAngle(),0.));
					_curveMaximumsDatas->append(QwtPointPolar(part.rightAngle(),value));
					_curveMaximumsDatas->append(QwtPointPolar(part.leftAngle(),value));
					_curveMaximumsDatas->append(QwtPointPolar(part.leftAngle(),0.));
					_curveHistogramMaximumsDatas.append(_curveHistogramDatas[i]);
					_maximumsIndex.append(i);
					i+=cursor-1;
					qDebug() << i;
				}
			}
		}
	}
	_curveHistogramMaximums.setSamples(_curveHistogramMaximumsDatas);
}

namespace {
	inline qreal firstdDerivated( const QVector<qreal> &sectorsSum, const int &index ) {
		return sectorsSum[index] - sectorsSum[index>0?index-1:sectorsSum.size()-1];
	}
}

void PieChartDiagrams::computeIntervals( const QVector<qreal> &sectorsSum ) {
	_curveIntervalsDatas->clear();
	_curveHistogramIntervalsDatas.clear();
	_curveHistogramIntervalsRealDatas.clear();
	if ( !_maximumsIndex.isEmpty() ) {
		const int nbSectors = sectorsSum.size();
		int nbMaximums = _maximumsIndex.size();
		int cursorMax, cursorMin;
		qreal derivated;
		bool isSupToLimit;
		QVector<QwtIntervalSample> setOfIntervals;
		setOfIntervals.reserve(nbSectors);
		if ( _intervalType != HistogramIntervalType::FROM_EDGE ) {
			qreal limit = _intervalType==HistogramIntervalType::FROM_MEANS?_dataMeans:_intervalType==HistogramIntervalType::FROM_MEDIAN?_dataMedian:_dataMeansMedian;
			for ( int i=0 ; i<nbMaximums ; ++i ) {
				setOfIntervals.clear();
				cursorMin = _maximumsIndex[i];
				// Si c'est le premier intervalle ou que le maximum courant n'est pas compris dans l'intervalle précédent.
				if ( _curveHistogramIntervalsRealDatas.isEmpty() || _curveHistogramIntervalsRealDatas.last().maxValue() < cursorMin ) {
					// On recherche les bornes min et max des potentielles de l'intervalle contenant le ième maximum
					derivated = firstdDerivated(sectorsSum,cursorMin);
					isSupToLimit = sectorsSum[cursorMin] > limit;
					while ( isSupToLimit || derivated > 0. ) {
						setOfIntervals.append(_curveHistogramDatas[cursorMin]);
						cursorMin--;
						if ( cursorMin < 0 ) cursorMin = nbSectors-1;
						derivated = firstdDerivated(sectorsSum,cursorMin);
						isSupToLimit &= sectorsSum[cursorMin] > limit;
					}

					cursorMax = _maximumsIndex[i]+1;
					if ( cursorMax == nbSectors ) cursorMax = 0;
					derivated = firstdDerivated(sectorsSum,cursorMax);
					isSupToLimit = sectorsSum[cursorMax] > limit;
					while ( isSupToLimit || derivated < 0. ) {
						setOfIntervals.append(_curveHistogramDatas[cursorMax]);
						cursorMax++;
						if ( cursorMax == nbSectors ) cursorMax = 0;
						derivated = firstdDerivated(sectorsSum,cursorMax);
						isSupToLimit &= sectorsSum[cursorMax] > limit;
					}
					cursorMax--;
					if ( cursorMax<0 ) cursorMax = nbSectors-1;

					// Si c'est le premier intervalle ou que le maximum courant n'est pas compris dans l'intervalle précédent.
					if ( _curveHistogramIntervalsRealDatas.isEmpty() || _curveHistogramIntervalsRealDatas.first().maxValue() != cursorMax ) {
						// Si les bornes définissent un intervalle qui ne contient pas 0.
						if ( cursorMax>cursorMin && qAbs(cursorMax-cursorMin) > 1 /*_minimumIntervalWidth*/ ) {
							_curveIntervalsDatas->append(QwtPointPolar( _pieChart.sector(cursorMin).rightAngle(), 0. ));
							for ( int j=cursorMin ; j<cursorMax ; ++j ) {
								_curveIntervalsDatas->append(QwtPointPolar(_pieChart.sector(j).rightAngle(),sectorsSum[j]));
								_curveIntervalsDatas->append(QwtPointPolar(_pieChart.sector(j).leftAngle(),sectorsSum[j]));
							}
							_curveIntervalsDatas->append(QwtPointPolar( _pieChart.sector(cursorMax).leftAngle(), 0. ));
						}
						// Si les bornes définissent bien un intervalle... mais qu'il contient 0.
						else if ( cursorMax<cursorMin && qAbs(cursorMax-(cursorMin-nbSectors)) > 1 /*_minimumIntervalWidth*/ ) {
							_curveIntervalsDatas->append(QwtPointPolar( _pieChart.sector(cursorMin).rightAngle(), 0. ));
							for ( int j=cursorMin ; j<nbSectors ; ++j ) {
								_curveIntervalsDatas->append(QwtPointPolar(_pieChart.sector(j).rightAngle(),sectorsSum[j]));
								_curveIntervalsDatas->append(QwtPointPolar(_pieChart.sector(j).leftAngle(),sectorsSum[j]));
							}
							for ( int j=0 ; j<cursorMax ; ++j ) {
								_curveIntervalsDatas->append(QwtPointPolar(_pieChart.sector(j).rightAngle(),sectorsSum[j]));
								_curveIntervalsDatas->append(QwtPointPolar(_pieChart.sector(j).leftAngle(),sectorsSum[j]));
							}
							_curveIntervalsDatas->append(QwtPointPolar( _pieChart.sector(cursorMax>=0?cursorMax:nbSectors-1).leftAngle(), 0. ));
						}
						_curveHistogramIntervalsDatas << setOfIntervals;
						_curveHistogramIntervalsRealDatas.append(QwtInterval(cursorMin,cursorMax));
					}
					else {
						i=nbMaximums;
					}
				}
			}
		}
		else {
			for ( int i=0 ; i<nbMaximums ; ++i ) {
				setOfIntervals.clear();
				// On recherche les bornes min et max des potentielles de l'intervalle contenant le ième maximum
				cursorMin = _maximumsIndex[i];
				derivated = firstdDerivated(sectorsSum,cursorMin);
				while ( derivated > 0. ) {
					setOfIntervals.append(_curveHistogramDatas[cursorMin]);
					cursorMin--;
					if ( cursorMin < 0 ) cursorMin = nbSectors-1;
					derivated = firstdDerivated(sectorsSum,cursorMin);
				}
				cursorMax = _maximumsIndex[i]+1;
				if ( cursorMax == nbSectors ) cursorMax = 0;
				derivated = firstdDerivated(sectorsSum,cursorMax);
				while ( derivated < 0. ) {
					setOfIntervals.append(_curveHistogramDatas[cursorMax]);
					cursorMax++;
					if ( cursorMax == nbSectors ) cursorMax = 0;
					derivated = firstdDerivated(sectorsSum,cursorMax);
				}
				cursorMax--;
				if ( cursorMax<0 ) cursorMax = nbSectors-1;

				// Si les bornes définissent bien un intervalle qui ne contient pas 0.
				if ( cursorMax>cursorMin && qAbs(cursorMax-cursorMin) > 1 /*_minimumIntervalWidth*/ ) {
					_curveIntervalsDatas->append(QwtPointPolar( _pieChart.sector(cursorMin).rightAngle(), 0. ));
					for ( int j=cursorMin ; j<cursorMax ; ++j ) {
						_curveIntervalsDatas->append(QwtPointPolar(_pieChart.sector(j).rightAngle(),sectorsSum[j]));
						_curveIntervalsDatas->append(QwtPointPolar(_pieChart.sector(j).leftAngle(),sectorsSum[j]));
					}
					_curveIntervalsDatas->append(QwtPointPolar( _pieChart.sector(cursorMax).leftAngle(), 0. ));

					_curveHistogramIntervalsDatas << setOfIntervals;
					_curveHistogramIntervalsRealDatas.append(QwtInterval(cursorMin,cursorMax));
				}
				// Si les bornes définissent bien un intervalle... mais qu'il contient 0.
				else if ( cursorMax<cursorMin && qAbs(cursorMax-(cursorMin-nbSectors)) > 1 /*_minimumIntervalWidth*/ ) {
					_curveIntervalsDatas->append(QwtPointPolar( _pieChart.sector(cursorMin).rightAngle(), 0. ));
					for ( int j=cursorMin ; j<nbSectors ; ++j ) {
						_curveIntervalsDatas->append(QwtPointPolar(_pieChart.sector(j).rightAngle(),sectorsSum[j]));
						_curveIntervalsDatas->append(QwtPointPolar(_pieChart.sector(j).leftAngle(),sectorsSum[j]));
					}
					for ( int j=0 ; j<cursorMax ; ++j ) {
						_curveIntervalsDatas->append(QwtPointPolar(_pieChart.sector(j).rightAngle(),sectorsSum[j]));
						_curveIntervalsDatas->append(QwtPointPolar(_pieChart.sector(j).leftAngle(),sectorsSum[j]));
					}
					_curveIntervalsDatas->append(QwtPointPolar( _pieChart.sector(cursorMax>=0?cursorMax:nbSectors-1).leftAngle(), 0. ));

					_curveHistogramIntervalsDatas << setOfIntervals;
					_curveHistogramIntervalsRealDatas.append(QwtInterval(cursorMin,cursorMax));
				}
			}
		}
	}
	_curveHistogramIntervals.setSamples(_curveHistogramIntervalsDatas);
	_curveHistogramMaximums.setSamples(_curveHistogramMaximumsDatas);

	qDebug() << "Intervalles de branches :";
	for ( int i=0 ; i<_curveHistogramIntervalsRealDatas.size() ; ++i ) {
		const QwtInterval &interval = _curveHistogramIntervalsRealDatas[i];
		qDebug() << "  [ " << interval.minValue() << ", " << interval.maxValue() << " ] => [" << _pieChart.sector(interval.minValue()).rightAngle()*RAD_TO_DEG_FACT << ", " << _pieChart.sector(interval.maxValue()).leftAngle()*RAD_TO_DEG_FACT << "] avec largeur = " << interval.width();
	}
}

void PieChartDiagrams::draw( QImage &image, const iCoord2D &center ) const {
	if ( !_maximumsIndex.isEmpty() ) {
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
		for ( i=0 ; i<_curveHistogramIntervalsRealDatas.size() ; ++i ) {
			const QwtInterval &interval = _curveHistogramIntervalsRealDatas[i];
			twoSides.append( TWO_PI-_pieChart.sector(interval.minValue()).orientation()-0.01 );
			twoSides.append( TWO_PI-_pieChart.sector(interval.maxValue()).orientation() );
		}

		// Dessin des deux côtés du secteur
		QPainter painter(&image);
		i = twoSides.size()-1;
		while ( i > 0 ) {
			painter.setPen(colors[i%3]);
			painter.setBrush(colors[i%3]);
			nbBoucle = 2;
			while ( nbBoucle-- > 0 ) {
				// Calcul des coordonnées du segment à tracer
				angle = twoSides[i--];
				x1 = x2 = centerX;
				y1 = y2 = centerY;
				if ( qFuzzyCompare(angle,PI_ON_TWO) ) y2 = height;
				else if ( qFuzzyCompare(angle,THREE_PI_ON_TWO) ) y2 = 0;
				else {
					const qreal a = tan(angle);
					const qreal b = centerY - (a*centerX);
					if ( angle < PI_ON_TWO || angle > THREE_PI_ON_TWO ) {
						x2 = width;
						y2 = a*width+b;
					}
					else {
						x2 = 0;
						y2 = b;
					}
				}
				// Tracé du segment droit
				painter.drawLine(x1,y1,x2,y2);
//				if ( nbBoucle == 0 ) {
//					QPolygon polygon;
//					polygon.append(QPoint(x1,y1));
//					polygon.append(QPoint(x2,y2));
//					polygon.append(QPoint(oldx,oldy));
//					painter.drawPolygon(polygon);
//				}
//				else {
//					oldx = x2;
//					oldy = y2;
//				}
			}
		}
	}
}
