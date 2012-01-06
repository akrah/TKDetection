#include "inc/slicehistogram.h"

#include "inc/billon.h"
#include <qwt_plot.h>

namespace {
	template<class T>
	inline T RESTRICT_TO_INTERVAL(T x, T min, T max) { return qMax((min),qMin((max),(x))); }
}

SliceHistogram::SliceHistogram( QwtPlot *parent ) : QObject(), QwtPlotHistogram(),  _billon(0), _lowThreshold(0), _highThreshold(0) {
	attach(parent);
}

void SliceHistogram::setModel( const Billon *billon ) {
	_billon = billon;
	if ( _billon != 0 ) {
		_lowThreshold = _billon->minValue();
		_highThreshold = _billon->maxValue();
	}
	else {
		_lowThreshold = _highThreshold = 0;
	}
	constructHistogram();
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

		double cumul;
		for (unsigned int k=1 ; k<depth ; k++) {
			const imat &slice = billon.slice(k);
			const imat &prevSlice = billon.slice(k-1);
			cumul = 0;
			for (unsigned int j=0 ; j<height ; j++) {
				for (unsigned int i=0 ; i<width ; i++) {
					cumul += qAbs(RESTRICT_TO_INTERVAL(slice.at(j,i),minValue,maxValue) - RESTRICT_TO_INTERVAL(prevSlice.at(j,i),minValue,maxValue));
				}
			}
			_datas.append(QwtIntervalSample(cumul/nbPixels,k-1,k));
		}
	}
	static_cast<QwtIntervalSeriesData *>(data())->setSamples(_datas);
	emit histogramUpdated();
}
