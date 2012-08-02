#include "inc/sliceview.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/opticalflow.h"
#include "inc/interval.h"
#include "inc/piechart.h"
#include "inc/piepart.h"

#include <QPainter>

SliceView::SliceView() : _typeOfView(SliceType::CURRENT),
	_movementThresholdMin(100), _movementThresholdMax(200), _useNextSliceInsteadOfCurrentSlice(false),
	_flowAlpha(FLOW_ALPHA_DEFAULT), _flowEpsilon(FLOW_EPSILON_DEFAULT), _flowMaximumIterations(FLOW_MAXIMUM_ITERATIONS),
	_restrictedAreaResolution(100), _restrictedAreaThreshold(-900), _restrictedAreaBeginRadius(5), _typeOfEdgeDetection(EdgeDetectionType::SOBEL),
	_cannyRadiusOfGaussianMask(2), _cannySigmaOfGaussianMask(2), _cannyMinimumGradient(100.), _cannyMinimumDeviation(0.9)
{
}

/*******************************
 * Public setters
 *******************************/

void SliceView::setTypeOfView( const SliceType::SliceType &type )
{
	if ( type > SliceType::_SLICE_TYPE_MIN_ && type < SliceType::_SLICE_TYPE_MAX_ )
	{
		_typeOfView = type;
	}
}

void SliceView::setMovementThresholdMin( const int &threshold )
{
	_movementThresholdMin = threshold;
}

void SliceView::setMovementThresholdMax( const int &threshold )
{
	_movementThresholdMax = threshold;
}

void SliceView::useNextSliceInsteadOfCurrentSlice( const bool &enable )
{
	_useNextSliceInsteadOfCurrentSlice = enable;
}

qreal SliceView::flowAlpha() const
{
	return _flowAlpha;
}

qreal SliceView::flowEpsilon() const
{
	return _flowEpsilon;
}

int SliceView::flowMaximumIterations() const
{
	return _flowMaximumIterations;
}

void SliceView::setFlowAlpha( const qreal &alpha )
{
	_flowAlpha = alpha;
}

void SliceView::setFlowEpsilon( const qreal &epsilon )
{
	_flowEpsilon = epsilon;
}

void SliceView::setFlowMaximumIterations( const int &maxIter )
{
	_flowMaximumIterations = maxIter;
}

void SliceView::setRestrictedAreaResolution( const int &resolution )
{
	_restrictedAreaResolution = resolution;
}

void SliceView::setRestrictedAreaThreshold( const int &threshold )
{
	_restrictedAreaThreshold = threshold;
}

void SliceView::setRestrictedAreaBeginRadius( const int &radius )
{
	_restrictedAreaBeginRadius = radius;
}

void SliceView::setEdgeDetectionType( const EdgeDetectionType::EdgeDetectionType &type )
{
	if ( type > EdgeDetectionType::_EDGE_DETECTION_MIN_ && type < EdgeDetectionType::_EDGE_DETECTION_MAX_ )
	{
		_typeOfEdgeDetection = type;
	}
}

void SliceView::setRadiusOfGaussianMask( const int &radius ) {
	_cannyRadiusOfGaussianMask = radius;
}

void SliceView::setSigmaOfGaussianMask( const qreal &sigma ) {
	_cannySigmaOfGaussianMask = sigma;
}

void SliceView::setCannyMinimumGradient( const qreal &minimumGradient ) {
	_cannyMinimumGradient = minimumGradient;
}

void SliceView::setCannyMinimumDeviation( const qreal &minimumDeviation ) {
	_cannyMinimumDeviation = minimumDeviation;
}

void SliceView::drawSlice( QImage &image, const Billon &billon, const Marrow *marrow, const int &sliceNumber, const Interval &intensityInterval )
{
	if ( sliceNumber > -1 && sliceNumber < static_cast<int>(billon.n_slices) )
	{
		switch (_typeOfView)
		{
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
			// Affichage de la coupe de détection de mouvements
			case SliceType::EDGE_DETECTION :
				drawEdgeDetectionSlice( image, billon, marrow, sliceNumber, intensityInterval );
				break;
			// Affichage de la coupe de flot optique
			case SliceType::FLOW :
				drawFlowSlice( image, billon, sliceNumber );
				break;
			// Affichage de la zone réduite
			case SliceType::RESTRICTED_AREA :
				drawCurrentSlice( image, billon, sliceNumber, intensityInterval );
				drawRestrictedArea( image, billon, marrow, sliceNumber, intensityInterval );
				break;
			// Affichage de la coupe originale
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

void SliceView::drawCurrentSlice( QImage &image, const Billon &billon, const int &sliceNumber, const Interval &intensityInterval )
{
	const Slice &slice = billon.slice(sliceNumber);
	const uint width = slice.n_cols;
	const uint height = slice.n_rows;
	const int minValue = intensityInterval.minValue();
	const int maxValue = intensityInterval.maxValue();
	const qreal fact = 255.0/intensityInterval.size();

	QRgb * line = (QRgb *) image.bits();
	int color;
	uint i,j;

	for ( j=0 ; j<height ; j++)
	{
		for ( i=0 ; i<width ; i++)
		{
			color = (RESTRICT_TO(minValue,slice.at(j,i),maxValue)-minValue)*fact;
			*(line++) = qRgb(color,color,color);
		}
	}
}

void SliceView::drawAverageSlice( QImage &image, const Billon &billon, const Interval &intensityInterval )
{
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = billon.n_slices;
	const int minValue = intensityInterval.minValue();
	const int maxValue = intensityInterval.maxValue();
	const double fact = 255.0/(depth*intensityInterval.size());

	QRgb * line = (QRgb *) image.bits();
	int color;

	for (uint j=0 ; j<height ; j++)
	{
		for (uint i=0 ; i<width ; i++)
		{
			color = depth*(-minValue);
			for (uint k=0 ; k<depth ; k++)
			{
				color += RESTRICT_TO(minValue,billon.at(j,i,k),maxValue);
			}
			color *= fact;
			*(line++) = qRgb(color,color,color);
		}
	}
}

void SliceView::drawMedianSlice( QImage &image, const Billon &billon, const Interval &intensityInterval )
{
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = billon.n_slices;
	const int minValue = intensityInterval.minValue();
	const int maxValue = intensityInterval.maxValue();
	const double fact = 255.0/intensityInterval.size();

	QRgb * line =(QRgb *) image.bits();
	int color;

	for (uint j=0 ; j<height ; j++)
	{
		for (uint i=0 ; i<width ; i++)
		{
			ivec tab(depth);
			for (uint k=0 ; k<depth ; k++)
			{
				tab(k) = RESTRICT_TO(minValue,billon.at(j,i,k),maxValue);
			}
			color = (median(tab)-minValue)*fact;
			*(line++) = qRgb(color,color,color);
		}
	}
}

void SliceView::drawMovementSlice( QImage &image, const Billon &billon, const int &sliceNumber, const Interval &intensityInterval )
{
	const Slice &previousSlice = billon.slice(sliceNumber > 0 ? sliceNumber-1 : sliceNumber+1);
	const Slice &toCompareSlice = billon.slice(_useNextSliceInsteadOfCurrentSlice && sliceNumber < static_cast<int>(billon.n_slices)-1 ? sliceNumber+1 : sliceNumber );
	const uint width = previousSlice.n_cols;
	const uint height = previousSlice.n_rows;
	const int minValue = intensityInterval.minValue();
	const int maxValue = intensityInterval.maxValue();
	const qreal fact = 255./intensityInterval.width();

	QRgb * line = (QRgb *) image.bits();
	int color;
	uint i,j;

	for ( j=0 ; j<height ; j++)
	{
		for ( i=0 ; i<width ; i++)
		{
			color = qAbs(RESTRICT_TO(minValue,previousSlice.at(j,i),maxValue) - RESTRICT_TO(minValue,toCompareSlice.at(j,i),maxValue)) * fact;
//			color += 0.5*color;
//			color = qBound(0,color,255);
			*(line++) = qRgb(color,color,color);
//			if ( qAbs(RESTRICT_TO(minValue,previousSlice.at(j,i),maxValue) - RESTRICT_TO(minValue,toCompareSlice.at(j,i),maxValue)) > 200 ) {
//				*(line++) = qRgb(255,255,255);
//			}
//			else {
//				*(line++) = qRgb(0,0,0);
//			}
		}
	}
}

void SliceView::drawEdgeDetectionSlice( QImage &image, const Billon &billon, const Marrow *marrow, const int &sliceNumber, const Interval &intensityInterval )
{
	const Slice &slice = billon.slice(sliceNumber);
	const int width = slice.n_cols;
	const int height = slice.n_rows;
	const qreal minValue = intensityInterval.minValue();
	const qreal maxValue = intensityInterval.maxValue();

	QRgb * line = (QRgb *) image.bits();
	int color, gNorme;
	int i,j,ki,kj;
	qreal nbValues, fact;

	arma::Mat<qreal> gaussianMask;
	arma::Mat<qreal> gaussianMat(billon.n_rows,billon.n_cols);
	gaussianMat.fill(0);
	arma::Mat<qreal> gradientMat(billon.n_rows,billon.n_cols);
	gradientMat.fill(0);
	arma::Mat<qreal> directionMat(billon.n_rows,billon.n_cols);
	directionMat.fill(0);
	const qreal radius = _cannyRadiusOfGaussianMask;
	const qreal diameter = 2*radius+1;
	const qreal sigma = _cannySigmaOfGaussianMask;
	const qreal sigmaDiv = 2*sigma*sigma;
	qreal gaussianFactor, e, cannyValue, gx, gy, grad, centerX, centerY, distX, distY;
	QPainter painter;

	switch (_typeOfEdgeDetection) {
		case EdgeDetectionType::SOBEL :
			// Gaussian filter construction
			gaussianMask.resize(2*radius+1,2*radius+1);
			gaussianFactor = 0;
			for ( kj=-radius ; kj<=radius ; ++kj ) {
				for ( ki=-radius ; ki<=radius ; ++ki ) {
					e = qExp( -(kj*kj+ki*ki) / sigmaDiv );
					gaussianFactor += e;
					gaussianMask.at(kj+radius,ki+radius) = e;
				}
			}
			for ( kj=0 ; kj<diameter ; ++kj ) {
				for ( ki=0 ; ki<diameter ; ++ki ) {
					gaussianMask.at(kj,ki) /= gaussianFactor;
				}
			}
			// Gaussian filter
			for ( j=radius ; j<height-radius ; j++ )
			{
				for ( i=radius ; i<width-radius ; i++ )
				{
					cannyValue = 0;
					for ( kj=-radius ; kj<radius ; ++kj ) {
						for ( ki=-radius ; ki<radius ; ++ki ) {
							cannyValue += gaussianMask.at(kj+radius,ki+radius)*RESTRICT_TO(minValue,(qreal)slice.at(j+kj,i+ki),maxValue);
						}
					}
					gaussianMat.at(j,i) = cannyValue;
				}
			}
			nbValues = intensityInterval.size();
			fact = 255./nbValues;
			line += width + 1;
			for ( j=1 ; j<height-1 ; j++)
			{
				for ( i=1 ; i<width-1 ; i++)
				{
					gx = gy = RESTRICT_TO(minValue,gaussianMat.at(j-1,i-1),maxValue) - RESTRICT_TO(minValue,gaussianMat.at(j+1,i+1),maxValue);
					gx += 2.*RESTRICT_TO(minValue,gaussianMat.at(j,i-1),maxValue) + RESTRICT_TO(minValue,gaussianMat.at(j+1,i-1),maxValue) - RESTRICT_TO(minValue,gaussianMat.at(j-1,i+1),maxValue) - 2.*RESTRICT_TO(minValue,gaussianMat.at(j,i+1),maxValue);
					gy += 2.*RESTRICT_TO(minValue,gaussianMat.at(j-1,i),maxValue) + RESTRICT_TO(minValue,gaussianMat.at(j-1,i+1),maxValue) - RESTRICT_TO(minValue,gaussianMat.at(j+1,i-1),maxValue) - 2.*RESTRICT_TO(minValue,gaussianMat.at(j+1,i),maxValue);
					gNorme = qSqrt(gx*gx+gy*gy);
					color = qBound(0.,gNorme*fact,255.);
					*(line++) = qRgb(color,color,color);
				}
				line+=2;
			}
			break;
		case EdgeDetectionType::LAPLACIAN :
			nbValues = intensityInterval.size()*8;
			fact = 255./nbValues;
			line += width + 1;
			for ( j=1 ; j<height-1 ; j++)
			{
				for ( i=1 ; i<width-1 ; i++)
				{
					color = qBound(0,
								   static_cast<int>(-6*RESTRICT_TO(minValue,(qreal)slice.at(j,i),maxValue) + RESTRICT_TO(minValue,(qreal)slice.at(j-1,i),maxValue) + RESTRICT_TO(minValue,(qreal)slice.at(j+1,i),maxValue) +
								   RESTRICT_TO(minValue,(qreal)slice.at(j,i-1),maxValue) + RESTRICT_TO(minValue,(qreal)slice.at(j,i+1),maxValue) + 0.5*RESTRICT_TO(minValue,(qreal)slice.at(j+1,i+1),maxValue) +
								   0.5*RESTRICT_TO(minValue,(qreal)slice.at(j+1,i-1),maxValue) + 0.5*RESTRICT_TO(minValue,(qreal)slice.at(j-1,i+1),maxValue) + 0.5*RESTRICT_TO(minValue,(qreal)slice.at(j-1,i-1),maxValue)),
								   255);
					*(line++) = qRgb(color,color,color);
				}
				line+=2;
			}
			break;
		case EdgeDetectionType::CANNY :
			// Gaussian filter construction
			gaussianMask.resize(2*radius+1,2*radius+1);
			gaussianFactor = 0;
			for ( kj=-radius ; kj<=radius ; ++kj ) {
				for ( ki=-radius ; ki<=radius ; ++ki ) {
					e = qExp( -(kj*kj+ki*ki) / sigmaDiv );
					gaussianFactor += e;
					gaussianMask.at(kj+radius,ki+radius) = e;
				}
			}
			for ( kj=0 ; kj<diameter ; ++kj ) {
				for ( ki=0 ; ki<diameter ; ++ki ) {
					gaussianMask.at(kj,ki) /= gaussianFactor;
				}
			}
			// Gaussian filter
			for ( j=radius ; j<height-radius ; j++ )
			{
				for ( i=radius ; i<width-radius ; i++ )
				{
					cannyValue = 0;
					for ( kj=-radius ; kj<radius ; ++kj ) {
						for ( ki=-radius ; ki<radius ; ++ki ) {
							cannyValue += gaussianMask.at(kj+radius,ki+radius)*RESTRICT_TO(minValue,(qreal)slice.at(j+kj,i+ki),maxValue);
						}
					}
					gaussianMat.at(j,i) = cannyValue-minValue;
				}
			}
			// Intensity gradient
			for ( j=radius ; j<height-radius ; j++ )
			{
				for ( i=radius ; i<width-radius ; i++ )
				{
					gx = gaussianMat.at(j,i+1) - gaussianMat.at(j,i-1);
					gy = gaussianMat.at(j-1,i) - gaussianMat.at(j+1,i);
					gradientMat.at(j,i) = qAbs(gx) + qAbs(gy);
					directionMat.at(j,i) = qAtan(gx/gy);
				}
			}
			painter.begin(&image);
			painter.setPen(Qt::white);
			centerX = marrow!=0?marrow->at(sliceNumber).x:width/2.;
			centerY = marrow!=0?marrow->at(sliceNumber).y:height/2.;
			for ( j=radius ; j<height-radius ; j++)
			{
				for ( i=radius ; i<width-radius ; i++)
				{
					grad = gradientMat.at(j,i);
					distX = qCos(directionMat.at(j,i));
					distY = qSin(directionMat.at(j,i));
					if ( grad > _cannyMinimumGradient && qAbs(qSqrt((centerX-i)*(centerX-i) + (centerY-j)*(centerY-j)) - qSqrt((centerX-i-distX)*(centerX-i-distX) + (centerY-j-distY)*(centerY-j-distY))) > _cannyMinimumDeviation ) {
						//painter.drawLine(i,j,i+distX,j+distY);
						painter.drawPoint(i,j);
					}
				}
			}
			break;
		default :
			break;
	}
}

void SliceView::drawFlowSlice( QImage &image, const Billon &billon, const int &sliceNumber )
{

	const Slice &currentSlice = billon.slice(sliceNumber);

	const int width = currentSlice.n_cols;
	const int height = currentSlice.n_rows;
	int i, j;

	VectorsField *field = OpticalFlow::compute(billon,sliceNumber,_flowAlpha,_flowEpsilon,_flowMaximumIterations);

	QRgb * line =(QRgb *) image.bits();
	qreal angle, norme;
	QColor color;

	for ( j=0 ; j<height-1 ; j++)
	{
		for ( i=0 ; i<width-1 ; i++)
		{
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
//	for ( j=5 ; j<height-1 ; j+=5 )
//	{
//		for ( i=5 ; i<width-1 ; i+=5 )
//		{
//			painter.drawLine(i,j,i+(*field)[j][i].x(),j+(*field)[j][i].y());
//		}
//	}

	delete field;
}

void SliceView::drawRestrictedArea( QImage &image, const Billon &billon, const Marrow *marrow, const int &sliceNumber, const Interval &intensityInterval )
{
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

	for ( i=0 ; i<nbPoints ; ++i )
	{
		orientation += (TWO_PI/static_cast<qreal>(nbPoints));
		cosAngle = qCos(orientation);
		sinAngle = -qSin(orientation);
		xEdge = xCenter + _restrictedAreaBeginRadius*cosAngle;
		yEdge = yCenter + _restrictedAreaBeginRadius*sinAngle;
		while ( RESTRICT_TO(min,currentSlice.at(yEdge,xEdge),max) > threshold && xEdge>0 && yEdge>0 && xEdge<imageWidth && yEdge<imageHeight )
		{
			xEdge += cosAngle;
			yEdge += sinAngle;
		}
		polygonPoints[k++] = xEdge;
		polygonPoints[k++] = yEdge;
	}
	polygonPoints[k++] = polygonPoints[0];
	polygonPoints[k] = polygonPoints[1];

	polygon.setPoints(nbPoints+1,polygonPoints);

	QPainter painter(&image);
	painter.setPen(Qt::green);
	painter.drawPolygon(polygon);
}
