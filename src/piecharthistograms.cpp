#include "inc/piecharthistograms.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/pie_def.h"
#include "inc/piechart.h"
#include "inc/piepart.h"
#include "inc/pointpolarseriesdata.h"
#include <qwt_plot_histogram.h>
#include <qwt_polar_curve.h>

namespace {
	template<class T>
	inline T RESTRICT_TO_INTERVAL(T x, T min, T max) { return qMax((min),qMin((max),(x))); }
}

PieChartHistograms::PieChartHistograms() :  _billon(0), _marrow(0), _pieChart(0), _beginSlice(-1), _endSlice(-1), _lowThreshold(0), _highThreshold(0), _polarCurve(0) {
}

PieChartHistograms::~PieChartHistograms() {
	clearAll();
}

/*******************************
 * Public getters
 *******************************/

int PieChartHistograms::count() const {
	return _histograms.size();
}

/*******************************
 * Public setters
 *******************************/

void PieChartHistograms::setModel( const Billon *billon ) {
	_billon = billon;
}

void PieChartHistograms::setModel( const PieChart * pieChart ) {
	_pieChart = pieChart;
}

void PieChartHistograms::setModel( const Marrow * marrow ) {
	_marrow = marrow;
}

void PieChartHistograms::attach( const QList<QwtPlot *> & plots ) {
	const int nbPlots = plots.size();
	const int nbHistograms = _histograms.size();
	for ( int i=0 ; i<nbPlots && i<nbHistograms ; ++i ) {
		if ( _histograms[i] != 0 && plots[i] != 0 )	_histograms[i]->attach(plots[i]);
	}
}

void PieChartHistograms::attach( QwtPolarPlot * polarPlot ) {
	if ( _polarCurve != 0 ) _polarCurve->attach(polarPlot);
}

void PieChartHistograms::setLowThreshold( const int &threshold ) {
	_lowThreshold = threshold;
}

void PieChartHistograms::setHighThreshold( const int &threshold ) {
	_highThreshold = threshold;
}

void PieChartHistograms::setBillonInterval( const int &beginSlice, const int &endSlice ) {
	const bool ok = beginSlice>=0 && beginSlice<=endSlice;
	_beginSlice = ok?beginSlice:-1;
	_endSlice = ok?endSlice:-1;
}

void PieChartHistograms::computeHistograms2() {
	clearAll();
	if ( _billon != 0 && _pieChart != 0 && intervalIsValid()  ) {

		QList< QVector<QwtIntervalSample> > datas;

		const int nbSectors = _pieChart->nbSectors();
		const int nbValues = _highThreshold-_lowThreshold+1;

		// Création des données des histogrammes : une liste d'intervalles par secteur angulaire
		datas.reserve(nbSectors);
		for ( int i=0 ; i<nbSectors ; ++i ) {
			datas.append( QVector<QwtIntervalSample>(nbValues,QwtIntervalSample(0.,0.,0.)) );
			for ( int j=_lowThreshold ; j<=_highThreshold ; ++j ) {
				datas[i][j-_lowThreshold].interval.setInterval(j,j+1);
			}
		}

		const int sliceWidth = _billon->n_cols;
		const int sliceMiddleX = sliceWidth/2;
		const int sliceHeight = _billon->n_rows;
		const int sliceMiddleY = sliceHeight/2;
		const bool existMarrow = _marrow != 0;
		int centerX, centerY;

		// Calcul des histogrammes en parcourant l'ensemble les tranches _beginSlice à _endSlice du billon
		for ( int k=_beginSlice ; k<=_endSlice ; ++k ) {
			centerX = existMarrow?_marrow->at(k).x:sliceMiddleX;
			centerY = existMarrow?_marrow->at(k).y:sliceMiddleY;
			for ( int j=0 ; j<sliceHeight ; ++j ) {
				for ( int i=0 ; i<sliceWidth ; ++i ) {
					const int sectorIdx = _pieChart->partOfAngle( TWO_PI-ANGLE(centerX,centerY,i,j) );
					const int pixVal = RESTRICT_TO_INTERVAL(_billon->at(j,i,k),_lowThreshold,_highThreshold)-_lowThreshold;
					if ( pixVal ) datas[sectorIdx][pixVal].value += 1.;
				}
			}
		}

		// Création des objets histogrammes et affectation de leurs données correspondantes
		_histograms.reserve(nbSectors);
		for ( int i=0 ; i<nbSectors ; ++i ) {
			_histograms.append(new QwtPlotHistogram());
			static_cast<QwtIntervalSeriesData *>(_histograms.last()->data())->setSamples(datas[i]);
		}
	}
}

void PieChartHistograms::computeHistograms() {
	clearAll();
	if ( _billon != 0 && _pieChart != 0 && intervalIsValid()  ) {

		QList< QVector<QwtIntervalSample> > datas;

		const int nbSectors = _pieChart->nbSectors();
		const int nbValues = _highThreshold-_lowThreshold+1;

		// Création des données des histogrammes : une liste d'intervalles par secteur angulaire
		datas.reserve(nbSectors);
		for ( int i=0 ; i<nbSectors ; ++i ) {
			datas.append( QVector<QwtIntervalSample>(nbValues,QwtIntervalSample(0.,0.,0.)) );
			for ( int j=0 ; j<nbValues ; ++j ) {
				datas[i][j].interval.setInterval(j,j+1);
			}
		}

		const int sliceWidth = _billon->n_cols;
		const int sliceMiddleX = sliceWidth/2;
		const int sliceHeight = _billon->n_rows;
		const int sliceMiddleY = sliceHeight/2;
		const bool existMarrow = _marrow != 0;
		int centerX, centerY;

		QVector<int> sectorsSum(nbSectors,0);

		// Calcul des histogrammes en parcourant l'ensemble les tranches _beginSlice à _endSlice du billon
		int previousSlice = _beginSlice==0?1:_beginSlice-1;
		for ( int k=_beginSlice ; k<=_endSlice ; ++k ) {
			centerX = existMarrow?_marrow->at(k).x:sliceMiddleX;
			centerY = existMarrow?_marrow->at(k).y:sliceMiddleY;
			for ( int j=0 ; j<sliceHeight ; ++j ) {
				for ( int i=0 ; i<sliceWidth ; ++i ) {
					const int sectorIdx = _pieChart->partOfAngle( TWO_PI-ANGLE(centerX,centerY,i,j) );
					const int diffVal = std::abs(RESTRICT_TO_INTERVAL(_billon->at(j,i,k),_lowThreshold,_highThreshold)-RESTRICT_TO_INTERVAL(_billon->at(j,i,previousSlice),_lowThreshold,_highThreshold));
					if ( diffVal > 140 ) {
						datas[sectorIdx][diffVal].value += 1.;
						sectorsSum[sectorIdx] += diffVal;
					}
				}
			}
			previousSlice = k;
		}

		// Création du diagramme circulaire
		_polarCurve = new QwtPolarCurve();
		PointPolarSeriesData *pointsDatas = new PointPolarSeriesData();
		const double coeff = 360./nbSectors;
		for ( int i=0 ; i<nbSectors ; ++i ) {
			pointsDatas->append(QwtPointPolar(i*coeff,sectorsSum[i]));
		}
		pointsDatas->append(QwtPointPolar(0,sectorsSum[0]));
		_polarCurve->setData(pointsDatas);

		// Création des objets histogrammes et affectation de leurs données correspondantes
		_histograms.reserve(nbSectors);
		for ( int i=0 ; i<nbSectors ; ++i ) {
			_histograms.append(new QwtPlotHistogram());
			static_cast<QwtIntervalSeriesData *>(_histograms.last()->data())->setSamples(datas[i]);
		}
	}
}

/*******************************
 * Private functions
 *******************************/

bool PieChartHistograms::intervalIsValid() const {
	bool ok = _beginSlice>-1 && _billon != 0 && _endSlice<static_cast<int>(_billon->n_slices) && _beginSlice<=_endSlice;
	return ok;
}

void PieChartHistograms::clearAll() {
	while ( !_histograms.isEmpty() ) {
		_histograms.removeLast();
	}
	if ( _polarCurve != 0 ) {
		delete _polarCurve;
		_polarCurve = 0;
	}
}
