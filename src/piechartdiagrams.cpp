#include "inc/piechartdiagrams.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/piechart.h"
#include "inc/piepart.h"
#include "inc/slicesinterval.h"
#include "inc/pointpolarseriesdata.h"
#include <qwt_plot_histogram.h>
#include <qwt_polar_curve.h>

PieChartDiagrams::PieChartDiagrams() : _polarCurve(0), _minimalDifference(0) {
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

int PieChartDiagrams::minimalDifference() const {
	return _minimalDifference;
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
	}
}

void PieChartDiagrams::detach() {
	if ( _polarCurve != 0 ) _polarCurve->detach();
	for ( int i=0 ; i<_histograms.size() ; ++i ) {
		if ( _histograms[i] != 0 ) _histograms[i]->detach();
	}
}

void PieChartDiagrams::setMinimalDifference( const int &minimalDifference ) {
	_minimalDifference = minimalDifference;
}

void PieChartDiagrams::compute( const Billon &billon, const Marrow *marrow, const PieChart &pieChart, const SlicesInterval &slicesInterval ) {
	clearAll();
	if ( slicesInterval.isValid() ) {
		const int nbSectors = pieChart.nbSectors();
		const int minValue = billon.minValue();
		const int nbValues = billon.maxValue()-minValue;
		const int sliceWidth = billon.n_cols;
		const int sliceHeight = billon.n_rows;
		const int minOfInterval = slicesInterval.min();
		const int maxOfInterval = slicesInterval.max();
		int i, j, k, centerX, centerY, sectorIdx, diffVal, diffMax, currentSliceValue, previousSliceValue;

		// Création des données des diagrammes
		QVector< QVector<QwtIntervalSample> > histogramsData(nbSectors,QVector<QwtIntervalSample>(nbValues,QwtIntervalSample(0.,0.,0.)));
		initializeDiagramsData( histogramsData, nbSectors, nbValues );

		// Calcul des diagrammes en parcourant les tranches du billon comprises dans l'intervalle
		QVector<int> sectorsSum(nbSectors,0);
		centerX = sliceWidth/2;
		centerY = sliceHeight/2;
		diffMax = 0;

		for ( k=minOfInterval ; k<=maxOfInterval ; ++k ) {
			const arma::Slice &slice = billon.slice(k);
			const arma::Slice &prevSlice = billon.slice(k-1);
			if ( marrow != 0 ) {
				centerX = marrow->at(k).x;
				centerY = marrow->at(k).y;
			}
			for ( j=0 ; j<sliceHeight ; ++j ) {
				for ( i=0 ; i<sliceWidth ; ++i ) {
					sectorIdx = pieChart.partOfAngle( TWO_PI-ANGLE(centerX,centerY,i,j) );
					currentSliceValue = slice.at(j,i);
					previousSliceValue = prevSlice.at(j,i);
					if ( (currentSliceValue > minValue) && (previousSliceValue > minValue) ) {
						diffVal = qAbs(currentSliceValue-previousSliceValue);
						if ( diffVal > _minimalDifference ) {
							histogramsData[sectorIdx][diffVal].value += 1.;
							sectorsSum[sectorIdx] += diffVal;
							diffMax = qMax(diffMax,diffVal);
						}
					}
				}
			}
		}

		createDiagrams( histogramsData, sectorsSum, nbSectors, nbValues, diffMax );
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
}

void PieChartDiagrams::initializeDiagramsData( QVector< QVector<QwtIntervalSample> > &histogramsData, const int &nbSectors, const int &nbValues ) {
	for ( int i=0 ; i<nbSectors ; ++i ) {
		for ( int j=0 ; j<nbValues ; ++j ) {
			histogramsData[i][j].interval.setInterval(j,j+1);
		}
	}
}

void PieChartDiagrams::createDiagrams( QVector< QVector<QwtIntervalSample> > &histogramsData, const QVector<int> &sectorsSum, const int &nbSectors, const int &nbValues, const int &diffMax ) {
	int i;
	// Création des objets histogrammes et affectation de leurs données correspondantes
	_histograms.reserve(nbSectors);
	for ( i=0 ; i<nbSectors ; ++i ) {
		histogramsData[i].remove(diffMax,nbValues-diffMax);
		_histograms.append(new QwtPlotHistogram());
		static_cast<QwtIntervalSeriesData *>(_histograms.last()->data())->setSamples(histogramsData[i]);
	}

	// Création du diagramme circulaire
	_polarCurve = new QwtPolarCurve();
	PointPolarSeriesData *pointsDatas = new PointPolarSeriesData();
	const qreal coeff = 360./nbSectors;
	for ( i=0 ; i<nbSectors ; ++i ) {
		pointsDatas->append(QwtPointPolar(i*coeff,sectorsSum[i]));
	}
	pointsDatas->append(QwtPointPolar(0,sectorsSum[0]));
	_polarCurve->setData(pointsDatas);
}
