#include "inc/slicehistogram.h"

#include "inc/billon.h"
#include "inc/global.h"
#include <qwt_plot_histogram.h>

SliceHistogram::SliceHistogram() : _billon(0), _histogram(new QwtPlotHistogram()), _lowThreshold(0), _highThreshold(0) {
}

SliceHistogram::~SliceHistogram() {
	if ( _histogram != 0 ) delete _histogram;
}

/*******************************
 * Public getters
 *******************************/

qreal SliceHistogram::value( const int &index ) {
	qreal res = 0.;
	if ( index > -1 && _datas.size() > index ) {
		res = _datas.at(index).value;
	}
	return res;
}

/*******************************
 * Public setters
 *******************************/

void SliceHistogram::setModel( const Billon *billon ) {
	_billon = billon;
	if ( _billon != 0 ) {
		_lowThreshold = _billon->minValue();
		_highThreshold = _billon->maxValue();
	}
	else {
		_lowThreshold = _highThreshold = 0;
	}
}

void SliceHistogram::attach( QwtPlot * const plot ) {
	if ( _histogram != 0 && plot != 0 ) {
		_histogram->attach(plot);
	}
}

void SliceHistogram::detach() {
	if ( _histogram != 0 ) {
		_histogram->detach();
	}
}

void SliceHistogram::setLowThreshold( const int &threshold ) {
	_lowThreshold = threshold;
}

void SliceHistogram::setHighThreshold( const int &threshold ) {
	_highThreshold = threshold;
}

void SliceHistogram::constructHistogram() {
	_datas.clear();
	if ( _billon != 0 ) {
		const Billon &billon = *_billon;
		const uint width = billon.n_cols;
		const uint height = billon.n_rows;
		const uint depth = billon.n_slices;
		const uint nbPixels = height*width;
		const int minValue = _lowThreshold;
		const int maxValue = _highThreshold;

		_datas.reserve(depth-1);

		qreal cumul;
		for ( uint k=1 ; k<depth ; ++k ) {
			const imat &slice = billon.slice(k);
			const imat &prevSlice = billon.slice(k-1);
			cumul = 0;
			for ( uint j=0 ; j<height ; ++j ) {
				for ( uint i=0 ; i<width ; ++i ) {
					cumul += qAbs(RESTRICT_TO_INTERVAL(slice.at(j,i),minValue,maxValue) - RESTRICT_TO_INTERVAL(prevSlice.at(j,i),minValue,maxValue));
				}
			}
			_datas.append(QwtIntervalSample(cumul/nbPixels,k-1,k));
		}

		QList<int> pics;
		qDebug() << "Pics primaires :";
		for ( int i=1 ; i<_datas.size()-1 ; ++i ) {
			if ( (_datas.at(i).value > _datas.at(i-1).value) && (_datas.at(i).value > _datas.at(i+1).value ) ) {
				pics.append(i);
				qDebug() << i;
			}
		}

		_pics.clear();
		qDebug() << "Pics significatifs :";
		for ( int i=1 ; i<pics.size()-1 ; ++i ) {
			if ( (_datas.at(pics.at(i)).value > _datas.at(pics.at(i-1)).value) && (_datas.at(pics.at(i)).value > _datas.at(pics.at(i+1)).value ) ) {
				_pics.append(pics.at(i));
				qDebug() << pics.at(i);
			}
		}
	}
	_histogram->setSamples(_datas);
}
