#include "inc/slicehistogram.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/intensityinterval.h"
#include "inc/global.h"
#include <qwt_plot_histogram.h>

SliceHistogram::SliceHistogram() : _histogram(new QwtPlotHistogram()), _histogramMaximums(new QwtPlotHistogram()) {
	_histogramMaximums->setBrush(Qt::green);
	_histogramMaximums->setPen(QPen(Qt::green));
}

SliceHistogram::~SliceHistogram() {
	if ( _histogram != 0 ) delete _histogram;
	if ( _histogramMaximums != 0 ) delete _histogramMaximums;
}

/*******************************
 * Public getters
 *******************************/

qreal SliceHistogram::value( const int &index ) const {
	qreal res = 0.;
	if ( index > -1 && _datasHistogram.size() > index ) {
		res = _datasHistogram.at(index).value;
	}
	return res;
}

int SliceHistogram::nbMaximums() const {
	int nbMaximums = _histogramMaximums == 0 ? 0 : _histogramMaximums->dataSize();
	return nbMaximums;
}

int SliceHistogram::sliceOfIemeMaximum( const int &maximumIndex ) const {
	int sliceIndex = -1;
	if ( _histogramMaximums != 0 && maximumIndex>-1 && maximumIndex<static_cast<int>(_histogramMaximums->dataSize()) ) {
		sliceIndex = static_cast<QwtIntervalSample>(_histogramMaximums->data()->sample(maximumIndex)).interval.minValue();
	}
	return sliceIndex;
}

/*******************************
 * Public setters
 *******************************/

void SliceHistogram::attach( QwtPlot * const plot ) {
	if ( plot != 0 ) {
		if ( _histogram != 0 ) {
			_histogram->attach(plot);
		}
		if ( _histogramMaximums != 0 ) {
			_histogramMaximums->attach(plot);
		}
	}
}

void SliceHistogram::detach() {
	if ( _histogram != 0 ) {
		_histogram->detach();
	}
	if ( _histogramMaximums != 0 ) {
		_histogramMaximums->detach();
	}
}

void SliceHistogram::clear() {
	_datasHistogram.clear();
	_histogram->setSamples(_datasHistogram);
	_datasMaximums.clear();
	_histogramMaximums->setSamples(_datasMaximums);
}

void SliceHistogram::constructHistogram( const Billon &billon, const IntensityInterval &intensityInterval ) {
	_datasHistogram.clear();

	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = billon.n_slices;
	const uint nbPixels = height*width;
	const int minValue = intensityInterval.min();
	const int maxValue = intensityInterval.max();

	_datasHistogram.reserve(depth-1);

	qreal cumul;
	for ( uint k=1 ; k<depth ; ++k ) {
		const arma::Slice &slice = billon.slice(k);
		const arma::Slice &prevSlice = billon.slice(k-1);
		cumul = 0;
		for ( uint j=0 ; j<height ; ++j ) {
			for ( uint i=0 ; i<width ; ++i ) {
				cumul += qAbs(RESTRICT_TO_INTERVAL(slice.at(j,i),minValue,maxValue) - RESTRICT_TO_INTERVAL(prevSlice.at(j,i),minValue,maxValue));
			}
		}
		_datasHistogram.append(QwtIntervalSample(cumul/nbPixels,k-1,k));
	}

	_histogram->setSamples(_datasHistogram);
	updateMaximums();
}

void SliceHistogram::constructHistogram( const Billon &billon, const Marrow &marrow, const IntensityInterval &intensityInterval ) {
	_datasHistogram.clear();

	const uint depth = billon.n_slices;
	const int minValue = intensityInterval.min();
	const int maxValue = intensityInterval.max();
	int diameter = 20;
	diameter /= billon.voxelWidth();
	const int radius = diameter/2;
	const int radiusMax = radius+1;
	int i, j, iRadius, nbPixels;
	uint k, marrowX, marrowY;
	qreal cumul;

	_datasHistogram.reserve(depth-1);
	nbPixels = 0;

	QList<int> circleLines;
	for ( j=-radius ; j<radiusMax ; ++j ) {
		circleLines.append(qSqrt(qAbs(qPow(radius,2)-qPow(j,2))));
		nbPixels += 2*circleLines.last()+1;
	}
	for ( k=1 ; k<depth ; ++k ) {
		const arma::Slice &slice = billon.slice(k);
		const arma::Slice &prevSlice = billon.slice(k-1);
		marrowX = marrow[k].x+radiusMax;
		marrowY = marrow[k].y+radiusMax;
		cumul = 0;
		for ( j=-radius ; j<radiusMax ; ++j ) {
			iRadius = circleLines[j+radius];
			for ( i=-iRadius ; i<iRadius+1 ; ++i ) {
				cumul += qAbs(RESTRICT_TO_INTERVAL(slice.at(marrowY+j,marrowX+i),minValue,maxValue) - RESTRICT_TO_INTERVAL(prevSlice.at(marrowY+j,marrowX+i),minValue,maxValue));
			}
		}
		_datasHistogram.append(QwtIntervalSample(cumul/nbPixels,k-1,k));
	}

	_histogram->setSamples(_datasHistogram);
	updateMaximums();
}

void SliceHistogram::updateMaximums() {
	_datasMaximums.clear();

	if ( _datasHistogram.size() > 0 ) {
		QList<int> pics;
		const int filterRadius = 15;
		const int max = _datasHistogram.size()-filterRadius;
		double value;
		qDebug() << "Pics primaires :";
		for ( int i=filterRadius ; i<max ; ++i ) {
			value = _datasHistogram.at(i).value;
			if ( (value > _datasHistogram.at(i-1).value) && (value > _datasHistogram.at(i+1).value) &&
				 (value > _datasHistogram.at(i-2).value) && (value > _datasHistogram.at(i+2).value) &&
				 (value > _datasHistogram.at(i-3).value) && (value > _datasHistogram.at(i+3).value) &&
				 (value > _datasHistogram.at(i-4).value) && (value > _datasHistogram.at(i+4).value) &&
				 (value > _datasHistogram.at(i-5).value) && (value > _datasHistogram.at(i+5).value) &&
				 (value > _datasHistogram.at(i-6).value) && (value > _datasHistogram.at(i+6).value) &&
				 (value > _datasHistogram.at(i-7).value) && (value > _datasHistogram.at(i+7).value) &&
				 (value > _datasHistogram.at(i-8).value) && (value > _datasHistogram.at(i+8).value) &&
				 (value > _datasHistogram.at(i-9).value) && (value > _datasHistogram.at(i+9).value) &&
				 (value > _datasHistogram.at(i-10).value) && (value > _datasHistogram.at(i+10).value) &&
				 (value > _datasHistogram.at(i-11).value) && (value > _datasHistogram.at(i+11).value) &&
				 (value > _datasHistogram.at(i-12).value) && (value > _datasHistogram.at(i+12).value) &&
				 (value > _datasHistogram.at(i-13).value) && (value > _datasHistogram.at(i+13).value) &&
				 (value > _datasHistogram.at(i-14).value) && (value > _datasHistogram.at(i+14).value) &&
				 (value > _datasHistogram.at(i-15).value) && (value > _datasHistogram.at(i+15).value) ) {
				pics.append(i);
				_datasMaximums.append(_datasHistogram.at(i));
				i+=filterRadius-1;
				qDebug() << i;
			}
		}
	}

	_histogramMaximums->setSamples(_datasMaximums);
}
