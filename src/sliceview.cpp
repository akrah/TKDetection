#include "inc/sliceview.h"

#include "inc/billon.h"
#include <QPainter>

namespace {
	template<class T>
	inline T RESTRICT_TO_INTERVAL(T x, T min, T max) { return qMax((min),qMin((max),(x))); }
}

SliceView::SliceView() : QObject(0), _pix(0), _billon(0), _marrow(0), _currentSlice(0), _lowThreshold(0), _highThreshold(0), _typeOfView(SliceType::CURRENT), _drawMarrow(false) {
}

SliceView::~SliceView() {
	if ( _pix != 0 ) delete _pix;
}

void SliceView::setModel( const Billon *billon ) {
	_billon = billon;
	_marrow = 0;
	_currentSlice = 0;
	if ( _billon != 0 ) {
		_lowThreshold = _billon->minValue();
		_highThreshold = _billon->maxValue();
	}
	else {
		_lowThreshold = _highThreshold = 0;
	}
	update();
}

void SliceView::setModel( const Marrow* marrow ) {
	_marrow = marrow;
	update();
}

void SliceView::drawSlice( const int &sliceNumber ) {
	if ( (_billon != 0) && (sliceNumber > -1) && (sliceNumber < static_cast<int>(_billon->n_slices)) ) _currentSlice = sliceNumber;
	update();
}

void SliceView::update() {
	int width = 0;
	int height = 0;
	if ( _pix != 0 ) {
		width = _pix->width();
		height = _pix->height();
		delete _pix;
		_pix = 0;
	}
	if ( _billon != 0 ) {
		switch (_typeOfView) {
			// Affichage de la coupe courante
			case SliceType::CURRENT:
				drawCurrentSlice();
				if ( _drawMarrow ) drawMarrow();
				break;
			// Affichage de la coupe moyenne
			case SliceType::AVERAGE :
				drawAverageSlice();
				break;
			// Affichage de la coupe médiane
			case SliceType::MEDIAN :
				drawMedianSlice();
				break;
			default :
				QImage image(width,height,QImage::Format_ARGB32);
				image.fill(0xff555555);
				_pix = new QPixmap(QPixmap::fromImage(image));
		}
	}
	else {
		QImage image(width,height,QImage::Format_ARGB32);
		image.fill(0xff555555);
		_pix = new QPixmap(QPixmap::fromImage(image));
	}
	emit updated(*_pix);
}

void SliceView::setLowThreshold(const int &threshold) {
	_lowThreshold = threshold;
	update();
	emit thresholdUpdated();
}

void SliceView::setHighThreshold(const int &threshold) {
	_highThreshold = threshold;
	update();
	emit thresholdUpdated();
}

void SliceView::setTypeOfView(const int &type) {
	if ( type > SliceType::_SLICE_TYPE_MIN_ && type < SliceType::_SLICE_TYPE_MAX_ ) {
		_typeOfView = static_cast<SliceType::SliceType>(type);
		update();
		emit typeOfViewChanged(_typeOfView);
	}
}

void SliceView::drawMarrow( bool enable ) {
	_drawMarrow = enable;
	update();
}

void SliceView::drawCurrentSlice() {
	const imat &slice = _billon->slice(_currentSlice);
	const uint width = slice.n_cols;
	const uint height = slice.n_rows;
	const int minValue = _lowThreshold;
	const int maxValue = _highThreshold;
	const float fact = 255.0/(maxValue-minValue);

	QImage image(width,height,QImage::Format_ARGB32);
	QRgb * line = (QRgb *) image.bits();

	for (unsigned int j=0 ; j<height ; j++) {
		for (unsigned int i=0 ; i<width ; i++) {
			const int c = (RESTRICT_TO_INTERVAL(slice.at(j,i),minValue,maxValue)-minValue)*fact;
			*(line++) = qRgb(c,c,c);
		}
	}

	_pix = new QPixmap(QPixmap::fromImage(image));
}

void SliceView::drawAverageSlice() {
	const Billon &billon = *_billon;
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = billon.n_slices;
	const int minValue = _lowThreshold;
	const int maxValue = _highThreshold;
	const float fact = 255.0/(depth*(maxValue-minValue));

	QImage image(width,height,QImage::Format_ARGB32);
	int c;
	QRgb * line = (QRgb *) image.bits();

	for (unsigned int j=0 ; j<height ; j++) {
		for (unsigned int i=0 ; i<width ; i++) {
			c = depth*(-minValue);
			for (unsigned int k=0 ; k<depth ; k++) {
				c += RESTRICT_TO_INTERVAL(billon.at(j,i,k),minValue,maxValue);
			}
			c *= fact;
			*(line++) = qRgb(c,c,c);
		}
	}
	_pix = new QPixmap(QPixmap::fromImage(image));
}

void SliceView::drawMedianSlice() {
	const Billon &billon = *_billon;
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = billon.n_slices;
	const int minValue = _lowThreshold;
	const int maxValue = _highThreshold;
	const float fact = 255.0/(maxValue-minValue);

	QImage image(width,height,QImage::Format_ARGB32);
	QRgb * line =(QRgb *) image.bits();

	for (unsigned int j=0 ; j<height ; j++) {
		for (unsigned int i=0 ; i<width ; i++) {
			ivec tab(depth);
			for (unsigned int k=0 ; k<depth ; k++) {
				tab(k) = RESTRICT_TO_INTERVAL(billon.at(j,i,k),minValue,maxValue);
			}
			const int c = (median(tab)-minValue)*fact;
			*(line++) = qRgb(c,c,c);
		}
	}
	_pix = new QPixmap(QPixmap::fromImage(image));
}

void SliceView::drawMarrow() {
	if ( _marrow != 0 ) {
		const Marrow &marrow = (*_marrow);
		if ( (_currentSlice>=marrow.begin) && (_currentSlice<=marrow.end) ) {
			const Coord2D &coordToDraw = marrow(_currentSlice-marrow.begin);
			std::cout << "Affichage de la moelle en " << coordToDraw << std::endl;

			QPainterPath myPath;
			myPath.addEllipse(coordToDraw.x-5,coordToDraw.y-5,10,10);
			QColor color(100,200,100);

			QPainter painter(_pix);
			painter.setBrush(color);
			painter.setPen(color);
			painter.drawPath(myPath);
		}
	}
}
