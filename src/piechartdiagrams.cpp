#include "inc/piechartdiagrams.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/piechart.h"
#include "inc/piepart.h"
#include "inc/slicesinterval.h"
#include "inc/intensityinterval.h"
#include "inc/pointpolarseriesdata.h"
#include <qwt_plot_histogram.h>

PieChartDiagrams::PieChartDiagrams() : _curveDatas(new PointPolarSeriesData()), _curveMaximumsDatas(new PointPolarSeriesData()), _highlightCurveDatas(new PointPolarSeriesData()), _movementsThresholdMin(100), _movementsThresholdMax(200), _marrowAroundDiameter(50) {
	_highlightCurve.setPen(QPen(Qt::red));
	_curveMaximums.setPen(QPen(Qt::green));
	_curveHistogramMaximums.setBrush(Qt::green);
	_curveHistogramMaximums.setPen(QPen(Qt::green));
	_curveHistogramIntervals.setBrush(Qt::blue);
	_curveHistogramIntervals.setPen(QPen(Qt::blue));
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
		_curveMaximums.attach(polarPlot);
		_highlightCurve.attach(polarPlot);
	}
}

void PieChartDiagrams::attach( QwtPlot * const plot ) {
	if ( plot != 0 ) {
		_curveHistogram.attach(plot);
		_curveHistogramIntervals.attach(plot);
		_curveHistogramMaximums.attach(plot);
	}
}

void PieChartDiagrams::detach() {
	_highlightCurve.detach();
	_curve.detach();
	_curveMaximums.detach();
	_curveHistogram.detach();
	_curveHistogramMaximums.detach();
	_curveHistogramIntervals.detach();
}

void PieChartDiagrams::clearAll() {
	_curveDatas->clear();
	_curveMaximumsDatas->clear();
	_highlightCurveDatas->clear();
	_highlightCurve.setData(_highlightCurveDatas);
	_curveHistogramDatas.clear();
	_curveHistogramMaximumsDatas.clear();
	_curveHistogramIntervalsDatas.clear();
	_curveHistogramIntervalsRealDatas.clear();
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

void PieChartDiagrams::compute( const Billon &billon, const Marrow *marrow, const PieChart &pieChart, const SlicesInterval &slicesInterval, const IntensityInterval &intensity ) {
	clearAll();
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
		QVector<int> sectorsSum(nbSectors,0);

		QList<int> circleLines;
		if ( marrow != 0 ) {
			circleLines.reserve(2*radius+1);
			for ( int lineIndex=-radius ; lineIndex<radiusMax ; ++lineIndex ) {
				circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
			}
		}

		int i, j, iRadius, iRadiusMax, currentSliceValue, previousSliceValue, sectorIdx;
		uint k, marrowX, marrowY, xPos, yPos;
		qreal diff;
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
		updateMaximums();
		computeIntervals();
	}
}

void PieChartDiagrams::highlightCurve( const int &index ) {
	_highlightCurveDatas->clear();
	if ( index > -1 && index < count() ) {
		const qreal coeff = 360./count();
		const qreal demiCoeff = coeff/2.;
		const qreal valueOfCurve = _curveDatas->sample(index*2).radius();

		_highlightCurveDatas->append(QwtPointPolar(index*coeff-demiCoeff,0));
		_highlightCurveDatas->append(QwtPointPolar(index*coeff-demiCoeff,valueOfCurve));
		_highlightCurveDatas->append(QwtPointPolar(index*coeff+demiCoeff,valueOfCurve));
		_highlightCurveDatas->append(QwtPointPolar(index*coeff+demiCoeff,0));
		_highlightCurveDatas->append(QwtPointPolar(index*coeff-demiCoeff,0));

	}
	_highlightCurve.setData(_highlightCurveDatas);
}

/*******************************
 * Private functions
 *******************************/

void PieChartDiagrams::createDiagrams( const QVector<int> &sectorsSum, const int &nbSectors ) {
	const qreal coeff = 360./nbSectors;
	const qreal demiCoeff = coeff/2.;
	qreal currentCoeff = -demiCoeff;
	int i, value;
	_curveDatas->clear();
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
	_curve.setData(_curveDatas);
	_curveHistogram.setSamples(_curveHistogramDatas);
}

void PieChartDiagrams::updateMaximums() {
	const int max = count();
	if ( max > 0 ) {
		double value;
		int cursor;
		bool isMax;

		_curveMaximumsDatas->clear();
		_curveHistogramMaximumsDatas.clear();
		qDebug() << "Pics angulaires :";
		for ( int i=0 ; i<max ; ++i ) {
			value = _curveHistogramDatas[i].value;
			if ( value > 0 ) {
				cursor = 1;
				do {
					isMax = ( (value > _curveHistogramDatas[i-cursor>=0?i-cursor:max+i-cursor].value) && (value > _curveHistogramDatas[i+cursor<max?i+cursor:i+cursor-max].value) );
					cursor++;
				}
				while ( isMax && cursor<3 );
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

	_curveMaximums.setData(_curveMaximumsDatas);
	_curveHistogramMaximums.setSamples(_curveHistogramMaximumsDatas);
}

int PieChartDiagrams::sliceOfIemeMaximum( const int &maximumIndex ) const {
	int sliceIndex = 0;
	const int &nbDatas = _curveHistogramDatas.size();
	if ( nbDatas != 0 && maximumIndex>-1 && maximumIndex<_curveHistogramMaximumsDatas.size() ) {
		sliceIndex = _curveHistogramMaximumsDatas.at(maximumIndex).interval.minValue()/360.*nbDatas;
	}
	return sliceIndex;
}


namespace {
	inline qreal firstdDerivated( const QVector< QwtIntervalSample > &histogramDatas, const int &index ) {
		return histogramDatas.at(index).value - histogramDatas.at(qAbs(index-1)).value;
	}
}

void PieChartDiagrams::computeIntervals() {
	_curveHistogramIntervalsDatas.clear();
	_curveHistogramIntervalsRealDatas.clear();
	int nbMaximums = _curveHistogramMaximumsDatas.size();
	if ( nbMaximums > 0 ) {
		const int sizeOfHistogram = count();
		int cursorMax, cursorMin;
		qreal derivated;
		QVector<QwtIntervalSample> setOfIntervals;
//		if ( _intervalType != HistogramIntervalType::FROM_EDGE ) {
//			qreal limit = _intervalType==HistogramIntervalType::FROM_MEANS?_dataMeans:_intervalType==HistogramIntervalType::FROM_MEDIAN?_dataMedian:_dataMeansMedian;
//			for ( int i=0 ; i<nbMaximums ; ++i ) {
//				cursorMin = sliceOfIemeMaximum(i);
//				if (_curveHistogramIntervalsRealDatas.size() == 0 || _curveHistogramIntervalsRealDatas.last().maxValue() < cursorMin ) {
//					setOfIntervals.clear();
//					derivated = firstdDerivated(_datasHistogram,cursorMin);
//					while ( cursorMin > 0 && (_datasHistogram.at(cursorMin).value > limit || derivated > 0.) ) {
//						setOfIntervals.append(_datasHistogram.at(cursorMin));
//						cursorMin--;
//						if ( cursorMin > 0 ) derivated = firstdDerivated(_datasHistogram,cursorMin);
//					}
//					cursorMin++;

//					if (_curveHistogramIntervalsRealDatas.size() == 0 || _curveHistogramIntervalsRealDatas.last().minValue() != cursorMin ) {
//						cursorMax = sliceOfIemeMaximum(i)+1;
//						derivated = firstdDerivated(_datasHistogram,cursorMax);
//						while ( cursorMax < sizeOfHistogram && (_datasHistogram.at(cursorMax).value > limit || derivated < 0.) ) {
//							setOfIntervals.append(_datasHistogram.at(cursorMax));
//							cursorMax++;
//							if ( cursorMax < sizeOfHistogram ) derivated = firstdDerivated(_datasHistogram,cursorMax);
//						}
//						cursorMax--;

//						if ( (cursorMax-cursorMin) > _minimumIntervalWidth ) {
//							_curveHistogramIntervalsDatas << setOfIntervals;
//							_curveHistogramIntervalsRealDatas.append(QwtInterval(cursorMin,cursorMax));
//						}
//						else {
//							_datasMaximums.remove(i);
//							nbMaximums--;
//							i--;
//						}
//					}
//				}
//			}
//		}
//		else {
			const qreal factor = count()/360.;
			const qreal invertedFactor = 360./static_cast<qreal>(count());
			for ( int i=0 ; i<nbMaximums ; ++i ) {
				cursorMin = sliceOfIemeMaximum(i);
				if (_curveHistogramIntervalsRealDatas.size() == 0 || _curveHistogramIntervalsRealDatas.last().maxValue()*factor < cursorMin ) {
					setOfIntervals.clear();
					derivated = firstdDerivated(_curveHistogramDatas,cursorMin);
					while ( derivated > 0. ) {
						setOfIntervals.append(_curveHistogramDatas.at(cursorMin));
						cursorMin--;
						if ( cursorMin < 0 ) cursorMin = sizeOfHistogram-1;
						derivated = firstdDerivated(_curveHistogramDatas,cursorMin);
					}

					if (_curveHistogramIntervalsRealDatas.size() == 0 || static_cast<int>(_curveHistogramIntervalsRealDatas.last().minValue()*factor) != cursorMin ) {
						cursorMax = sliceOfIemeMaximum(i)+1;
						derivated = firstdDerivated(_curveHistogramDatas,cursorMax);
						while ( derivated < 0. ) {
							setOfIntervals.append(_curveHistogramDatas.at(cursorMax));
							cursorMax++;
							if ( cursorMax == sizeOfHistogram ) cursorMax = 0;
							derivated = firstdDerivated(_curveHistogramDatas,cursorMax);
						}

						if ( qAbs(cursorMax-cursorMin) > 1 /*_minimumIntervalWidth*/ ) {
								_curveHistogramIntervalsDatas << setOfIntervals;
								_curveHistogramIntervalsRealDatas.append(QwtInterval(cursorMin*invertedFactor,cursorMax*invertedFactor));
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
//	}
	_curveHistogramIntervals.setSamples(_curveHistogramIntervalsDatas);
	_curveHistogramMaximums.setSamples(_curveHistogramMaximumsDatas);

	qDebug() << "Intervalles de branches :";
	for ( int i=0 ; i<_curveHistogramIntervalsRealDatas.size() ; ++i ) {
		const QwtInterval &interval = _curveHistogramIntervalsRealDatas.at(i);
		qDebug() << "  [ " << interval.minValue() << ", " << interval.maxValue() << " ] avec largeur = " << interval.width();
	}
}
