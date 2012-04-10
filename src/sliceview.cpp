#include "inc/sliceview.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/opticalflow.h"
#include "inc/intensityinterval.h"
#include "inc/piechart.h"
#include "inc/piepart.h"

#include <QPainter>

SliceView::SliceView() : _typeOfView(SliceType::CURRENT),
	_movementThresholdMin(0), _movementThresholdMax(1000), _movementWithBackground(false), _useNextSliceInsteadOfCurrentSlice(false),
	_flowAlpha(FLOW_ALPHA_DEFAULT), _flowEpsilon(FLOW_EPSILON_DEFAULT), _flowMaximumIterations(FLOW_MAXIMUM_ITERATIONS),
	_restrictedAreaResolution(100), _restrictedAreaThreshold(-900), _restrictedAreaDrawCircle(true), _restrictedAreaBeginRadius(0)
{
}

/*******************************
 * Public setters
 *******************************/

void SliceView::setTypeOfView( const SliceType::SliceType &type ) {
	if ( type > SliceType::_SLICE_TYPE_MIN_ && type < SliceType::_SLICE_TYPE_MAX_ ) {
		_typeOfView = type;
	}
}

void SliceView::setMovementThresholdMin( const int &threshold ) {
	_movementThresholdMin = threshold;
}

void SliceView::setMovementThresholdMax( const int &threshold ) {
	_movementThresholdMax = threshold;
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

void SliceView::setRestrictedAreaResolution( const int &resolution ) {
	_restrictedAreaResolution = resolution;
}

void SliceView::setRestrictedAreaThreshold( const int &threshold )  {
	_restrictedAreaThreshold = threshold;
}

void SliceView::enableRestrictedAreaCircle( const bool &enable )  {
	_restrictedAreaDrawCircle = enable;
}

void SliceView::setRestrictedAreaBeginRadius( const int &radius ) {
	_restrictedAreaBeginRadius = radius;
}

void SliceView::drawSlice( QImage &image, const Billon &billon, const Marrow *marrow, const int &sliceNumber, const IntensityInterval &intensityInterval ) {
	if ( sliceNumber > -1 && sliceNumber < static_cast<int>(billon.n_slices) ) {
		switch (_typeOfView) {
			// Affichage de la coupe moyenne
			case SliceType::AVERAGE :
				drawAverageSlice( image, billon, intensityInterval );
				break;
			// Affichage de la coupe médiane
			case SliceType::MEDIAN :
				drawMedianSlice( image, billon, intensityInterval );
				break;
			// Affichage de la coupe de mouvements
			case SliceType::MOVEMENT :
				drawMovementSlice( image, billon, sliceNumber, intensityInterval );
				break;
			case SliceType::FLOW :
				drawFlowSlice( image, billon, sliceNumber );
				break;
			case SliceType::RESTRICTED_AREA :
				drawCurrentSlice( image, billon, sliceNumber, intensityInterval );
				drawRestrictedArea( image, billon, marrow, sliceNumber, intensityInterval );
				break;
			case SliceType::CURRENT:
			default :
				// Affichage de la coupe courante
				drawCurrentSlice( image, billon, sliceNumber, intensityInterval );
				break;
		}
	}
}


/*******************************
 * Private functions
 *******************************/

void SliceView::drawCurrentSlice( QImage &image, const Billon &billon, const int &sliceNumber, const IntensityInterval &intensityInterval ) {
	const Slice &slice = billon.slice(sliceNumber);
	const uint width = slice.n_cols;
	const uint height = slice.n_rows;
	const int minValue = intensityInterval.min();
	const int maxValue = intensityInterval.max();
	const qreal fact = 255.0/intensityInterval.size();

	QRgb * line = (QRgb *) image.bits();
	int color;
	uint i,j;

	for ( j=0 ; j<height ; j++) {
		for ( i=0 ; i<width ; i++) {
			color = (qBound(minValue,slice.at(j,i),maxValue)-minValue)*fact;
			*(line++) = qRgb(color,color,color);
		}
	}
}

void SliceView::drawAverageSlice( QImage &image, const Billon &billon, const IntensityInterval &intensityInterval ) {
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = billon.n_slices;
	const int minValue = intensityInterval.min();
	const int maxValue = intensityInterval.max();
	const double fact = 255.0/(depth*intensityInterval.size());

	QRgb * line = (QRgb *) image.bits();
	int color;

	for (uint j=0 ; j<height ; j++) {
		for (uint i=0 ; i<width ; i++) {
			color = depth*(-minValue);
			for (uint k=0 ; k<depth ; k++) {
				color += qBound(minValue,billon.at(j,i,k),maxValue);
			}
			color *= fact;
			*(line++) = qRgb(color,color,color);
		}
	}
}

void SliceView::drawMedianSlice( QImage &image, const Billon &billon, const IntensityInterval &intensityInterval ) {
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = billon.n_slices;
	const int minValue = intensityInterval.min();
	const int maxValue = intensityInterval.max();
	const double fact = 255.0/intensityInterval.size();

	QRgb * line =(QRgb *) image.bits();
	int color;

	for (uint j=0 ; j<height ; j++) {
		for (uint i=0 ; i<width ; i++) {
			ivec tab(depth);
			for (uint k=0 ; k<depth ; k++) {
				tab(k) = qBound(minValue,billon.at(j,i,k),maxValue);
			}
			color = (median(tab)-minValue)*fact;
			*(line++) = qRgb(color,color,color);
		}
	}
}

void SliceView::drawMovementSlice( QImage &image, const Billon &billon, const int &sliceNumber, const IntensityInterval &intensityInterval ) {

	const Slice &previousSlice = billon.slice(sliceNumber > 0 ? sliceNumber-1 : sliceNumber+1);
	const Slice &currentSlice = billon.slice(sliceNumber);
	const Slice &toCompareSlice = billon.slice(_useNextSliceInsteadOfCurrentSlice && sliceNumber < static_cast<int>(billon.n_slices)-1 ? sliceNumber+1 : sliceNumber );

	const int width = previousSlice.n_cols;
	const int height = previousSlice.n_rows;
	const int minValue = intensityInterval.min();
	const int maxValue = intensityInterval.max();
	const qreal fact = 255.0/intensityInterval.size();

	const QRgb background = qRgb(0,0,0);
	const QRgb foreground = qRgb(0,200,200);
	QRgb *line;

	int i, j, color, pixelAbsDiff;

	line = (QRgb *) image.bits();
	if ( _movementWithBackground ) {
		for ( j=0 ; j<height ; j++) {
			for ( i=0 ; i<width ; i++) {
				pixelAbsDiff = qAbs(((qBound(minValue,previousSlice.at(j,i),maxValue)-minValue)) - ((qBound(minValue,toCompareSlice.at(j,i),maxValue)-minValue)));
				if ( pixelAbsDiff > _movementThresholdMin && pixelAbsDiff < _movementThresholdMax ) *line = foreground;
				else {
					color = (qBound(minValue,currentSlice.at(j,i),maxValue)-minValue)*fact;
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
				pixelAbsDiff = qAbs(((qBound(minValue,previousSlice.at(j,i),maxValue)-minValue)) - ((qBound(minValue,toCompareSlice.at(j,i),maxValue)-minValue)));
				if ( pixelAbsDiff > _movementThresholdMin && pixelAbsDiff < _movementThresholdMax ) *line = foreground;
				else *line = background;
				line++;
			}
		}
	}
}

void SliceView::drawFlowSlice( QImage &image, const Billon &billon, const int &sliceNumber ) {

	const Slice &currentSlice = billon.slice(sliceNumber);

	const int width = currentSlice.n_cols;
	const int height = currentSlice.n_rows;
	int i, j;

	VectorsField *field = OpticalFlow::compute(billon,sliceNumber,_flowAlpha,_flowEpsilon,_flowMaximumIterations);

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

//	QPainter painter(&image);
//	painter.setPen(Qt::white);
//	for ( j=5 ; j<height-1 ; j+=5 ) {
//		for ( i=5 ; i<width-1 ; i+=5 ) {
//			painter.drawLine(i,j,i+(*field)[j][i].x(),j+(*field)[j][i].y());
//		}
//	}

	delete field;
}

void SliceView::drawRestrictedArea( QImage &image, const Billon &billon, const Marrow *marrow, const int &sliceNumber, const IntensityInterval &intensityInterval ) {
	const int nbPoints = _restrictedAreaResolution;

	const int max = intensityInterval.max();
	const int min = intensityInterval.min();
	const int threshold = qBound(min,_restrictedAreaThreshold,max);

	const Slice &currentSlice = billon.slice(sliceNumber);

	const int imageWidth = currentSlice.n_cols;
	const int imageHeight = currentSlice.n_rows;
	const int xCenter = (marrow != 0 && marrow->interval().containsClosed(sliceNumber))?marrow->at(sliceNumber-marrow->interval().min()).x:imageWidth/2;
	const int yCenter = (marrow != 0 && marrow->interval().containsClosed(sliceNumber))?marrow->at(sliceNumber-marrow->interval().min()).y:imageHeight/2;

	QPolygon polygon(nbPoints);
	int polygonPoints[2*nbPoints+2];

	qreal xEdge, yEdge, orientation, cosAngle, sinAngle;
	int i,j,k;
	orientation = 0.;
	k = 0;

	for ( i=0 ; i<nbPoints ; ++i ) {
		orientation += (TWO_PI/static_cast<qreal>(nbPoints));
		cosAngle = qCos(orientation);
		sinAngle = -qSin(orientation);
		xEdge = xCenter + _restrictedAreaBeginRadius*cosAngle;
		yEdge = yCenter + _restrictedAreaBeginRadius*sinAngle;
		while ( qBound(min,currentSlice.at(yEdge,xEdge),max) > threshold && xEdge>0 && yEdge>0 && xEdge<imageWidth && yEdge<imageHeight ) {
			xEdge += cosAngle;
			yEdge += sinAngle;
		}
		polygonPoints[k++] = xEdge;
		polygonPoints[k++] = yEdge;
	}
	polygonPoints[k++] = polygonPoints[0];
	polygonPoints[k] = polygonPoints[1];

	polygon.setPoints(nbPoints+1,polygonPoints);

	QPainter painter;

	if ( _restrictedAreaDrawCircle ) {
		QRect boudingRect = polygon.boundingRect();
		const int xLeft = boudingRect.left();
		const int xRight = boudingRect.right();
		const int yTop = boudingRect.top();
		const int yBottom = boudingRect.bottom()+1;
		const int endLineWidth = imageWidth-xRight;

		const QRgb bg = qRgb(0,0,0);

		painter.begin(&image);
			painter.fillRect(0,0,imageWidth,yTop,bg);
			painter.fillRect(0,yTop,xLeft,yBottom,bg);
			painter.fillRect(xRight,yTop,imageWidth,yBottom,bg);
			painter.fillRect(0,yBottom,imageWidth,imageHeight,bg);
		painter.end();

		QRgb * line =(QRgb *) image.bits();
		line += imageWidth*yTop;
		for ( j=yTop ; j<yBottom ; ++j ) {
			line += xLeft;
			for ( i=xLeft ; i<xRight ; ++i ) {
				if ( !polygon.containsPoint(QPoint(i,j),Qt::WindingFill) ) {
					 *line = bg;
				}
				line++;
			}
			line += endLineWidth;
		}
	}

	painter.begin(&image);
	painter.setPen(Qt::green);
	painter.drawPolygon(polygon);
}

//void SliceView::drawRestrictedArea( QImage &image, const Billon &billon, const int &sliceNumber, const IntensityInterval &intensityInterval ) {
//	const int nbPoints = _restrictedAreaResolution;

//	const int max = intensityInterval.max();
//	const int min = intensityInterval.min();
//	const int threshold = qBound(min,_restrictedAreaThreshold,max);

//	const Slice &currentSlice = billon.slice(sliceNumber);

//	QPainter painter(&image);
//	painter.setPen(Qt::green);

//	const int xCenter = image.width()/2;
//	const int yCenter = image.height()/2;

//	qreal xEdge, yEdge, xOldEdge, yOldEdge, xOrigin, yOrigin, xBary, yBary, orientation, radiusMax, radiusMean, cosAngle, sinAngle;
//	orientation = xBary = yBary = radiusMax = radiusMean = 0.;

//	for ( int i=0 ; i<nbPoints ; ++i ) {
//		orientation += (TWO_PI/(qreal)nbPoints);
//		cosAngle = qCos(orientation);
//		sinAngle = -qSin(orientation);
//		xEdge = xCenter + 50*cosAngle;
//		yEdge = yCenter + 50*sinAngle;
//		while ( qBound(min,currentSlice.at(yEdge,xEdge),max) > threshold ) {
//			xEdge += cosAngle;
//			yEdge += sinAngle;
//		}
//		if ( i == 0 ) {
//			xOrigin = xEdge;
//			yOrigin = yEdge;
//		}
//		else {
//			painter.drawLine(static_cast<int>(xOldEdge),static_cast<int>(yOldEdge),static_cast<int>(xEdge),static_cast<int>(yEdge));
//		}
//		xBary += xEdge;
//		yBary += yEdge;
//		xOldEdge = xEdge;
//		yOldEdge = yEdge;
//		radiusMean += qSqrt( qPow(xEdge-xCenter,2) + qPow(yEdge-yCenter,2) );
//		radiusMax = qMax(radiusMax, qSqrt(qPow(xEdge-xCenter,2) + qPow(yEdge-yCenter,2)) );
//	}
//	painter.drawLine(xOldEdge,yOldEdge,xOrigin,yOrigin);

//	if ( _restrictedAreaDrawCircle ) {
//		xBary /= (qreal)nbPoints;
//		yBary /= (qreal)nbPoints;
//		radiusMean /= (qreal)nbPoints;
//		const QPointF center(xBary,yBary);
//		painter.drawEllipse(center,3,3);
//		painter.drawEllipse(center,radiusMax,radiusMax);
//		painter.setPen(Qt::red);
////		painter.drawEllipse(center,radiusMean,radiusMean);
//		painter.setPen(QColor(255,127,0));
//		painter.drawEllipse(center,(radiusMean+radiusMax)/2.,(radiusMean+radiusMax)/2.);

////		QRgb * line =(QRgb *) image.bits();
////		QRgb bg = qRgb(0,0,0);
////		const int width = currentSlice.n_cols;
////		const int height = currentSlice.n_rows;
////		for ( int j=0 ; j<height ; ++j ) {
////			for ( int i=0 ; i<width ; ++i ) {
////				if ( qSqrt( qPow(i-xBary,2) + qPow(j-yBary,2) ) > radiusLength ) {
////					 *line = bg;
////				}
////				line++;
////			}
////		}

//	}
//}
