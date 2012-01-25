#include "inc/piechartdiagrams.h"

#include "inc/billon.h"
#include "inc/global.h"
#include "inc/marrow.h"
#include "inc/piechart.h"
#include "inc/piepart.h"
#include "inc/pointpolarseriesdata.h"
#include <qwt_plot_histogram.h>
#include <qwt_polar_curve.h>

PieChartDiagrams::PieChartDiagrams() :  _billon(0), _marrow(0), _pieChart(0), _beginSlice(-1), _endSlice(-1), _lowThreshold(0), _highThreshold(0), _minimalDifference(140), _polarCurve(0) {
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

void PieChartDiagrams::setModel( const Billon * const billon ) {
	_billon = billon;
}

void PieChartDiagrams::setModel( const PieChart * const pieChart ) {
	_pieChart = pieChart;
}

void PieChartDiagrams::setModel( const Marrow * const marrow ) {
	_marrow = marrow;
}

void PieChartDiagrams::attach( const QList<QwtPlot *> & plots ) {
	const int nbPlots = plots.size();
	const int nbHistograms = _histograms.size();
	for ( int i=0 ; i<nbPlots && i<nbHistograms ; ++i ) {
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

void PieChartDiagrams::setLowThreshold( const int &threshold ) {
	_lowThreshold = threshold;
}

void PieChartDiagrams::setHighThreshold( const int &threshold ) {
	_highThreshold = threshold;
}

void PieChartDiagrams::setBillonInterval( const int &beginSlice, const int &endSlice ) {
	const bool ok = beginSlice>=0 && beginSlice<=endSlice;
	_beginSlice = ok?beginSlice:-1;
	_endSlice = ok?endSlice:-1;
}

void PieChartDiagrams::setMinimalDifference( const int &minimalDifference ) {
	_minimalDifference = minimalDifference;
}

void PieChartDiagrams::compute() {
	clearAll();
	if ( _billon != 0 && _pieChart != 0 && intervalIsValid()  ) {

		const int nbSectors = _pieChart->nbSectors();
		const int nbValues = _highThreshold-_lowThreshold+1;

		// Création des données des diagrammes
		QVector<int> sectorsSum(nbSectors,0);
		QVector< QVector<QwtIntervalSample> > histogramsData(nbSectors,QVector<QwtIntervalSample>(nbValues,QwtIntervalSample(0.,0.,0.)));

		for ( int i=0 ; i<nbSectors ; ++i ) {
			for ( int j=0 ; j<nbValues ; ++j ) {
				histogramsData[i][j].interval.setInterval(j,j+1);
			}
		}

		// Calcul des diagrammes en parcourant les tranches _beginSlice à _endSlice du billon
		const int sliceWidth = _billon->n_cols;
		const int sliceMiddleX = sliceWidth/2;
		const int sliceHeight = _billon->n_rows;
		const int sliceMiddleY = sliceHeight/2;
		const bool existMarrow = _marrow != 0;
		int previousSlice = _beginSlice==0?1:_beginSlice-1;
		int centerX, centerY;
		int diffMax = 0;

		for ( int k=_beginSlice ; k<=_endSlice ; ++k ) {
			centerX = existMarrow?_marrow->at(k).x:sliceMiddleX;
			centerY = existMarrow?_marrow->at(k).y:sliceMiddleY;
			for ( int j=0 ; j<sliceHeight ; ++j ) {
				for ( int i=0 ; i<sliceWidth ; ++i ) {
					const int sectorIdx = _pieChart->partOfAngle( TWO_PI-ANGLE(centerX,centerY,i,j) );
					const int diffVal = qAbs(RESTRICT_TO_INTERVAL(_billon->at(j,i,k),_lowThreshold,_highThreshold)-RESTRICT_TO_INTERVAL(_billon->at(j,i,previousSlice),_lowThreshold,_highThreshold));
					if ( diffVal > _minimalDifference ) {
						histogramsData[sectorIdx][diffVal].value += 1.;
						sectorsSum[sectorIdx] += diffVal;
						diffMax = qMax(diffMax,diffVal);
					}
				}
			}
			previousSlice = k;
		}

		// Création des objets histogrammes et affectation de leurs données correspondantes
		_histograms.reserve(nbSectors);
		for ( int i=0 ; i<nbSectors ; ++i ) {
			histogramsData[i].remove(diffMax,nbValues-diffMax);
			_histograms.append(new QwtPlotHistogram());
			static_cast<QwtIntervalSeriesData *>(_histograms.last()->data())->setSamples(histogramsData[i]);
		}

		// Création du diagramme circulaire
		_polarCurve = new QwtPolarCurve();
		PointPolarSeriesData *pointsDatas = new PointPolarSeriesData();
		const qreal coeff = 360./nbSectors;
		for ( int i=0 ; i<nbSectors ; ++i ) {
			pointsDatas->append(QwtPointPolar(i*coeff,sectorsSum[i]));
		}
		pointsDatas->append(QwtPointPolar(0,sectorsSum[0]));
		_polarCurve->setData(pointsDatas);
	}
}

/*******************************
 * Private functions
 *******************************/

bool PieChartDiagrams::intervalIsValid() const {
	bool ok = (_beginSlice > -1) && (_billon != 0) && (_endSlice < static_cast<int>(_billon->n_slices)) && (_beginSlice <= _endSlice);
	return ok;
}

void PieChartDiagrams::clearAll() {
	while ( !_histograms.isEmpty() ) {
		_histograms.removeLast();
	}
	if ( _polarCurve != 0 ) {
		delete _polarCurve;
		_polarCurve = 0;
	}
}
