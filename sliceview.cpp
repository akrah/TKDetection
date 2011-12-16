#include "sliceview.h"

#include "billon.h"
#include <QPainter>

SliceView::SliceView() : QObject(0), QPixmap(), _billon(0), _lowThreshold(0), _highThreshold(0), _currentSlice(0) {
}

int SliceView::currentSlice() {
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
	if ( _billon != 0 && sliceNumber > -1 && sliceNumber < (int)_billon->n_slices ) {
		_currentSlice = sliceNumber;

		const uint width = _billon->n_cols;
		const uint height = _billon->n_rows;
		const float minValue = _lowThreshold;
		const float maxValue = _highThreshold;
		const float fact = (255.0/(maxValue-minValue));

		QImage image(width,height,QImage::Format_ARGB32);
		int c;
		QRgb * line;
		for (unsigned int j=0 ; j<height ; j++) {
			line = (QRgb *) image.scanLine(j);
			for (unsigned int i=0 ; i<width ; i++) {
				c = _billon->at(j,i,sliceNumber);
				c = (qMax(minValue,qMin(maxValue,(float)c))-minValue)*fact;
				line[i] = qRgb(c,c,c);
			}
		}
		convertFromImage(image);
	}
	else {
		fill(0x00555555);
		QPainter painter(this);
		painter.drawLine(0,0,width(),height());
		painter.drawLine(width(),0,0,height());
	}
	emit updated(*this);
}

void SliceView::drawAverageSlice() {
	if ( _billon != 0 ) {
		const uint width = _billon->n_cols;
		const uint height = _billon->n_rows;
		const uint depth = _billon->n_slices;
		const float minValue = _lowThreshold;
		const float maxValue = _highThreshold;
		const float fact = (255.0/(depth*(maxValue-minValue)));

		QImage image(width,height,QImage::Format_ARGB32);
		float c;
		QRgb * line;
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
		convertFromImage(image);
	}
	else {
		fill(0x00555555);
		QPainter painter(this);
		painter.drawLine(0,0,width(),height());
		painter.drawLine(width(),0,0,height());
	}
	emit updated(*this);
}

void SliceView::drawMedianSlice() {
	if ( _billon != 0 ) {
		const uint width = _billon->n_cols;
		const uint height = _billon->n_rows;
		const uint depth = _billon->n_slices;
		const float minValue = _lowThreshold;
		const float maxValue = _highThreshold;
		const float fact = 255.0/(maxValue-minValue);

		QImage image(width,height,QImage::Format_ARGB32);
		float c;
		QRgb * line;
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
		convertFromImage(image);
	}
	else {
		fill(0x00555555);
		QPainter painter(this);
		painter.drawLine(0,0,width(),height());
		painter.drawLine(width(),0,0,height());
	}
	emit updated(*this);
}

void SliceView::setLowThreshold(const int &threshold) {
	_lowThreshold = threshold;
	emit thresholdUpdated();
}

void SliceView::setHighThreshold(const int &threshold) {
	_highThreshold = threshold;
	emit thresholdUpdated();
}

