#include "inc/sliceview.h"

#include "inc/billon.h"
#include "inc/global.h"
#include "inc/marrow.h"
#include "inc/opticalflow.h"
#include "inc/intensityinterval.h"
#include <QPainter>

SliceView::SliceView() : _billon(0), _marrow(0), _typeOfView(SliceType::CURRENT),
	_movementThreshold(0), _movementGroupMinimumRadius(1), _movementWithBackground(false), _useNextSliceInsteadOfCurrentSlice(false),
	_flowAlpha(FLOW_ALPHA_DEFAULT), _flowEpsilon(FLOW_EPSILON_DEFAULT), _flowMaximumIterations(FLOW_MAXIMUM_ITERATIONS) {
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

void SliceView::setMovementThreshold( const int &threshold ) {
	_movementThreshold = threshold;
}

void SliceView::setMovementGroupMinimumRadius( const int &radius ) {
	_movementGroupMinimumRadius = qMax(radius,0);
}

void SliceView::enableMovementWithBackground( const bool &enable ) {
	_movementWithBackground = enable;
}

void SliceView::useNextSliceInsteadOfCurrentSlice( const bool &enable ) {
	_useNextSliceInsteadOfCurrentSlice = enable;
}

qreal SliceView::flowAlpha() const {
	return _flowAlpha;
}

qreal SliceView::flowEpsilon() const {
	return _flowEpsilon;
}

int SliceView::flowMaximumIterations() const {
	return _flowMaximumIterations;
}

void SliceView::setFlowAlpha( const qreal &alpha ) {
	_flowAlpha = alpha;
}

void SliceView::setFlowEpsilon( const qreal &epsilon ) {
	_flowEpsilon = epsilon;
}

void SliceView::setFlowMaximumIterations( const int &maxIter ) {
	_flowMaximumIterations = maxIter;
}

void SliceView::drawSlice( QPainter &painter, const int &sliceNumber, const IntensityInterval &intensityInterval ) {
	if ( _billon != 0 && sliceNumber > -1 && sliceNumber < static_cast<int>(_billon->n_slices) ) {
		switch (_typeOfView) {
			// Affichage de la coupe moyenne
			case SliceType::AVERAGE :
				drawAverageSlice( painter, intensityInterval );
				break;
			// Affichage de la coupe médiane
			case SliceType::MEDIAN :
				drawMedianSlice( painter, intensityInterval );
				break;
			// Affichage de la coupe de mouvements
			case SliceType::MOVEMENT :
				drawMovementSlice( painter, sliceNumber, intensityInterval );
				break;
			case SliceType::FLOW :
				drawFlowSlice( painter, sliceNumber );
				break;
			case SliceType::CURRENT:
			default :
				// Affichage de la coupe courante
				drawCurrentSlice( painter, sliceNumber, intensityInterval );
				break;
		}
	}
}


/*******************************
 * Private functions
 *******************************/

void SliceView::drawCurrentSlice( QPainter &painter, const int &sliceNumber, const IntensityInterval &intensityInterval ) {
	const imat &slice = _billon->slice(sliceNumber);
	const uint width = slice.n_cols;
	const uint height = slice.n_rows;
	const int minValue = intensityInterval.min();
	const int maxValue = intensityInterval.max();
	const qreal fact = 255.0/intensityInterval.size();

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

void SliceView::drawAverageSlice( QPainter &painter, const IntensityInterval &intensityInterval ) {
	const Billon &billon = *_billon;
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = billon.n_slices;
	const int minValue = intensityInterval.min();
	const int maxValue = intensityInterval.max();
	const double fact = 255.0/(depth*intensityInterval.size());

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

void SliceView::drawMedianSlice( QPainter &painter, const IntensityInterval &intensityInterval ) {
	const Billon &billon = *_billon;
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = billon.n_slices;
	const int minValue = intensityInterval.min();
	const int maxValue = intensityInterval.max();
	const double fact = 255.0/intensityInterval.size();

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

void SliceView::drawMovementSlice( QPainter &painter, const int &sliceNumber, const IntensityInterval &intensityInterval ) {

	const imat &previousSlice = _billon->slice(sliceNumber > 0 ? sliceNumber-1 : sliceNumber+1);
	const imat &toCompareSlice = _billon->slice(_useNextSliceInsteadOfCurrentSlice && sliceNumber < static_cast<int>(_billon->n_slices)-1 ? sliceNumber+1 : sliceNumber );

	const int width = previousSlice.n_cols;
	const int height = previousSlice.n_rows;
	const int minValue = intensityInterval.min();
	const int maxValue = intensityInterval.max();
	const qreal fact = 255.0/intensityInterval.size();

	QImage image(width,height,QImage::Format_ARGB32);
	const QRgb background = qRgb(0,0,0);
	const QRgb foreground = qRgb(0,200,200);
	QRgb *line;

	int i, j, color, pixelAbsDiff;

	line = (QRgb *) image.bits();
	if ( _movementWithBackground ) {
		for ( j=0 ; j<height ; j++) {
			for ( i=0 ; i<width ; i++) {
				pixelAbsDiff = qAbs(((RESTRICT_TO_INTERVAL(previousSlice.at(j,i),minValue,maxValue)-minValue)*fact) - ((RESTRICT_TO_INTERVAL(toCompareSlice.at(j,i),minValue,maxValue)-minValue)*fact));
				if ( pixelAbsDiff > _movementThreshold ) *line = foreground;
				else {
					color = (RESTRICT_TO_INTERVAL(previousSlice.at(j,i),minValue,maxValue)-minValue)*fact;
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
				pixelAbsDiff = qAbs(((RESTRICT_TO_INTERVAL(previousSlice.at(j,i),minValue,maxValue)-minValue)*fact) - ((RESTRICT_TO_INTERVAL(toCompareSlice.at(j,i),minValue,maxValue)-minValue)*fact));
				if ( pixelAbsDiff > _movementThreshold ) *line = foreground;
				else *line = background;
				line++;
			}
		}
	}

	// Suppressionndes points isolés
	if ( _movementGroupMinimumRadius > 0 ) {
		const int productRadiusWidth = _movementGroupMinimumRadius*width;
		const int diffHeightRadius = height-_movementGroupMinimumRadius;
		const int diffWidthRadius = width-_movementGroupMinimumRadius;
		const int nbCompare = _movementGroupMinimumRadius*8;
		const int nbChange = qPow(2*_movementGroupMinimumRadius-1,2);

		int indexToCompare[nbCompare];
		int indexToChange[nbChange];
		const int *supCompare = indexToCompare+nbCompare;
		const int *supChange = indexToChange+nbChange;
		int *pointer;

		bool isBackground;
		int counter;

		counter = 0;
		indexToCompare[counter++] = -productRadiusWidth-_movementGroupMinimumRadius;
		indexToCompare[counter++] = -productRadiusWidth+_movementGroupMinimumRadius;
		indexToCompare[counter++] = productRadiusWidth-_movementGroupMinimumRadius;
		indexToCompare[counter++] = productRadiusWidth+_movementGroupMinimumRadius;
		for ( i=-_movementGroupMinimumRadius+1 ; i<_movementGroupMinimumRadius ; i++ ) {
			indexToCompare[counter++] = -productRadiusWidth+i;
			indexToCompare[counter++] = productRadiusWidth+i;
			indexToCompare[counter++] = i*width-_movementGroupMinimumRadius;
			indexToCompare[counter++] = i*width+_movementGroupMinimumRadius;
		}

		counter = 0;
		for ( j=-_movementGroupMinimumRadius+1 ; j<_movementGroupMinimumRadius ; j++ ) {
			for ( i=-_movementGroupMinimumRadius+1 ; i<_movementGroupMinimumRadius ; i++ ) {
				indexToChange[counter++] = j*width+i;
			}
		}

		line = ((QRgb *) image.bits()) + productRadiusWidth + _movementGroupMinimumRadius;
		if ( _movementWithBackground ) {
			for ( j=_movementGroupMinimumRadius ; j<diffHeightRadius ; j++) {
				for ( i=_movementGroupMinimumRadius ; i<diffWidthRadius ; i++) {
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
							color = (RESTRICT_TO_INTERVAL(previousSlice.at( j+pointerValue/width, i+(pointerValue/width)%width ),minValue,maxValue)-minValue)*fact;
							*(line+pointerValue) = qRgb(color,color,color);
							//*(line+pointerValue) = background;
							pointer++;
						}
					}
					line++;
				}
				line+=(2*_movementGroupMinimumRadius);
			}
		}
		else {
			for ( j=_movementGroupMinimumRadius ; j<diffHeightRadius ; j++) {
				for ( i=_movementGroupMinimumRadius ; i<diffWidthRadius ; i++) {
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
				line+=_movementGroupMinimumRadius*2;
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


	VectorsField *field = OpticalFlow::compute(*_billon,sliceNumber,_flowAlpha,_flowEpsilon,_flowMaximumIterations);

	QImage image(width,height,QImage::Format_ARGB32);
	QRgb * line =(QRgb *) image.bits();
	qreal angle, norme;
	QColor color;

	for ( j=0 ; j<height-1 ; j++) {
		for ( i=0 ; i<width-1 ; i++) {
			angle = (ANGLE(0,0,(*field)[j][i].x(),(*field)[j][i].y())+PI_ON_FOUR)*RAD_TO_DEG_FACT;
			if (angle>360.) angle -= 360.;
			norme = qMin(qSqrt( qPow((*field)[j][i].x(),2) + qPow((*field)[j][i].y(),2) )*20.,255.);
			color.setHsv(angle,norme,norme);
			*(line++) = color.rgb();
		}
		line++;
	}

	painter.drawImage(0,0,image);

//	painter.setPen(Qt::white);
//	for ( j=5 ; j<height-1 ; j+=5 ) {
//		for ( i=5 ; i<width-1 ; i+=5 ) {
//			painter.drawLine(i,j,i+(*field)[j][i].x(),j+(*field)[j][i].y());
//		}
//	}

	delete field;
}
