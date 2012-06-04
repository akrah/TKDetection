#include "inc/sliceview.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/opticalflow.h"
#include "inc/interval.h"
#include "inc/piechart.h"
#include "inc/piepart.h"

#include <QPainter>

SliceView::SliceView() : _typeOfView(SliceType::CURRENT),
	_movementThresholdMin(100), _movementThresholdMax(200), _movementWithBackground(false), _useNextSliceInsteadOfCurrentSlice(false),
	_flowAlpha(FLOW_ALPHA_DEFAULT), _flowEpsilon(FLOW_EPSILON_DEFAULT), _flowMaximumIterations(FLOW_MAXIMUM_ITERATIONS),
	_restrictedAreaResolution(100), _restrictedAreaThreshold(-900), _restrictedAreaBeginRadius(5)
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

void SliceView::setRestrictedAreaBeginRadius( const int &radius ) {
	_restrictedAreaBeginRadius = radius;
}

void SliceView::drawSlice( QImage &image, const Billon &billon, const Marrow *marrow, const int &sliceNumber, const Interval &intensityInterval ) {
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

void SliceView::drawCurrentSlice( QImage &image, const Billon &billon, const int &sliceNumber, const Interval &intensityInterval ) {
	const Slice &slice = billon.slice(sliceNumber);
	const uint width = slice.n_cols;
	const uint height = slice.n_rows;
	const int minValue = intensityInterval.minValue();
	const int maxValue = intensityInterval.maxValue();
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

void SliceView::drawAverageSlice( QImage &image, const Billon &billon, const Interval &intensityInterval ) {
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = billon.n_slices;
	const int minValue = intensityInterval.minValue();
	const int maxValue = intensityInterval.maxValue();
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

void SliceView::drawMedianSlice( QImage &image, const Billon &billon, const Interval &intensityInterval ) {
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = billon.n_slices;
	const int minValue = intensityInterval.minValue();
	const int maxValue = intensityInterval.maxValue();
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

void SliceView::drawMovementSlice( QImage &image, const Billon &billon, const int &sliceNumber, const Interval &intensityInterval ) {

//	const Slice &previousSlice = billon.slice(sliceNumber > 0 ? sliceNumber-1 : sliceNumber+1);
//	const Slice &currentSlice = billon.slice(sliceNumber);
//	const Slice &toCompareSlice = billon.slice(_useNextSliceInsteadOfCurrentSlice && sliceNumber < static_cast<int>(billon.n_slices)-1 ? sliceNumber+1 : sliceNumber );

//	const int width = previousSlice.n_cols;
//	const int height = previousSlice.n_rows;
//	const int minValue = intensityInterval.min();
//	const int maxValue = intensityInterval.max();
//	const qreal fact = 255.0/intensityInterval.size();

//	const QRgb background = qRgb(0,0,0);
//	const QRgb foreground = qRgb(0,200,200);
//	QRgb *line;

//	int i, j, color, pixelAbsDiff;

//	line = (QRgb *) image.bits();
//	if ( _movementWithBackground ) {
//		for ( j=0 ; j<height ; j++) {
//			for ( i=0 ; i<width ; i++) {
//				pixelAbsDiff = qAbs(qBound(minValue,previousSlice.at(j,i),maxValue) - qBound(minValue,toCompareSlice.at(j,i),maxValue));
//				if ( pixelAbsDiff > _movementThresholdMin && pixelAbsDiff < _movementThresholdMax ) *line = foreground;
//				else {
//					color = (qBound(minValue,currentSlice.at(j,i),maxValue)-minValue)*fact;
//					*line = qRgb(color,color,color);
//					//*line = background;
//				}
//				line++;
//			}
//		}
//	}
//	else {
//		for ( j=0 ; j<height ; j++) {
//			for ( i=0 ; i<width ; i++) {
//				pixelAbsDiff = qAbs(qBound(minValue,previousSlice.at(j,i),maxValue) - qBound(minValue,toCompareSlice.at(j,i),maxValue));
//				if ( pixelAbsDiff > _movementThresholdMin && pixelAbsDiff < _movementThresholdMax ) *line = foreground;
//				else *line = background;
//				line++;
//			}
//		}
//	}
	const Slice &previousSlice = billon.slice(sliceNumber > 0 ? sliceNumber-1 : sliceNumber+1);
	const Slice &toCompareSlice = billon.slice(_useNextSliceInsteadOfCurrentSlice && sliceNumber < static_cast<int>(billon.n_slices)-1 ? sliceNumber+1 : sliceNumber );
	const uint width = previousSlice.n_cols;
	const uint height = previousSlice.n_rows;
	const int minValue = intensityInterval.minValue();
	const int maxValue = intensityInterval.maxValue();
	const qreal fact = 255./500;

	QRgb * line = (QRgb *) image.bits();
	int color;
	uint i,j;

	for ( j=0 ; j<height ; j++) {
		for ( i=0 ; i<width ; i++) {
			color = qMin(qAbs(qBound(minValue,previousSlice.at(j,i),maxValue) - qBound(minValue,toCompareSlice.at(j,i),maxValue)),500) * fact;
			*(line++) = qRgb(color,color,color);
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

void SliceView::drawRestrictedArea( QImage &image, const Billon &billon, const Marrow *marrow, const int &sliceNumber, const Interval &intensityInterval ) {
	const int nbPoints = _restrictedAreaResolution;

	const int max = intensityInterval.maxValue();
	const int min = intensityInterval.minValue();
	const int threshold = qBound(min,_restrictedAreaThreshold,max);

	const Slice &currentSlice = billon.slice(sliceNumber);

	const int imageWidth = currentSlice.n_cols;
	const int imageHeight = currentSlice.n_rows;
	const int xCenter = (marrow != 0 && marrow->interval().containsClosed(sliceNumber))?marrow->at(sliceNumber-marrow->interval().minValue()).x:imageWidth/2;
	const int yCenter = (marrow != 0 && marrow->interval().containsClosed(sliceNumber))?marrow->at(sliceNumber-marrow->interval().minValue()).y:imageHeight/2;

	QPolygon polygon(nbPoints);
	int polygonPoints[2*nbPoints+2];

	qreal xEdge, yEdge, orientation, cosAngle, sinAngle;
	int i,k;
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

	const qreal radius = 0.5*qSqrt(polygon.boundingRect().width()*polygon.boundingRect().width() + polygon.boundingRect().height()*polygon.boundingRect().height());
	std::cout << "Rayon de la boite englobante : " << radius << " (" << radius*billon.voxelWidth() << " mm)" << std::endl;

	QPainter painter(&image);
	painter.setPen(Qt::green);
	painter.drawPolygon(polygon);
}
