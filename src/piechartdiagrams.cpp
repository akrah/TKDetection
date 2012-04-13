#include "inc/piechartdiagrams.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/piechart.h"
#include "inc/piepart.h"
#include "inc/slicesinterval.h"
#include "inc/intensityinterval.h"
#include "inc/pointpolarseriesdata.h"
#include <qwt_plot_histogram.h>

PieChartDiagrams::PieChartDiagrams() : _polarCurve(0), _movementsThresholdMin(100), _movementsThresholdMax(200), _marrowAroundDiameter(50) {
	_highlightCurve.setPen(QPen(Qt::green));
}

PieChartDiagrams::~PieChartDiagrams() {
	clearAll();
}

/*******************************
 * Public getters
 *******************************/

int PieChartDiagrams::count() const {
	return _histograms.size();
}

/*******************************
 * Public setters
 *******************************/

void PieChartDiagrams::attach( const QList<QwtPlot *> & plots ) {
	const int nbMaxIter = qMin(plots.size(),_histograms.size());
	for ( int i=0 ; i<nbMaxIter ; ++i ) {
		if ( _histograms[i] != 0 && plots[i] != 0 ) {
			_histograms[i]->attach(plots[i]);
		}
	}
}

void PieChartDiagrams::attach( QwtPolarPlot * const polarPlot ) {
	if ( _polarCurve != 0 && polarPlot != 0 ) {
		_polarCurve->attach(polarPlot);
		_highlightCurve.attach(polarPlot);
	}
}

void PieChartDiagrams::detach() {
	_highlightCurve.detach();
	if ( _polarCurve != 0 ) _polarCurve->detach();
	for ( int i=0 ; i<_histograms.size() ; ++i ) {
		if ( _histograms[i] != 0 ) _histograms[i]->detach();
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

void PieChartDiagrams::compute( const Billon &billon, const Marrow *marrow, const PieChart &pieChart, const SlicesInterval &slicesInterval, const IntensityInterval &intensity ) {
	if ( slicesInterval.isValid() ) {
		const uint width = billon.n_cols;
		const uint height = billon.n_rows;
		const int minValue = intensity.min();
		const int maxValue = intensity.max();
		const int nbValues = _movementsThresholdMax-_movementsThresholdMin;
		const int nbSectors = pieChart.nbSectors();
		const uint minOfInterval = slicesInterval.min();
		const uint maxOfInterval = slicesInterval.max();
		const int diameter = _marrowAroundDiameter;
		const int radius = diameter/(2.*billon.voxelWidth());
		const int radiusMax = radius+1;
		const qreal squareRadius = qPow(radius,2);

		// Création des données des diagrammes
		QVector< QVector<QwtIntervalSample> > histogramsData(nbSectors,QVector<QwtIntervalSample>(nbValues,QwtIntervalSample(0.,0.,0.)));
		initializeDiagramsData( histogramsData, nbSectors, nbValues );

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
									histogramsData[sectorIdx][diff-1].value += 1.;
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
								histogramsData[sectorIdx][diff-1].value += 1.;
								sectorsSum[sectorIdx] += diff;
							}
						}
					}
				}
			}
		}

		createDiagrams( histogramsData, sectorsSum, nbSectors );
	}
}

void PieChartDiagrams::highlightCurve( const int &index ) {
	if ( !_histograms.isEmpty() && index > -1 && index < _histograms.size() ) {
		const qreal coeff = 360./_histograms.size();
		const qreal demiCoeff = coeff/2.;
		const qreal valueOfCurve = _polarCurve->sample(index*2).radius();

		PointPolarSeriesData *pointsDatas = new PointPolarSeriesData();
		pointsDatas->append(QwtPointPolar(index*coeff-demiCoeff,0));
		pointsDatas->append(QwtPointPolar(index*coeff-demiCoeff,valueOfCurve));
		pointsDatas->append(QwtPointPolar(index*coeff+demiCoeff,valueOfCurve));
		pointsDatas->append(QwtPointPolar(index*coeff+demiCoeff,0));
		pointsDatas->append(QwtPointPolar(index*coeff-demiCoeff,0));

		_highlightCurve.setData(pointsDatas);
	}
}

/*******************************
 * Private functions
 *******************************/

void PieChartDiagrams::clearAll() {
	qDeleteAll(_histograms);
	_histograms.clear();
	if ( _polarCurve != 0 ) {
		delete _polarCurve;
		_polarCurve = 0;
	}
	_highlightCurve.setData(new PointPolarSeriesData());
}

void PieChartDiagrams::initializeDiagramsData( QVector< QVector<QwtIntervalSample> > &histogramsData, const int &nbSectors, const int &nbValues ) {
	for ( int i=0 ; i<nbSectors ; ++i ) {
		for ( int j=0 ; j<nbValues ; ++j ) {
			histogramsData[i][j].interval.setInterval(j,j+1);
		}
	}
}

void PieChartDiagrams::createDiagrams( QVector< QVector<QwtIntervalSample> > &histogramsData, const QVector<int> &sectorsSum, const int &nbSectors ) {
	clearAll();

	int i;
	// Création des objets histogrammes et affectation de leurs données correspondantes
	_histograms.reserve(nbSectors);
	for ( i=0 ; i<nbSectors ; ++i ) {
		_histograms.append(new QwtPlotHistogram());
		static_cast<QwtIntervalSeriesData *>(_histograms.last()->data())->setSamples(histogramsData[i]);
	}

	// Création du diagramme circulaire
	_polarCurve = new QwtPolarCurve();
	PointPolarSeriesData *pointsDatas = new PointPolarSeriesData();
	const qreal coeff = 360./nbSectors;
	const qreal demiCoeff = coeff/2.;
	qreal currentCoeff = -demiCoeff;
	for ( i=0 ; i<nbSectors ; ++i ) {
		pointsDatas->append(QwtPointPolar(currentCoeff,sectorsSum[i]));
		currentCoeff+=coeff;
		pointsDatas->append(QwtPointPolar(currentCoeff,sectorsSum[i]));
	}
	pointsDatas->append(QwtPointPolar(-demiCoeff,sectorsSum[0]));
	_polarCurve->setData(pointsDatas);
}
