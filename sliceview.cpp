#include "sliceview.h"

#include "billon.h"
#include <QPainter>

SliceView::SliceView() : QObject(0), QPixmap(), _billon(0), _currentSlice(0), _lowThreshold(0), _highThreshold(0), _typeOfView(CURRENT_SLICE) {
}

int SliceView::currentSlice() const {
	return _currentSlice;
}

void SliceView::setModel( const Billon *billon ) {
	_billon = billon;
	_currentSlice = 0;
	if ( _billon != 0 ) {
		_lowThreshold = _billon->minValue();
		_highThreshold = _billon->maxValue();
	}
}

void SliceView::drawSlice( const int &sliceNumber ) {
	if ( _billon != 0 ) {
		if ( (sliceNumber > -1) && (sliceNumber < static_cast<int>(_billon->n_slices)) ) _currentSlice = sliceNumber;

		const uint width = _billon->n_cols;
		const uint height = _billon->n_rows;
		const uint depth = _billon->n_slices;
		const float minValue = _lowThreshold;
		const float maxValue = _highThreshold;

		QImage image(width,height,QImage::Format_ARGB32);
		int c;
		float fact;
		QRgb * line;

		switch (_typeOfView) {
			// Affichage de la coupe moyenne
			case AVERAGE_SLICE :
				fact = (255.0/(depth*(maxValue-minValue)));
				for (unsigned int j=0 ; j<height ; j++) {
					line = (QRgb *) image.scanLine(j);
					for (unsigned int i=0 ; i<width ; i++) {
						c = 0;
						for (unsigned int k=0 ; k<depth ; k++) {
							c += qMax(minValue,qMin(maxValue,(float)_billon->at(j,i,k)))-minValue;
						}
						c = c*fact;
						line[i] = qRgb(c,c,c);
					}
				}
				break;
			// Affichage de la coupe médiane
			case MEDIAN_SLICE :
				fact = 255.0/(maxValue-minValue);
				for (unsigned int j=0 ; j<height ; j++) {
					line = (QRgb *) image.scanLine(j);
					for (unsigned int i=0 ; i<width ; i++) {
						ivec tab(depth);
						for (unsigned int k=0 ; k<depth ; k++) {
							tab(k) = qMax(minValue,qMin(maxValue,(float)_billon->at(j,i,k)))-minValue;
						}
						c = median(tab)*fact;
						line[i] = qRgb(c,c,c);
					}
				}
				break;
			// Affichage de la coupe courante
			case CURRENT_SLICE:
				fact = 255.0/(maxValue-minValue);
				for (unsigned int j=0 ; j<height ; j++) {
					line = (QRgb *) image.scanLine(j);
					for (unsigned int i=0 ; i<width ; i++) {
						c = _billon->at(j,i,_currentSlice);
						c = (qMax(minValue,qMin(maxValue,(float)c))-minValue)*fact;
						line[i] = qRgb(c,c,c);
					}
				}
				break;
			default :
				image.fill(0xff333333);
		}
		convertFromImage(image);
	}
	else {
		QImage image(width(),height(),QImage::Format_ARGB32);
		image.fill(0x00555555);
		convertFromImage(image);
	}
	emit updated(*this);
}

void SliceView::setLowThreshold(const int &threshold) {
	_lowThreshold = threshold;
	drawSlice();
	emit thresholdUpdated();
}

void SliceView::setHighThreshold(const int &threshold) {
	_highThreshold = threshold;
	drawSlice();
	emit thresholdUpdated();
}

void SliceView::setTypeOfView(const int &type) {
	_typeOfView = type;
	drawSlice();
	emit typeOfViewChanged(type);
}
