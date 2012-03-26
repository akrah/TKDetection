#include "inc/sliceview.h"

#include "inc/billon.h"
#include "inc/global.h"
#include "inc/marrow.h"
#include "inc/opticalflow.h"
#include "inc/intensityinterval.h"
#include "inc/piechart.h"
#include "inc/piepart.h"

#include <QPainter>

SliceView::SliceView() : _typeOfView(SliceType::CURRENT),
	_movementThreshold(0), _movementWithBackground(false), _useNextSliceInsteadOfCurrentSlice(false),
	_flowAlpha(FLOW_ALPHA_DEFAULT), _flowEpsilon(FLOW_EPSILON_DEFAULT), _flowMaximumIterations(FLOW_MAXIMUM_ITERATIONS),
	_restrictedAreaResolution(100), _restrictedAreaSmooth(0.5), _restrictedAreaThreshold(-900), _restrictedAreaDrawCircle(true)
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

void SliceView::setMovementThreshold( const int &threshold ) {
	_movementThreshold = threshold;
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

void SliceView::setRestrictedAreaSmooth( const qreal &smooth ) {
	_restrictedAreaSmooth = smooth;
}

void SliceView::setRestrictedAreaThreshold( const int &threshold )  {
	_restrictedAreaThreshold = threshold;
}

void SliceView::enableRestrictedAreaCircle( const bool &enable )  {
	_restrictedAreaDrawCircle = enable;
}

void SliceView::drawSlice( QImage &image, const Billon &billon, const int &sliceNumber, const IntensityInterval &intensityInterval ) {
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
				drawRestrictedArea( image, billon, sliceNumber, intensityInterval );
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
	const arma::Mat<__billon_type__> &slice = billon.slice(sliceNumber);
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
			color = (RESTRICT_TO_INTERVAL(slice.at(j,i),minValue,maxValue)-minValue)*fact;
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
				color += RESTRICT_TO_INTERVAL(billon.at(j,i,k),minValue,maxValue);
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
				tab(k) = RESTRICT_TO_INTERVAL(billon.at(j,i,k),minValue,maxValue);
			}
			color = (median(tab)-minValue)*fact;
			*(line++) = qRgb(color,color,color);
		}
	}
}

void SliceView::drawMovementSlice( QImage &image, const Billon &billon, const int &sliceNumber, const IntensityInterval &intensityInterval ) {

	const arma::Mat<__billon_type__> &previousSlice = billon.slice(sliceNumber > 0 ? sliceNumber-1 : sliceNumber+1);
	const arma::Mat<__billon_type__> &currentSlice = billon.slice(sliceNumber);
	const arma::Mat<__billon_type__> &toCompareSlice = billon.slice(_useNextSliceInsteadOfCurrentSlice && sliceNumber < static_cast<int>(billon.n_slices)-1 ? sliceNumber+1 : sliceNumber );

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
				pixelAbsDiff = qAbs(((RESTRICT_TO_INTERVAL(previousSlice.at(j,i),minValue,maxValue)-minValue)) - ((RESTRICT_TO_INTERVAL(toCompareSlice.at(j,i),minValue,maxValue)-minValue)));
				if ( pixelAbsDiff > _movementThreshold ) *line = foreground;
				else {
					color = (RESTRICT_TO_INTERVAL(currentSlice.at(j,i),minValue,maxValue)-minValue)*fact;
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
				pixelAbsDiff = qAbs(((RESTRICT_TO_INTERVAL(previousSlice.at(j,i),minValue,maxValue)-minValue)) - ((RESTRICT_TO_INTERVAL(toCompareSlice.at(j,i),minValue,maxValue)-minValue)));
				if ( pixelAbsDiff > _movementThreshold ) *line = foreground;
				else *line = background;
				line++;
			}
		}
	}
}

void SliceView::drawFlowSlice( QImage &image, const Billon &billon, const int &sliceNumber ) {

	const arma::Mat<__billon_type__> &currentSlice = billon.slice(sliceNumber);

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

void SliceView::drawRestrictedArea( QImage &image, const Billon &billon, const int &sliceNumber, const IntensityInterval &intensityInterval ) {
	const int nbPoints = _restrictedAreaResolution;
	const qreal lissageFact = _restrictedAreaSmooth;
	const int threshold = _restrictedAreaThreshold;

	PieChart pie(0,nbPoints);
	const QList<PiePart> &sectors = pie.sectors();
	const arma::Mat<__billon_type__> &currentSlice = billon.slice(sliceNumber);

	QPainter painter(&image);
	painter.setPen(Qt::green);

	const int xCenter = image.width()/2;
	const int yCenter = image.height()/2;

	qreal xEdge, yEdge, xOldEdge, yOldEdge, xOrigin, yOrigin, distX, distY, xBary, yBary, orientation, radiusLength, cosAngle, sinAngle;
	xBary = yBary = radiusLength = 0.;

	for ( int i=0 ; i<sectors.size() ; ++i ) {
		orientation = sectors[i].orientation();
		cosAngle = qCos(orientation);
		sinAngle = -qSin(orientation);
//		xEdge = xCenter + 50*cosAngle;
//		yEdge = yCenter + 50*sinAngle;
		while ( currentSlice.at(yEdge,xEdge) > threshold ) {
			xEdge += cosAngle;
			yEdge += sinAngle;
		}
		if ( i != 0 ) {
			distX = xOldEdge-xEdge;
			distY = yOldEdge-yEdge;
			if ( qAbs(distX) > 10. ) xEdge += distX*lissageFact;
			if ( qAbs(distY) > 10. ) yEdge += distY*lissageFact;
			painter.drawLine(xOldEdge,yOldEdge,xEdge,yEdge);
		}
		else {
			xOrigin = xEdge;
			yOrigin = yEdge;
		}
		xBary += xEdge;
		yBary += yEdge;
		xOldEdge = xEdge;
		yOldEdge = yEdge;
		radiusLength += qSqrt( qPow(xEdge-xCenter,2) + qPow(yEdge-yCenter,2) );
		//painter.drawEllipse((int)xEdge-1,(int)yEdge-1,2,2);
		//painter.drawPoint((int)xEdge,(int)yEdge);
	}
	painter.drawLine(xOldEdge,yOldEdge,xOrigin,yOrigin);
	xBary /= (qreal)nbPoints;
	yBary /= (qreal)nbPoints;
	const QPointF center(xBary,yBary);
	radiusLength += qSqrt( qPow(xBary-xCenter,2) + qPow(yBary-yCenter,2) )*(qreal)nbPoints;
	radiusLength /= (qreal)nbPoints;

	if ( _restrictedAreaDrawCircle ) {
		painter.drawEllipse(center,3,3);
		painter.drawEllipse(center,radiusLength,radiusLength);
	}
}


//void SliceView::drawRestrictedArea( QImage &image, const Billon &billon, const int &sliceNumber, const IntensityInterval &intensityInterval ) {
//	const int threshold = _restrictedAreaThreshold;

//	const arma::Mat<__billon_type__> &currentSlice = billon.slice(sliceNumber);

//	QPainter painter(&image);
//	painter.setPen(Qt::green);

//	const int width = billon.n_cols-1;
//	const int height = billon.n_rows-1;
//	int iForward, iBackward, j;
//	bool find;

//	for ( j=1 ; j<height ; ++j ) {
//		find = false;
//		for ( iForward=1 ; iForward<width && !find ; ++iForward ) {
//			find = (currentSlice.at(j,iForward) > threshold);
//		}
//		if ( find ) {
//			painter.drawEllipse(iForward-1,j-1,2,2);
//			find = false;
//			for ( iBackward=width-1 ; iBackward>0 && !find ; --iBackward ) {
//				find = (currentSlice.at(j,iBackward) > threshold);
//			}
//			if ( find && iBackward > iForward ) {
//				painter.drawEllipse(iBackward-1,j-1,2,2);
//			}
//		}
//	}
//}
