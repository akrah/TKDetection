#include "inc/sliceview.h"

#include "inc/billon.h"
#include "inc/global.h"
#include "inc/marrow.h"
#include <QPainter>

SliceView::SliceView() : _billon(0), _marrow(0), _lowThreshold(0), _highThreshold(0), _typeOfView(SliceType::CURRENT), _motionThreshold(0), _motionWithBackground(false) {
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

void SliceView::enableMotionWithBackground( const bool &enable ) {
	_motionWithBackground = enable;
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
	const int previousIndex = sliceNumber > 0 ? sliceNumber-1 : sliceNumber;
	const imat &currentSlice = _billon->slice(sliceNumber);
	const imat &previousSlice = _billon->slice(previousIndex);

	const uint width = currentSlice.n_cols;
	const uint height = currentSlice.n_rows;
	const int &minValue = _lowThreshold;
	const int &maxValue = _highThreshold;
	const qreal fact = 255.0/(maxValue-minValue);
	const QRgb background = qRgb(0,0,0);
	const QRgb foreground = qRgb(0,200,200);

	QImage image(width,height,QImage::Format_ARGB32);
	QRgb * line = (QRgb *) image.bits();
	int pixelAbsDiff;

	if ( _motionWithBackground ) {
		for (uint j=0 ; j<height ; j++) {
			for (uint i=0 ; i<width ; i++) {
				pixelAbsDiff = qAbs(((RESTRICT_TO_INTERVAL(currentSlice.at(j,i),minValue,maxValue)-minValue)*fact) - ((RESTRICT_TO_INTERVAL(previousSlice.at(j,i),minValue,maxValue)-minValue)*fact));
				if ( pixelAbsDiff > _motionThreshold ) *line = foreground;
				else {
					const int color = (RESTRICT_TO_INTERVAL(currentSlice.at(j,i),minValue,maxValue)-minValue)*fact;
					//*line = qRgb(color,color,color);
					*line = background;
				}
				line++;
			}
		}
	}
	else {
		for (uint j=0 ; j<height ; j++) {
			for (uint i=0 ; i<width ; i++) {
				pixelAbsDiff = qAbs(((RESTRICT_TO_INTERVAL(currentSlice.at(j,i),minValue,maxValue)-minValue)*fact) - ((RESTRICT_TO_INTERVAL(previousSlice.at(j,i),minValue,maxValue)-minValue)*fact));
				if ( pixelAbsDiff > _motionThreshold ) *line = foreground;
				else *line = background;
				line++;
			}
		}
	}

//	// Calcul du mouvement en alignant les coupes par rapport à la moelle la moelle
//	const Coord2D marrowShift = _marrow->at(sliceNumber).shiftTo(_marrow->at(previousIndex));
//	const int &xShift = marrowShift.x;
//	const int &yShift = marrowShift.y;
//	image.fill(0xff000000);
//	if ( yShift < 0 ) line += -yShift*width;
//	for (int j=(yShift>=0?0:-yShift) ; j<(yShift>=0?height-yShift:height) ; j++) {
//		if ( xShift < 0 ) line += -xShift;
//		for (int i=(xShift>=0?0:-xShift) ; i<(xShift>=0?width-xShift:width) ; i++) {
//			pixelAbsDiff = qAbs(((RESTRICT_TO_INTERVAL(currentSlice.at(j,i),minValue,maxValue)-minValue)*fact) - ((RESTRICT_TO_INTERVAL(previousSlice.at(j+yShift,i+xShift),minValue,maxValue)-minValue)*fact));
//			if ( pixelAbsDiff > _motionThreshold ) *line = foreground;
//			else {
////				const int color = (RESTRICT_TO_INTERVAL(currentSlice.at(j,i),minValue,maxValue)-minValue)*fact;
////				*line = qRgb(color,color,color);
//				*line = background;
//			}
//			line++;
//		}
//		if ( xShift > 0 ) line += xShift;
//	}

	// Suppressionndes points isolés
	const int rayon = 2;
	QList<int> indexToCompare;
	indexToCompare.append(-rayon*width-rayon);
	indexToCompare.append(rayon*width-rayon);
	indexToCompare.append(-rayon*width+rayon);
	indexToCompare.append(rayon*width+rayon);
	for ( int i=-rayon+1 ; i<rayon ; i++ ) {
		indexToCompare.append(-rayon*width+i);
		indexToCompare.append(rayon*width+i);
		indexToCompare.append(i*width-rayon);
		indexToCompare.append(i*width+rayon);
	}
	QList<int> indexToChange;
	for ( int j=-rayon+1 ; j<rayon ; j++ ) {
		for ( int i=-rayon+1 ; i<rayon ; i++ ) {
			indexToChange.append(j*width+i);
		}
	}

//	line = ((QRgb *) image.bits()) + width + 1;
//	if ( _motionWithBackground ) {
//		for (uint j=1 ; j<height-1 ; j++) {
//			for (uint i=1 ; i<width-1 ; i++) {
//				if ( *line           == foreground &&
//					 *(line-width-1) != foreground && *(line-width) != foreground && *(line-width+1) != foreground &&
//					 *(line-1)       != foreground &&                                *(line+1)       != foreground &&
//					 *(line+width-1) != foreground && *(line+width) != foreground && *(line+width+1) != foreground ) {
//								const int color = (RESTRICT_TO_INTERVAL(currentSlice.at(j,i),minValue,maxValue)-minValue)*fact;
//								*line = qRgb(color,color,color);
//				}
//				line++;
//			}
//			line+=2;
//		}
//	}
//	else {
//		for (uint j=1 ; j<height-1 ; j++) {
//			for (uint i=1 ; i<width-1 ; i++) {
//				if ( *line           == foreground &&
//					 *(line-width-1) != foreground && *(line-width) != foreground && *(line-width+1) != foreground &&
//					 *(line-1)       != foreground &&                                *(line+1)       != foreground &&
//					 *(line+width-1) != foreground && *(line+width) != foreground && *(line+width+1) != foreground ) {
//								*line = background;
//				}
//				line++;
//			}
//			line+=2;
//		}
//	}

	line = ((QRgb *) image.bits()) + rayon*width + rayon;
	if ( _motionWithBackground ) {
//		for (uint j=2 ; j<height-2 ; j++) {
//			for (uint i=2 ; i<width-2 ; i++) {
//				if ( *(line-2*width-2) != foreground && *(line-2*width-1) != foreground && *(line-2*width) != foreground && *(line-2*width+1) != foreground && *(line-2*width+2) != foreground &&
//					 *(line-width-2)   != foreground &&                                                                                                        *(line-width+2)   != foreground &&
//					 *(line-2)         != foreground &&                                                                                                        *(line+2)         != foreground &&
//					 *(line+width-2)   != foreground &&                                                                                                        *(line+width+2)   != foreground &&
//					 *(line+2*width-2) != foreground && *(line+2*width-1) != foreground && *(line+2*width) != foreground && *(line+2*width+1) != foreground && *(line+2*width+2) != foreground ) {
//								const int color = (RESTRICT_TO_INTERVAL(currentSlice.at(j,i),minValue,maxValue)-minValue)*fact;
//								const QRgb rgbColor = qRgb(color,color,color);
//								*(line-width-1) = rgbColor; *(line-width) = rgbColor; *(line-width+1) = rgbColor;
//								*(line-1)       = rgbColor; *(line)       = rgbColor; *(line+1)       = rgbColor;
//								*(line+width-1) = rgbColor; *(line+width) = rgbColor; *(line+width+1) = rgbColor;
//				}
//				line++;
//			}
//			line+=4;
//		}
//	}
//	else {
		QListIterator<int> iterCompare(indexToCompare);
		QListIterator<int> iterChange(indexToChange);
		for (uint j=rayon ; j<height-rayon ; j++) {
			for (uint i=rayon ; i<width-rayon ; i++) {
//				if ( *(line-2*width-2) != foreground && *(line-2*width-1) != foreground && *(line-2*width) != foreground && *(line-2*width+1) != foreground && *(line-2*width+2) != foreground &&
//					 *(line-width-2)   != foreground &&                                                                                                        *(line-width+2)   != foreground &&
//					 *(line-2)         != foreground &&                                                                                                        *(line+2)         != foreground &&
//					 *(line+width-2)   != foreground &&                                                                                                        *(line+width+2)   != foreground &&
//					 *(line+2*width-2) != foreground && *(line+2*width-1) != foreground && *(line+2*width) != foreground && *(line+2*width+1) != foreground && *(line+2*width+2) != foreground ) {
//								*(line-width-1) = background; *(line-width) = background; *(line-width+1) = background;
//								*(line-1)       = background; *(line)       = background; *(line+1)       = background;
//								*(line+width-1) = background; *(line+width) = background; *(line+width+1) = background;
//				}
//				line++;
				bool isBackground = true;
				iterCompare.toFront();
				while ( isBackground && iterCompare.hasNext() ) {
					isBackground &= (*(line+iterCompare.next()) != foreground);
				}
				if ( isBackground ) {
					iterChange.toFront();
					while ( iterChange.hasNext() ) {
						*(line+iterChange.next()) = background;
					}
				}
				line++;
			}
			line+=4;
		}
	}

//	// Erosion
//	image = imageErodee.copy(0,0,width,height);
//	line = ((QRgb *) image.bits()) + width + 1;
//	lineErodee = ((QRgb *) imageErodee.bits()) + width + 1;
//	for (uint j=1 ; j<height-1 ; j++) {
//		for (uint i=1 ; i<width-1 ; i++) {
//			if ( *(line) == background ) {
//				*(lineErodee-width-1) = background; *(lineErodee-width) = background; *(lineErodee-width+1) = background;
//				*(lineErodee-1) = background; *(lineErodee+1) = background;
//				*(lineErodee+width-1) = background; *(lineErodee+width) = background; *(lineErodee+width+1) = background;
//			}
//			line++;
//			lineErodee++;
//		}
//		line+=2;
//		lineErodee+=2;
//	}

//	// Dilatation
//	image = imageErodee.copy(0,0,width,height);
//	line = ((QRgb *) image.bits()) + width + 1;
//	lineErodee = ((QRgb *) imageErodee.bits()) + width + 1;
//	for (uint j=1 ; j<height-1 ; j++) {
//		for (uint i=1 ; i<width-1 ; i++) {
//			if ( *(line) == foreground ) {
//				*(lineErodee-width-1) = foreground; *(lineErodee-width) = foreground; *(lineErodee-width+1) = foreground;
//				*(lineErodee-1) = foreground; *(lineErodee+1) = foreground;
//				*(lineErodee+width-1) = foreground; *(lineErodee+width) = foreground; *(lineErodee+width+1) = foreground;
//			}
//			line++;
//			lineErodee++;
//		}
//		line+=2;
//		lineErodee+=2;
//	}

	painter.drawImage(0,0,image);
}
