#include "inc/piechartdiagrams.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/piechart.h"
#include "inc/piepart.h"
#include "inc/slicesinterval.h"
#include "inc/intensityinterval.h"
#include "inc/pointpolarseriesdata.h"
#include <qwt_plot_histogram.h>

PieChartDiagrams::PieChartDiagrams() :  _curve(new QwtPolarCurve()), _curveMaximums(new QwtPolarCurve()), _highlightCurve(new QwtPolarCurve()), _curveHistogram(new QwtPlotHistogram()), _curveHistogramMaximums(new QwtPlotHistogram()), _movementsThresholdMin(100), _movementsThresholdMax(200), _marrowAroundDiameter(50) {
	_highlightCurve->setPen(QPen(Qt::green));
	_curveHistogramMaximums->setBrush(Qt::green);
	_curveHistogramMaximums->setPen(QPen(Qt::green));
}

PieChartDiagrams::~PieChartDiagrams() {
	clearAll();
	if ( _curve != 0 ) delete _curve;
	if ( _curveMaximums != 0 ) delete _curveMaximums;
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
		_curve->attach(polarPlot);
		_curveMaximums->attach(polarPlot);
		_highlightCurve->attach(polarPlot);
	}
}

void PieChartDiagrams::attach( QwtPlot * const plot ) {
	if ( plot != 0 ) {
		_curveHistogram->attach(plot);
		_curveHistogramMaximums->attach(plot);
	}
}

void PieChartDiagrams::detach() {
	_highlightCurve->detach();
	_curve->detach();
	_curveMaximums->detach();
	_curveHistogram->detach();
	_curveHistogramMaximums->detach();
}

void PieChartDiagrams::clearAll() {
	if ( _curve != 0 ) {
		delete _curve;
		_curve = new QwtPolarCurve();
	}

	if ( _curveMaximums != 0 ) {
		delete _curveMaximums;
		_curveMaximums = new QwtPolarCurve();
	}

	_curveHistogramDatas.clear();
	_curveHistogram->setSamples(_curveHistogramDatas);

	_curveHistogramMaximumsDatas.clear();
	_curveHistogramMaximums->setSamples(_curveHistogramMaximumsDatas);
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
	}
}

void PieChartDiagrams::highlightCurve( const int &index ) {
	if ( index > -1 && index < count() ) {
		const qreal coeff = 360./count();
		const qreal demiCoeff = coeff/2.;
		const qreal valueOfCurve = _curve->sample(index*2).radius();

		PointPolarSeriesData *pointsDatas = new PointPolarSeriesData();
		pointsDatas->append(QwtPointPolar(index*coeff-demiCoeff,0));
		pointsDatas->append(QwtPointPolar(index*coeff-demiCoeff,valueOfCurve));
		pointsDatas->append(QwtPointPolar(index*coeff+demiCoeff,valueOfCurve));
		pointsDatas->append(QwtPointPolar(index*coeff+demiCoeff,0));
		pointsDatas->append(QwtPointPolar(index*coeff-demiCoeff,0));

		_highlightCurve->setData(pointsDatas);
	}
}

/*******************************
 * Private functions
 *******************************/

void PieChartDiagrams::createDiagrams( const QVector<int> &sectorsSum, const int &nbSectors ) {
	const qreal coeff = 360./nbSectors;
	const qreal demiCoeff = coeff/2.;
	qreal currentCoeff = -demiCoeff;
	int i, value;
	PointPolarSeriesData *curveDatas = new PointPolarSeriesData();
	_curveHistogramDatas.fill(QwtIntervalSample(),nbSectors);
	for ( i=0 ; i<nbSectors ; ++i ) {
		value = sectorsSum[i];
		QwtIntervalSample &interval = _curveHistogramDatas[i];

		curveDatas->append(QwtPointPolar(currentCoeff,value));
		interval.interval.setMinValue(currentCoeff);

		currentCoeff+=coeff;

		curveDatas->append(QwtPointPolar(currentCoeff,value));
		interval.interval.setMaxValue(currentCoeff);
		interval.value = sectorsSum[i];
	}
	curveDatas->append(QwtPointPolar(-demiCoeff,sectorsSum[0]));
	_curve->setData(curveDatas);
	_curveHistogram->setSamples(_curveHistogramDatas);
}

void PieChartDiagrams::updateMaximums() {
	const int max = count();
	PointPolarSeriesData *_curveMaximumsDatas = new PointPolarSeriesData();

	if ( max > 0 ) {
		double value;
		int cursor;
		bool isMax;

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
					_curveMaximumsDatas->append(_curve->sample(i));
					_curveHistogramMaximumsDatas.append(_curveHistogramDatas[i]);
					i+=cursor-1;
					qDebug() << i;
				}
			}
		}
	}

	_curveMaximums->setData(_curveMaximumsDatas);
	_curveHistogramMaximums->setSamples(_curveHistogramMaximumsDatas);
}
