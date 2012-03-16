#include "inc/slicehistogram.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/intensityinterval.h"
#include "inc/global.h"
#include <qwt_plot_histogram.h>

SliceHistogram::SliceHistogram() : _billon(0), _marrow(0), _histogram(new QwtPlotHistogram()), _histogramMaximums(new QwtPlotHistogram()) {
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

void SliceHistogram::setModel( const Billon *billon ) {
	_billon = billon;
}

void SliceHistogram::setModel( const Marrow *marrow ) {
	_marrow = marrow;
}

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

void SliceHistogram::constructHistogram( const IntensityInterval &intensityInterval ) {
	_datasHistogram.clear();
	_datasMaximums.clear();
	if ( _billon != 0 ) {
		const Billon &billon = *_billon;
		const uint width = billon.n_cols;
		const uint height = billon.n_rows;
		const uint depth = billon.n_slices;
		const uint nbPixels = height*width;
		const int minValue = intensityInterval.min();
		const int maxValue = intensityInterval.max();

		_datasHistogram.reserve(depth-1);

		qreal cumul;
		if ( _marrow == 0 ) {
			for ( uint k=1 ; k<depth ; ++k ) {
				const arma::imat &slice = billon.slice(k);
				const arma::imat &prevSlice = billon.slice(k-1);
				cumul = 0;
				for ( uint j=0 ; j<height ; ++j ) {
					for ( uint i=0 ; i<width ; ++i ) {
						cumul += qAbs(RESTRICT_TO_INTERVAL(slice.at(j,i),minValue,maxValue) - RESTRICT_TO_INTERVAL(prevSlice.at(j,i),minValue,maxValue));
					}
				}
				_datasHistogram.append(QwtIntervalSample(cumul/nbPixels,k-1,k));
			}
		}
		else {
			uint marrowX, marrowY;
			int iRadius;
			int diameter = 10;
			diameter /= _billon->voxelWidth();
			const int radius = diameter/2;
			const int radiusMax = radius+1;
			QList<int> circleLines;
			int nbPixels2;
			for ( int j=-radius ; j<radiusMax ; ++j ) {
				circleLines.append(qSqrt(qAbs(qPow(radius,2)-qPow(j,2))));
				nbPixels2 += 2*circleLines.last()+1;
			}
			for ( uint k=1 ; k<depth ; ++k ) {
				const arma::imat &slice = billon.slice(k);
				const arma::imat &prevSlice = billon.slice(k-1);
				marrowX = _marrow->at(k).x+radiusMax;
				marrowY = _marrow->at(k).y+radiusMax;
				cumul = 0;
				for ( int j=-radius ; j<radiusMax ; ++j ) {
					iRadius = circleLines[j+radius];
					for ( int i=-iRadius ; i<iRadius+1 ; ++i ) {
						cumul += qAbs(RESTRICT_TO_INTERVAL(slice.at(marrowY+j,marrowX+i),minValue,maxValue) - RESTRICT_TO_INTERVAL(prevSlice.at(marrowY+j,marrowX+i),minValue,maxValue));
					}
				}
				_datasHistogram.append(QwtIntervalSample(cumul/nbPixels2,k-1,k));
			}
		}

		QList<int> pics;
		qDebug() << "Pics primaires :";
		for ( int i=10 ; i<_datasHistogram.size()-10 ; ++i ) {
			double value = _datasHistogram.at(i).value;
			if ( (value > _datasHistogram.at(i-1).value) && (value > _datasHistogram.at(i+1).value) &&
				 (value > _datasHistogram.at(i-2).value) && (value > _datasHistogram.at(i+2).value) &&
				 (value > _datasHistogram.at(i-3).value) && (value > _datasHistogram.at(i+3).value) &&
				 (value > _datasHistogram.at(i-4).value) && (value > _datasHistogram.at(i+4).value) &&
				 (value > _datasHistogram.at(i-5).value) && (value > _datasHistogram.at(i+5).value) &&
				 (value > _datasHistogram.at(i-6).value) && (value > _datasHistogram.at(i+6).value) &&
				 (value > _datasHistogram.at(i-7).value) && (value > _datasHistogram.at(i+7).value) &&
				 (value > _datasHistogram.at(i-8).value) && (value > _datasHistogram.at(i+8).value) &&
				 (value > _datasHistogram.at(i-9).value) && (value > _datasHistogram.at(i+9).value) &&
				 (value > _datasHistogram.at(i-10).value) && (value > _datasHistogram.at(i+10).value)) {
				pics.append(i);
				_datasMaximums.append(_datasHistogram.at(i));
				qDebug() << i;
			}
		}

//		qDebug() << "Pics significatifs :";
//		QList<int> pics2;
//		pics2.append(pics.first());
//		for ( int i=1 ; i<pics.size()-1 ; ++i ) {
//			double value = _datasHistogram.at(pics.at(i)).value;
//			if ( (value > _datasHistogram.at(pics.at(i-1)).value) && (value > _datasHistogram.at(pics.at(i+1)).value ) && (value > _datasHistogram.at(pics.at(i-2)).value) && (value > _datasHistogram.at(pics.at(i+2)).value ) ) {
//				pics2.append(pics.at(i));
//				//_datasMaximums.append(_datasHistogram.at(pics.at(i)));
//				qDebug() << pics.at(i);
//			}
//		}
//		pics2.append(pics.last());

//		qDebug() << "Pics 3èmé niveau :";
//		for ( int i=1 ; i<pics2.size()-1 ; ++i ) {
//			if ( (_datasHistogram.at(pics2.at(i)).value > _datasHistogram.at(pics2.at(i-1)).value) && (_datasHistogram.at(pics2.at(i)).value > _datasHistogram.at(pics2.at(i+1)).value ) ) {
//				_datasMaximums.append(_datasHistogram.at(pics2.at(i)));
//				qDebug() << pics2.at(i);
//			}
//		}
	}
	_histogram->setSamples(_datasHistogram);
	_histogramMaximums->setSamples(_datasMaximums);
}
