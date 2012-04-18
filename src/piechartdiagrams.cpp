#include "inc/piechartdiagrams.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/piechart.h"
#include "inc/piepart.h"
#include "inc/slicesinterval.h"
#include "inc/intensityinterval.h"
#include "inc/pointpolarseriesdata.h"

#include <qwt_plot_histogram.h>

#include <QPainter>

PieChartDiagrams::PieChartDiagrams() : _curveDatas(new PointPolarSeriesData()), _curveMaximumsDatas(new PointPolarSeriesData()), _highlightCurveDatas(new PointPolarSeriesData()), _curveIntervalsDatas(new PointPolarSeriesData()),
	_movementsThresholdMin(100), _movementsThresholdMax(200), _marrowAroundDiameter(50), _intervalType(HistogramIntervalType::FROM_MIDDLE_OF_MEANS_AND_MEDIAN)
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

void PieChartDiagrams::setMovementsThresholdMin( const int &threshold ) {
	_movementsThresholdMin = threshold;
}

void PieChartDiagrams::setMovementsThresholdMax( const int &threshold ) {
	_movementsThresholdMax = threshold;
}

void PieChartDiagrams::useNextSliceInsteadOfCurrentSlice( const bool &enable ) {
	_useNextSlice = enable;
}

void PieChartDiagrams::setMarrowArroundDiameter( const int &diameter ) {
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

void PieChartDiagrams::compute( const Billon &billon, const Marrow *marrow, const PieChart &pieChart, const SlicesInterval &slicesInterval, const IntensityInterval &intensity ) {
	if ( slicesInterval.isValid() ) {
		const uint width = billon.n_cols;
		const uint height = billon.n_rows;
		const int minValue = intensity.min();
		const int maxValue = intensity.max();
		const int nbSectors = pieChart.nbSectors();
		const uint minOfInterval = slicesInterval.min();
		const uint maxOfInterval = slicesInterval.max();
		const int diameter = _marrowAroundDiameter;
		const int radius = diameter/(2.*billon.voxelWidth());
		const int radiusMax = radius+1;
		const qreal squareRadius = qPow(radius,2);

		// Calcul des diagrammes en parcourant les tranches du billon comprises dans l'intervalle
		QVector<__billon_type__> sectorsSum(nbSectors,0);

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
							sectorIdx = pieChart.partOfAngle( TWO_PI-ANGLE(marrowX,marrowY,xPos,yPos) );
							currentSliceValue = slice.at(yPos,xPos);
							previousSliceValue = prevSlice.at(yPos,xPos);
							if ( (currentSliceValue > minValue) && (previousSliceValue > minValue) ) {
								diff = qAbs(qBound(minValue,currentSliceValue,maxValue) - qBound(minValue,previousSliceValue,maxValue));
								if ( (diff > _movementsThresholdMin) && (diff < _movementsThresholdMax) ) {
									diff -= _movementsThresholdMin;
									sectorsSum[sectorIdx] += diff;
								}
							}
						}
					}
				}
			}
			else {
				for ( j=0 ; j<static_cast<int>(height) ; ++j ) {
					for ( i=0 ; i<static_cast<int>(width) ; ++i ) {
						sectorIdx = pieChart.partOfAngle( TWO_PI-ANGLE(marrowX,marrowY,i,j) );
						currentSliceValue = slice.at(j,i);
						previousSliceValue = prevSlice.at(j,i);
						if ( (currentSliceValue > minValue) && (previousSliceValue > minValue) ) {
							diff = qAbs(qBound(minValue,currentSliceValue,maxValue) - qBound(minValue,previousSliceValue,maxValue));
							if ( diff > _movementsThresholdMin && diff < _movementsThresholdMax ) {
								diff -= _movementsThresholdMin;
								sectorsSum[sectorIdx] += diff;
							}
						}
					}
				}
			}
		}

		createDiagrams( sectorsSum, nbSectors );
		computeMeansAndMedian();
		computeMaximums();
		computeIntervals();
	}
}

void PieChartDiagrams::highlightCurve( const int &index ) {
	_highlightCurveDatas->clear();
	_highlightCurveHistogramDatas.clear();
	if ( index > -1 && index < count() ) {
		const qreal coeff = 360./count();
		const qreal angleStart = coeff*index-(coeff/2.);
		const qreal angleEnd = coeff*index+(coeff/2.);
		const qreal valueOfCurve = _curveHistogramDatas[index].value;

		_highlightCurveDatas->append(QwtPointPolar(angleStart,0.));
		_highlightCurveDatas->append(QwtPointPolar(angleStart,valueOfCurve));
		_highlightCurveDatas->append(QwtPointPolar(angleEnd,valueOfCurve));
		_highlightCurveDatas->append(QwtPointPolar(angleEnd,0.));

		_highlightCurveHistogramDatas.append(QwtIntervalSample(valueOfCurve,angleStart,angleEnd));
	}
	_highlightCurveHistogram.setSamples(_highlightCurveHistogramDatas);
}

/*******************************
 * Private functions
 *******************************/
int PieChartDiagrams::sliceOfIemeMaximum( const int &maximumIndex ) const {
	int sliceIndex = 0;
	const int &nbDatas = _curveHistogramDatas.size();
	const qreal factor = nbDatas/360.;
	if ( nbDatas != 0 && maximumIndex>-1 && maximumIndex<_curveHistogramMaximumsDatas.size() ) {
		sliceIndex = _curveHistogramMaximumsDatas.at(maximumIndex).interval.minValue()*factor+1;
	}
	return sliceIndex;
}

void PieChartDiagrams::createDiagrams( const QVector<int> &sectorsSum, const int &nbSectors ) {
	const qreal coeff = 360./nbSectors;
	const qreal demiCoeff = coeff/2.;
	qreal currentCoeff = -demiCoeff;
	int i, value;
	_curveDatas->clear();
	_curveHistogramDatas.clear();
	_curveHistogramDatas.fill(QwtIntervalSample(),nbSectors);
	for ( i=0 ; i<nbSectors ; ++i ) {
		value = sectorsSum[i];
		QwtIntervalSample &interval = _curveHistogramDatas[i];

		_curveDatas->append(QwtPointPolar(currentCoeff,value));
		interval.interval.setMinValue(currentCoeff);

		currentCoeff+=coeff;

		_curveDatas->append(QwtPointPolar(currentCoeff,value));
		interval.interval.setMaxValue(currentCoeff);
		interval.value = sectorsSum[i];
	}
	_curveDatas->append(QwtPointPolar(-demiCoeff,sectorsSum[0]));
	_curveHistogram.setSamples(_curveHistogramDatas);
}

void PieChartDiagrams::computeMaximums() {
	const int nbDatas = count();
	if ( nbDatas > 0 ) {
		double value;
		int i, cursor;
		bool isMax;
		_curveMaximumsDatas->clear();
		_curveHistogramMaximumsDatas.clear();
		qDebug() << "Pics angulaires :";
		for ( i=0 ; i<nbDatas ; ++i ) {
			value = _curveHistogramDatas[i].value;
			if ( value > _dataMeans ) {
				cursor = 1;
				do {
					isMax = ( (value > _curveHistogramDatas[i-cursor>=0?i-cursor:nbDatas+i-cursor].value) && (value > _curveHistogramDatas[i+cursor<nbDatas?i+cursor:i+cursor-nbDatas].value) );
					cursor++;
				}
				while ( isMax && cursor<1 );
				if ( isMax ) {
					_curveMaximumsDatas->append(QwtPointPolar(_curveDatas->sample(2*i).azimuth(),0.));
					_curveMaximumsDatas->append(_curve.sample(2*i));
					_curveMaximumsDatas->append(_curve.sample(2*i+1));
					_curveMaximumsDatas->append(QwtPointPolar(_curveDatas->sample(2*i+1).azimuth(),0.));
					_curveHistogramMaximumsDatas.append(_curveHistogramDatas[i]);
					i+=cursor-1;
					qDebug() << i;
				}
			}
		}
	}
	_curveHistogramMaximums.setSamples(_curveHistogramMaximumsDatas);
}

void PieChartDiagrams::computeMeansAndMedian() {
	const int nbDatas = count();
	qreal xMeans[2] = { 0., 360. };
	qreal yMeans[2] = { 0., 0. };
	qreal xMedian[2] = { 0., 360. };
	qreal yMedian[2] = { 0., 0. };
	qreal xMeansMedian[2] = { 0., 360. };
	qreal yMeansMedian[2] = { 0., 0. };
	qreal currentValue;
	_dataMeans = 0.;
	_dataMedian = 0.;
	_dataMeansMedian = 0.;
	if ( nbDatas > 0 ) {
		QVector<qreal> listToSort(nbDatas);
		for ( int i=0 ; i<nbDatas ; ++i ) {
			currentValue = _curveHistogramDatas.at(i).value;
			_dataMeans += currentValue;
			listToSort[i] = currentValue;
		}
		_dataMeans /= static_cast<qreal>(nbDatas);
		qSort(listToSort);
		if ( nbDatas % 2 == 0 ) _dataMedian = (listToSort.at((nbDatas/2)-1)+listToSort.at(nbDatas/2))/2.;
		else _dataMedian = listToSort.at((nbDatas+1)/2-1);
		_dataMeansMedian = (_dataMeans+_dataMedian)/2.;

		yMeans[0] = yMeans[1] = _dataMeans;
		yMedian[0] = yMedian[1] = _dataMedian;
		yMeansMedian[0] = yMeansMedian[1] = _dataMeansMedian;
	}
	_curveMeans.setSamples(xMeans,yMeans,2);
	_curveMedian.setSamples(xMedian,yMedian,2);
	_curveMeansMedian.setSamples(xMeansMedian,yMeansMedian,2);
}

namespace {
	inline qreal firstdDerivated( const QVector< QwtIntervalSample > &histogramDatas, const int &index ) {
		return histogramDatas.at(index<histogramDatas.size()?index:0).value - histogramDatas.at(index>0?index-1:histogramDatas.size()-1).value;
	}
}

void PieChartDiagrams::computeIntervals() {
	_curveIntervalsDatas->clear();
	_curveHistogramIntervalsDatas.clear();
	_curveHistogramIntervalsRealDatas.clear();
	int nbMaximums = _curveHistogramMaximumsDatas.size();
	if ( nbMaximums > 0 ) {
		const int sizeOfHistogram = count();
		const qreal coeffIndiceToDeg = 360./static_cast<qreal>(sizeOfHistogram);
		const qreal coeffDegToIndice = static_cast<qreal>(sizeOfHistogram)/360.;
		int cursorMax, cursorMin;
		qreal derivated;
		QVector<QwtIntervalSample> setOfIntervals;
		if ( _intervalType != HistogramIntervalType::FROM_EDGE ) {
			qreal limit = _intervalType==HistogramIntervalType::FROM_MEANS?_dataMeans:_intervalType==HistogramIntervalType::FROM_MEDIAN?_dataMedian:_dataMeansMedian;
			for ( int i=0 ; i<nbMaximums ; ++i ) {
				cursorMin = sliceOfIemeMaximum(i);
				if (_curveHistogramIntervalsRealDatas.size() == 0 ||
					(_curveHistogramIntervalsRealDatas.last().isValid() && static_cast<int>(_curveHistogramIntervalsRealDatas.last().maxValue()*coeffDegToIndice) < cursorMin) ||
					(!_curveHistogramIntervalsRealDatas.last().isValid() && static_cast<int>(_curveHistogramIntervalsRealDatas.last().minValue()*coeffDegToIndice) > cursorMin)) {
					setOfIntervals.clear();
					derivated = firstdDerivated(_curveHistogramDatas,cursorMin);
					while ( _curveHistogramDatas.at(cursorMin).value > limit || derivated > 0. ) {
						setOfIntervals.append(_curveHistogramDatas.at(cursorMin));
						cursorMin--;
						if ( cursorMin < 0 ) cursorMin = sizeOfHistogram-1;
						derivated = firstdDerivated(_curveHistogramDatas,cursorMin);
					}

					if (_curveHistogramIntervalsRealDatas.size() == 0 || _curveHistogramIntervalsRealDatas.last().minValue()*coeffDegToIndice != cursorMin ) {
						cursorMax = sliceOfIemeMaximum(i)+1;
						if ( cursorMax == sizeOfHistogram ) cursorMax = 0;
						derivated = firstdDerivated(_curveHistogramDatas,cursorMax);
						while ( _curveHistogramDatas.at(cursorMax).value > limit || derivated < 0. ) {
							setOfIntervals.append(_curveHistogramDatas.at(cursorMax));
							cursorMax++;
							if ( cursorMax == sizeOfHistogram ) cursorMax = 0;
							derivated = firstdDerivated(_curveHistogramDatas,cursorMax);
						}
						cursorMax--;

						if ( qAbs(cursorMax-cursorMin) > 1 /*_minimumIntervalWidth*/ ) {
							if ( cursorMax>cursorMin ) {
								QwtPointPolar pointInf = _curveDatas->sample(cursorMin*2);
								pointInf.setRadius(0.);
								_curveIntervalsDatas->append(pointInf);
								for ( int j=cursorMin ; j<cursorMax ; ++j ) {
									_curveIntervalsDatas->append(_curveDatas->sample(j*2));
									_curveIntervalsDatas->append(_curveDatas->sample(j*2+1));
								}
								QwtPointPolar pointSup = _curveDatas->sample(cursorMax*2-1);
								pointSup.setRadius(0.);
								_curveIntervalsDatas->append(pointSup);
							}
							else {
								QwtPointPolar pointInf = _curveDatas->sample(cursorMin*2);
								pointInf.setRadius(0.);
								_curveIntervalsDatas->append(pointInf);
								for ( int j=cursorMin ; j<sizeOfHistogram ; ++j ) {
									_curveIntervalsDatas->append(_curveDatas->sample(j*2));
									_curveIntervalsDatas->append(_curveDatas->sample(j*2+1));
								}
								for ( int j=0 ; j<cursorMax ; ++j ) {
									_curveIntervalsDatas->append(_curveDatas->sample(j*2));
									_curveIntervalsDatas->append(_curveDatas->sample(j*2+1));
								}
								QwtPointPolar pointSup = cursorMax>0?_curveDatas->sample(cursorMax*2-1):_curveDatas->sample(sizeOfHistogram-1);
								pointSup.setRadius(0.);
								_curveIntervalsDatas->append(pointSup);

							}
							_curveHistogramIntervalsDatas << setOfIntervals;
							_curveHistogramIntervalsRealDatas.append(QwtInterval(cursorMin*coeffIndiceToDeg,cursorMax*coeffIndiceToDeg));
						}
						else {
							_curveHistogramMaximumsDatas.remove(i);
							nbMaximums--;
							i--;
						}
					}
				}
			}
		}
		else {
			for ( int i=0 ; i<nbMaximums ; ++i ) {
				cursorMin = sliceOfIemeMaximum(i);
				setOfIntervals.clear();
				derivated = firstdDerivated(_curveHistogramDatas,cursorMin);
				while ( derivated > 0. ) {
					setOfIntervals.append(_curveHistogramDatas.at(cursorMin));
					cursorMin--;
					if ( cursorMin < 0 ) cursorMin = sizeOfHistogram-1;
					derivated = firstdDerivated(_curveHistogramDatas,cursorMin);
				}
				cursorMax = sliceOfIemeMaximum(i)+1;
				if ( cursorMax == sizeOfHistogram ) cursorMax = 0;
				derivated = firstdDerivated(_curveHistogramDatas,cursorMax);
				while ( derivated < 0. ) {
					setOfIntervals.append(_curveHistogramDatas.at(cursorMax));
					cursorMax++;
					if ( cursorMax == sizeOfHistogram ) cursorMax = 0;
					derivated = firstdDerivated(_curveHistogramDatas,cursorMax);
				}
				cursorMax--;

				if ( qAbs(cursorMax-cursorMin) > 1 /*_minimumIntervalWidth*/ ) {
					if ( cursorMax>cursorMin ) {
						QwtPointPolar pointInf = _curveDatas->sample(cursorMin*2);
						pointInf.setRadius(0.);
						_curveIntervalsDatas->append(pointInf);
						for ( int j=cursorMin ; j<cursorMax ; ++j ) {
							_curveIntervalsDatas->append(_curveDatas->sample(j*2));
							_curveIntervalsDatas->append(_curveDatas->sample(j*2+1));
						}
						QwtPointPolar pointSup = _curveDatas->sample(cursorMax*2-1);
						pointSup.setRadius(0.);
						_curveIntervalsDatas->append(pointSup);
					}
					else {
						QwtPointPolar pointInf = _curveDatas->sample(cursorMin*2);
						pointInf.setRadius(0.);
						_curveIntervalsDatas->append(pointInf);
						for ( int j=cursorMin ; j<sizeOfHistogram ; ++j ) {
							_curveIntervalsDatas->append(_curveDatas->sample(j*2));
							_curveIntervalsDatas->append(_curveDatas->sample(j*2+1));
						}
						for ( int j=0 ; j<cursorMax ; ++j ) {
							_curveIntervalsDatas->append(_curveDatas->sample(j*2));
							_curveIntervalsDatas->append(_curveDatas->sample(j*2+1));
						}
						QwtPointPolar pointSup = cursorMax>0?_curveDatas->sample(cursorMax*2-1):_curveDatas->sample(sizeOfHistogram-1);
						pointSup.setRadius(0.);
						_curveIntervalsDatas->append(pointSup);

					}
					_curveHistogramIntervalsDatas << setOfIntervals;
					_curveHistogramIntervalsRealDatas.append(QwtInterval(cursorMin*coeffIndiceToDeg,cursorMax*coeffIndiceToDeg));
				}
				else {
					_curveHistogramMaximumsDatas.remove(i);
					nbMaximums--;
					i--;
				}
			}
		}
	}
	_curveHistogramIntervals.setSamples(_curveHistogramIntervalsDatas);
	_curveHistogramMaximums.setSamples(_curveHistogramMaximumsDatas);

	qDebug() << "Intervalles de branches :";
	for ( int i=0 ; i<_curveHistogramIntervalsRealDatas.size() ; ++i ) {
		const QwtInterval &interval = _curveHistogramIntervalsRealDatas.at(i);
		qDebug() << "  [ " << interval.minValue() << ", " << interval.maxValue() << " ] avec largeur = " << interval.width();
	}
}

void PieChartDiagrams::draw( QImage &image, const iCoord2D &center ) const {
	if ( _curveMaximumsDatas->size() > 0 ) {
		const int width = image.width();
		const int height = image.height();
		const int centerX = center.x;
		const int centerY = center.y;

		// Liste qui va contenir les angles des deux côté du secteur à dessiner
		// Permet de factoriser le code de calcul des coordonnées juste en dessous
		QList<qreal> twoSides;
		for ( int i=0 ; i<_curveHistogramIntervalsRealDatas.size() ; ++i ) {
			twoSides.append( TWO_PI-(_curveHistogramIntervalsRealDatas[i].minValue()+1)*DEG_TO_RAD_FACT );
			twoSides.append( TWO_PI-(_curveHistogramIntervalsRealDatas[i].maxValue()-1)*DEG_TO_RAD_FACT );
		}

		QPainter painter(&image);
		QColor colors[] = { Qt::blue, Qt::cyan, Qt::magenta };

		// Dessin des deux côtés du secteur
		qreal angle, x1,y1,x2,y2;
		while ( !twoSides.isEmpty() ) {
			painter.setPen(colors[twoSides.size()%3]);
			int two = 2;
			while (two--) {
				// Calcul des coordonnées du segment à tracer
				angle = twoSides.takeLast();
				x1 = x2 = centerX;
				y1 = y2 = centerY;
				if ( qFuzzyCompare(angle,PI_ON_TWO) ) y2 = height;
				else if ( qFuzzyCompare(angle,THREE_PI_ON_TWO) ) y1 = 0;
				else {
					const qreal a = tan(angle);
					const qreal b = centerY - (a*centerX);
					if ( angle < PI_ON_TWO || angle > THREE_PI_ON_TWO ) {
						x2 = width;
						y2 = a*width+b;
					}
					else {
						x1 = 0;
						y1 = b;
					}
				}
				// Tracé du segment droit
				painter.drawLine(x1,y1,x2,y2);
			}
		}
	}
}
