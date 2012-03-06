#include "inc/sliceview.h"

#include "inc/billon.h"
#include "inc/global.h"
#include "inc/marrow.h"
#include "inc/opticalflow.h"
#include <QPainter>

SliceView::SliceView() : _billon(0), _marrow(0), _lowThreshold(0), _highThreshold(0), _typeOfView(SliceType::CURRENT),
	_motionThreshold(0), _motionGroupMinimumRadius(1), _motionWithBackground(false), _useNextSliceInsteadOfCurrentSlice(false) {
}

/*******************************
 * Public setters
 *******************************/

void SliceView::setModel( const Billon *billon ) {
	_billon = billon;
}

void SliceView::setModel( const Marrow *marrow ) {
	_marrow = marrow;
}

void SliceView::setTypeOfView( const SliceType::SliceType &type ) {
	if ( type > SliceType::_SLICE_TYPE_MIN_ && type < SliceType::_SLICE_TYPE_MAX_ ) {
		_typeOfView = type;
	}
}

void SliceView::setLowThreshold(const int &threshold) {
	_lowThreshold = threshold;
}

void SliceView::setHighThreshold(const int &threshold) {
	_highThreshold = threshold;
}

void SliceView::setMotionThreshold( const int &threshold ) {
	_motionThreshold = threshold;
}

void SliceView::setMotionGroupMinimumRadius( const int &radius ) {
	_motionGroupMinimumRadius = qMax(radius,0);
}

void SliceView::enableMotionWithBackground( const bool &enable ) {
	_motionWithBackground = enable;
}

void SliceView::useNextSliceInsteadOfCurrentSlice( const bool &enable ) {
	_useNextSliceInsteadOfCurrentSlice = enable;
}

void SliceView::drawSlice( QPainter &painter, const int &sliceNumber ) {
	if ( _billon != 0 && sliceNumber > -1 && sliceNumber < static_cast<int>(_billon->n_slices) ) {
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
			case SliceType::FLOW :
				drawFlowSlice( painter, sliceNumber );
				break;
			case SliceType::CURRENT:
			default :
				// Affichage de la coupe courante
				drawCurrentSlice( painter, sliceNumber );
				break;
		}
	}
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

//	// Affichage des coupes avec la moelle au centre de l'image
//	const Coord2D marrowShift = _marrow->at(sliceNumber).shiftTo(Coord2D(width/2,height/2));
//	const int &xShift = marrowShift.x;
//	const int &yShift = marrowShift.y;
//	if ( yShift > 0 ) line += yShift*width;
//	image.fill(0xff000000);
//	for (int j=(yShift<0?-yShift:0) ; j<(yShift<0?height:height-yShift) ; j++) {
//		if ( xShift > 0 ) line += xShift;
//		for (int i=(xShift<0?-xShift:0) ; i<(xShift<0?width:width-xShift) ; i++) {
//			color = (RESTRICT_TO_INTERVAL(slice.at(j,i),minValue,maxValue)-minValue)*fact;
//			*(line++) = qRgb(color,color,color);
//		}
//		if ( xShift < 0 ) line += -xShift;
//	}

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

	const imat &previousSlice = _billon->slice(sliceNumber > 0 ? sliceNumber-1 : sliceNumber+1);
	const imat &toCompareSlice = _billon->slice(_useNextSliceInsteadOfCurrentSlice && sliceNumber < static_cast<int>(_billon->n_slices)-1 ? sliceNumber+1 : sliceNumber );

	const int width = previousSlice.n_cols;
	const int height = previousSlice.n_rows;
	const qreal fact = 255.0/(_highThreshold-_lowThreshold);

	QImage image(width,height,QImage::Format_ARGB32);
	const QRgb background = qRgb(0,0,0);
	const QRgb foreground = qRgb(0,200,200);
	QRgb *line;

	int i, j, color, pixelAbsDiff;

	line = (QRgb *) image.bits();
	if ( _motionWithBackground ) {
		for ( j=0 ; j<height ; j++) {
			for ( i=0 ; i<width ; i++) {
				pixelAbsDiff = qAbs(((RESTRICT_TO_INTERVAL(previousSlice.at(j,i),_lowThreshold,_highThreshold)-_lowThreshold)*fact) - ((RESTRICT_TO_INTERVAL(toCompareSlice.at(j,i),_lowThreshold,_highThreshold)-_lowThreshold)*fact));
				if ( pixelAbsDiff > _motionThreshold ) *line = foreground;
				else {
					color = (RESTRICT_TO_INTERVAL(previousSlice.at(j,i),_lowThreshold,_highThreshold)-_lowThreshold)*fact;
					*line = qRgb(color,color,color);
					//*line = background;
				}
				line++;
			}
		}
	}
	else {
		for ( j=0 ; j<height ; j++) {
			for ( i=0 ; i<width ; i++) {
				pixelAbsDiff = qAbs(((RESTRICT_TO_INTERVAL(previousSlice.at(j,i),_lowThreshold,_highThreshold)-_lowThreshold)*fact) - ((RESTRICT_TO_INTERVAL(toCompareSlice.at(j,i),_lowThreshold,_highThreshold)-_lowThreshold)*fact));
				if ( pixelAbsDiff > _motionThreshold ) *line = foreground;
				else *line = background;
				line++;
			}
		}
	}

	// Suppressionndes points isolés
	if ( _motionGroupMinimumRadius > 0 ) {
		const int productRadiusWidth = _motionGroupMinimumRadius*width;
		const int diffHeightRadius = height-_motionGroupMinimumRadius;
		const int diffWidthRadius = width-_motionGroupMinimumRadius;
		const int nbCompare = _motionGroupMinimumRadius*8;
		const int nbChange = qPow(2*_motionGroupMinimumRadius-1,2);

		int indexToCompare[nbCompare];
		int indexToChange[nbChange];
		const int *supCompare = indexToCompare+nbCompare;
		const int *supChange = indexToChange+nbChange;
		int *pointer;

		bool isBackground;
		int counter;

		counter = 0;
		indexToCompare[counter++] = -productRadiusWidth-_motionGroupMinimumRadius;
		indexToCompare[counter++] = -productRadiusWidth+_motionGroupMinimumRadius;
		indexToCompare[counter++] = productRadiusWidth-_motionGroupMinimumRadius;
		indexToCompare[counter++] = productRadiusWidth+_motionGroupMinimumRadius;
		for ( i=-_motionGroupMinimumRadius+1 ; i<_motionGroupMinimumRadius ; i++ ) {
			indexToCompare[counter++] = -productRadiusWidth+i;
			indexToCompare[counter++] = productRadiusWidth+i;
			indexToCompare[counter++] = i*width-_motionGroupMinimumRadius;
			indexToCompare[counter++] = i*width+_motionGroupMinimumRadius;
		}

		counter = 0;
		for ( j=-_motionGroupMinimumRadius+1 ; j<_motionGroupMinimumRadius ; j++ ) {
			for ( i=-_motionGroupMinimumRadius+1 ; i<_motionGroupMinimumRadius ; i++ ) {
				indexToChange[counter++] = j*width+i;
			}
		}

		line = ((QRgb *) image.bits()) + productRadiusWidth + _motionGroupMinimumRadius;
		if ( _motionWithBackground ) {
			for ( j=_motionGroupMinimumRadius ; j<diffHeightRadius ; j++) {
				for ( i=_motionGroupMinimumRadius ; i<diffWidthRadius ; i++) {
					isBackground = true;
					pointer = indexToCompare;
					while ( pointer != supCompare ) {
						isBackground &= (*(line+*pointer) != foreground);
						pointer++;
					}
					if ( isBackground ) {
						pointer = indexToChange;
						while ( pointer != supChange ) {
							const int &pointerValue = *pointer;
							color = (RESTRICT_TO_INTERVAL(previousSlice.at( j+pointerValue/width, i+(pointerValue/width)%width ),_lowThreshold,_highThreshold)-_lowThreshold)*fact;
							*(line+pointerValue) = qRgb(color,color,color);
							//*(line+pointerValue) = background;
							pointer++;
						}
					}
					line++;
				}
				line+=(2*_motionGroupMinimumRadius);
			}
		}
		else {
			for ( j=_motionGroupMinimumRadius ; j<diffHeightRadius ; j++) {
				for ( i=_motionGroupMinimumRadius ; i<diffWidthRadius ; i++) {
					isBackground = true;
					pointer = indexToCompare;
					while ( pointer != supCompare ) {
						isBackground &= (*(line+*(pointer++)) != foreground);
					}
					if ( isBackground ) {
						pointer = indexToChange;
						while ( pointer != supChange ) {
							*(line+*(pointer++)) = background;
						}
					}
					line++;
				}
				line+=_motionGroupMinimumRadius*2;
			}
		}
	}

	painter.drawImage(0,0,image);
}

void SliceView::drawFlowSlice( QPainter &painter, const int &sliceNumber ) {

	const imat &currentSlice = _billon->slice(sliceNumber);

	const int width = currentSlice.n_cols;
	const int height = currentSlice.n_rows;

	int i, j;

	painter.setPen(Qt::white);

	OpticalFlow flow(_billon);
	VectorsField *field = flow.computeFlowOnSlice(sliceNumber);

	for ( j=5 ; j<height-1 ; j+=5 ) {
		for ( i=5 ; i<width-1 ; i+=5 ) {
			//if ( qSqrt(qPow((*field)[j][i].x(),2) + qPow((*field)[j][i].y(),2)) > 1 )
				painter.drawLine(i,j,i+(*field)[j][i].x(),j+(*field)[j][i].y());
		}
	}

	delete field;
}
