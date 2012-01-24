#include "inc/sliceview.h"

#include "inc/billon.h"
#include "inc/global.h"
#include "inc/marrow.h"
#include <QPainter>

SliceView::SliceView() : QObject(0), _billon(0), _lowThreshold(0), _highThreshold(0), _typeOfView(SliceType::CURRENT) {
}

SliceView::~SliceView() {
}

/*******************************
 * Public setters
 *******************************/

void SliceView::setModel( const Billon *billon ) {
	_billon = billon;
}

void SliceView::setTypeOfView( const SliceType::SliceType &type ) {
	if ( type > SliceType::_SLICE_TYPE_MIN_ && type < SliceType::_SLICE_TYPE_MAX_ ) {
		_typeOfView = type;
	}
}

/*******************************
 * Public slots
 *******************************/

void SliceView::drawSlice( QPainter &painter, const int &sliceNumber ) {
	if ( _billon != 0 && sliceNumber > -1 && sliceNumber < static_cast<int>(_billon->n_slices)) {
		switch (_typeOfView) {
			// Affichage de la coupe moyenne
			case SliceType::AVERAGE :
				drawAverageSlice( painter);
				break;
			// Affichage de la coupe médiane
			case SliceType::MEDIAN :
				drawMedianSlice( painter );
				break;
			case SliceType::CURRENT:
			default :
				// Affichage de la coupe courante
				drawCurrentSlice( painter, sliceNumber );
				break;
		}
	}
}

void SliceView::setLowThreshold(const int &threshold) {
	_lowThreshold = threshold;
}

void SliceView::setHighThreshold(const int &threshold) {
	_highThreshold = threshold;
}

/*******************************
 * Private functions
 *******************************/

void SliceView::drawCurrentSlice( QPainter &painter, const int &sliceNumber ) {
	const imat &slice = _billon->slice(sliceNumber);
	const uint width = slice.n_cols;
	const uint height = slice.n_rows;
	const int minValue = _lowThreshold;
	const int maxValue = _highThreshold;
	const qreal fact = 255.0/(maxValue-minValue);

	QImage image(width,height,QImage::Format_ARGB32);
	QRgb * line = (QRgb *) image.bits();

	for (uint j=0 ; j<height ; j++) {
		for (uint i=0 ; i<width ; i++) {
			const int c = (RESTRICT_TO_INTERVAL(slice.at(j,i),minValue,maxValue)-minValue)*fact;
			*(line++) = qRgb(c,c,c);
		}
	}

	painter.drawImage(0,0,image);
}

void SliceView::drawAverageSlice( QPainter &painter ) {
	const Billon &billon = *_billon;
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = billon.n_slices;
	const int minValue = _lowThreshold;
	const int maxValue = _highThreshold;
	const double fact = 255.0/(depth*(maxValue-minValue));

	QImage image(width,height,QImage::Format_ARGB32);
	int c;
	QRgb * line = (QRgb *) image.bits();

	for (uint j=0 ; j<height ; j++) {
		for (uint i=0 ; i<width ; i++) {
			c = depth*(-minValue);
			for (uint k=0 ; k<depth ; k++) {
				c += RESTRICT_TO_INTERVAL(billon.at(j,i,k),minValue,maxValue);
			}
			c *= fact;
			*(line++) = qRgb(c,c,c);
		}
	}

	painter.drawImage(0,0,image);
}

void SliceView::drawMedianSlice( QPainter &painter ) {
	const Billon &billon = *_billon;
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = billon.n_slices;
	const int minValue = _lowThreshold;
	const int maxValue = _highThreshold;
	const double fact = 255.0/(maxValue-minValue);

	QImage image(width,height,QImage::Format_ARGB32);
	QRgb * line =(QRgb *) image.bits();

	for (uint j=0 ; j<height ; j++) {
		for (uint i=0 ; i<width ; i++) {
			ivec tab(depth);
			for (uint k=0 ; k<depth ; k++) {
				tab(k) = RESTRICT_TO_INTERVAL(billon.at(j,i,k),minValue,maxValue);
			}
			const int c = (median(tab)-minValue)*fact;
			*(line++) = qRgb(c,c,c);
		}
	}

	painter.drawImage(0,0,image);
}
