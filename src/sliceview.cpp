#include "inc/sliceview.h"

#include "inc/billon.h"
#include "inc/global.h"
#include "inc/marrow.h"
#include <QPainter>

SliceView::SliceView() : _billon(0), _lowThreshold(0), _highThreshold(0), _typeOfView(SliceType::CURRENT) {
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
				drawAverageSlice( painter );
				break;
			// Affichage de la coupe médiane
			case SliceType::MEDIAN :
				drawMedianSlice( painter );
				break;
			// Affichage de la coupe de mouvements
			case SliceType::MOVEMENT :
				drawMovementSlice( painter, sliceNumber );
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
	int color;

	for (uint j=0 ; j<height ; j++) {
		for (uint i=0 ; i<width ; i++) {
			color = (RESTRICT_TO_INTERVAL(slice.at(j,i),minValue,maxValue)-minValue)*fact;
			*(line++) = qRgb(color,color,color);
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
	QRgb * line = (QRgb *) image.bits();
	int color;

	for (uint j=0 ; j<height ; j++) {
		for (uint i=0 ; i<width ; i++) {
			color = depth*(-minValue);
			for (uint k=0 ; k<depth ; k++) {
				color += RESTRICT_TO_INTERVAL(billon.at(j,i,k),minValue,maxValue);
			}
			color *= fact;
			*(line++) = qRgb(color,color,color);
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
	int color;

	for (uint j=0 ; j<height ; j++) {
		for (uint i=0 ; i<width ; i++) {
			ivec tab(depth);
			for (uint k=0 ; k<depth ; k++) {
				tab(k) = RESTRICT_TO_INTERVAL(billon.at(j,i,k),minValue,maxValue);
			}
			color = (median(tab)-minValue)*fact;
			*(line++) = qRgb(color,color,color);
		}
	}

	painter.drawImage(0,0,image);
}

void SliceView::drawMovementSlice( QPainter &painter, const int &sliceNumber ) {
	const imat &currentSlice = _billon->slice(sliceNumber);
	const imat &nextSlice = sliceNumber < static_cast<int>(_billon->n_slices)-1 ? _billon->slice(sliceNumber+1) : _billon->slice(sliceNumber);
	const imat &previousSlice = sliceNumber > 0 ? _billon->slice(sliceNumber-1) : _billon->slice(sliceNumber);
	const uint width = nextSlice.n_cols;
	const uint height = nextSlice.n_rows;
	const int minValue = _lowThreshold;
	const int maxValue = _highThreshold;
	const qreal fact = 255.0/(maxValue-minValue);

	QImage image(width,height,QImage::Format_ARGB32);
	QRgb * line = (QRgb *) image.bits();
	int color;

	for (uint j=0 ; j<height ; j++) {
		for (uint i=0 ; i<width ; i++) {
			color = qAbs(((RESTRICT_TO_INTERVAL(nextSlice.at(j,i),minValue,maxValue)-minValue)*fact) - ((RESTRICT_TO_INTERVAL(previousSlice.at(j,i),minValue,maxValue)-minValue)*fact));
			if ( color > 5 && (( currentSlice.at(i,j) > nextSlice.at(j,i) && currentSlice.at(i,j) < previousSlice.at(j,i) ) || ( currentSlice.at(i,j) < nextSlice.at(j,i) && currentSlice.at(i,j) > previousSlice.at(j,i) )) ) *(line++) = qRgb(255,255,255);
			else *(line++) = qRgb(0,0,0);
		}
	}

	painter.drawImage(0,0,image);
}
