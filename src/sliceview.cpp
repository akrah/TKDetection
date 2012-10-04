#include "inc/sliceview.h"

#include "def/def_opticalflow.h"
#include "inc/billon.h"
#include "inc/opticalflow.h"

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QVector2D>

SliceView::SliceView() : _typeOfView(CURRENT),
	_flowAlpha(FLOW_ALPHA_DEFAULT), _flowEpsilon(FLOW_EPSILON_DEFAULT), _flowMaximumIterations(FLOW_MAXIMUM_ITERATIONS),
	_restrictedAreaResolution(100), _restrictedAreaThreshold(-900), _restrictedAreaBeginRadius(5), _typeOfEdgeDetection(SOBEL),
	_cannyRadiusOfGaussianMask(2), _cannySigmaOfGaussianMask(2), _cannyMinimumGradient(100.), _cannyMinimumDeviation(0.9)
{
}

/*******************************
 * Public setters
 *******************************/

void SliceView::setTypeOfView( const SliceType &type )
{
	if ( type > _SLICE_TYPE_MIN_ && type < _SLICE_TYPE_MAX_ )
	{
		_typeOfView = type;
	}
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

void SliceView::setEdgeDetectionType( const EdgeDetectionType &type )
{
	if ( type > _EDGE_DETECTION_MIN_ && type < _EDGE_DETECTION_MAX_ )
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

void SliceView::drawSlice( QImage &image, const Billon &billon, const iCoord2D &center, const int &sliceIndex, const Interval<int> &intensityInterval, const Interval<int> &motionInterval )
{
	if ( sliceIndex > -1 && sliceIndex < static_cast<int>(billon.n_slices) )
	{
		switch (_typeOfView)
		{
			// Affichage de la coupe de mouvements
			case MOVEMENT :
				drawMovementSlice( image, billon, sliceIndex, intensityInterval, motionInterval );
				break;
			// Affichage de la coupe de détection de mouvements
			case EDGE_DETECTION :
				drawEdgeDetectionSlice( image, billon, center, sliceIndex, intensityInterval );
				break;
			// Affichage de la coupe de flot optique
			case FLOW :
				drawFlowSlice( image, billon, sliceIndex );
				break;
			// Affichage de la zone réduite
			case RESTRICTED_AREA :
				drawCurrentSlice( image, billon, sliceIndex, intensityInterval );
				drawRestrictedArea( image, billon, center, sliceIndex, intensityInterval.min() );
				break;
			// Affichage de la coupe originale
			case CURRENT:
			default :
				// Affichage de la coupe courante
				drawCurrentSlice( image, billon, sliceIndex, intensityInterval );
				break;
		}
	}
}


/*******************************
 * Private functions
 *******************************/

void SliceView::drawCurrentSlice( QImage &image, const Billon &billon, const int &sliceIndex, const Interval<int> &intensityInterval )
{
	const Slice &slice = billon.slice(sliceIndex);
	const uint width = slice.n_cols;
	const uint height = slice.n_rows;
	const int minValue = intensityInterval.min();
	const int maxValue = intensityInterval.max();
	const qreal fact = 255.0/intensityInterval.size();

	QRgb * line = (QRgb *) image.bits();
	int color;
	uint i,j;

	for ( j=0 ; j<height ; j++)
	{
		for ( i=0 ; i<width ; i++)
		{
			color = (qMax(qMin(slice.at(j,i),maxValue),minValue)-minValue)*fact;
			*(line++) = qRgb(color,color,color);
		}
	}
}

void SliceView::drawMovementSlice( QImage &image, const Billon &billon, const int &sliceIndex, const Interval<int> &intensityInterval, const Interval<int> &motionInterval )
{
	const Slice &currentSlice = billon.slice(sliceIndex);
	const Slice &previousSlice = billon.previousSlice(sliceIndex);
	const uint width = previousSlice.n_cols;
	const uint height = previousSlice.n_rows;
	const qreal fact = 255./motionInterval.width();

	image.fill(0xff000000);
	QRgb * line = (QRgb *) image.bits();
	int color;
	uint i,j;

	for ( j=0 ; j<height ; j++)
	{
		for ( i=0 ; i<width ; i++)
		{
			if ( intensityInterval.containsClosed(currentSlice.at(j,i)) && intensityInterval.containsClosed(previousSlice.at(j,i)) )
			{
				color = qAbs(currentSlice.at(j,i) - previousSlice.at(j,i));
				if ( motionInterval.containsClosed(color) )
				{
					color *= fact;
					*line = qRgb(color,color,color);
				}
			}
			++line;
		}
	}
}

void SliceView::drawEdgeDetectionSlice( QImage &image, const Billon &billon, const iCoord2D &center, const int &sliceNumber, const Interval<int> &intensityInterval )
{
	const Slice &slice = billon.slice(sliceNumber);
	const int width = slice.n_cols;
	const int height = slice.n_rows;
	const int minValue = intensityInterval.min();
	const int maxValue = intensityInterval.max();

	QRgb * line = (QRgb *) image.bits();
	int color, gNorme;
	int i,j,ki,kj;
	qreal nbValues, fact;

	arma::Mat<qreal> gaussianMask;
	arma::Mat<int> gaussianMat(billon.n_rows,billon.n_cols);
	gaussianMat.fill(0);
	arma::Mat<qreal> gradientMat(billon.n_rows,billon.n_cols);
	gradientMat.fill(0);
	arma::Mat<qreal> directionMat(billon.n_rows,billon.n_cols);
	directionMat.fill(0);
	const qreal radius = _cannyRadiusOfGaussianMask;
	const qreal diameter = 2*radius+1;
	const qreal sigma = _cannySigmaOfGaussianMask;
	const qreal sigmaDiv = 2*sigma*sigma;
	qreal gaussianFactor, e, cannyValue, gx, gy, grad, distX, distY;
	QPainter painter;

	switch (_typeOfEdgeDetection) {
		case SOBEL :
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
					for ( kj=-radius ; kj<radius ; ++kj )
					{
						for ( ki=-radius ; ki<radius ; ++ki )
						{
							if ( intensityInterval.containsClosed(slice.at(j+kj,i+ki)) )
							{
								cannyValue += gaussianMask.at(kj+radius,ki+radius)*slice.at(j+kj,i+ki);
							}
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
					gx = gy = qMax(qMin(gaussianMat.at(j-1,i-1),maxValue),minValue) - qMax(qMin(gaussianMat.at(j+1,i+1),maxValue),minValue);
					gx += 2.*qMax(qMin(gaussianMat.at(j,i-1),maxValue),minValue) + qMax(qMin(gaussianMat.at(j+1,i-1),maxValue),minValue) - qMax(qMin(gaussianMat.at(j-1,i+1),maxValue),minValue) - 2.*qMax(qMin(gaussianMat.at(j,i+1),maxValue),minValue);
					gy += 2.*qMax(qMin(gaussianMat.at(j-1,i),maxValue),minValue) + qMax(qMin(gaussianMat.at(j-1,i+1),maxValue),minValue) - qMax(qMin(gaussianMat.at(j+1,i-1),maxValue),minValue) - 2.*qMax(qMin(gaussianMat.at(j+1,i),maxValue),minValue);
					gNorme = qSqrt(gx*gx+gy*gy);
					color = qBound(0.,gNorme*fact,255.);
					*(line++) = qRgb(color,color,color);
				}
				line+=2;
			}
			break;
		case LAPLACIAN :
			nbValues = intensityInterval.size()*8;
			fact = 255./nbValues;
			line += width + 1;
			for ( j=1 ; j<height-1 ; j++)
			{
				for ( i=1 ; i<width-1 ; i++)
				{
					color = qBound(0,
								   static_cast<int>(-6*qMax(qMin(slice.at(j,i),maxValue),minValue) + qMax(qMin(slice.at(j-1,i),maxValue),minValue) + qMax(qMin(slice.at(j+1,i),maxValue),minValue) +
								   qMax(qMin(slice.at(j,i-1),maxValue),minValue) + qMax(qMin(slice.at(j,i+1),maxValue),minValue) + 0.5*qMax(qMin(slice.at(j+1,i+1),maxValue),minValue) +
								   0.5*qMax(qMin(slice.at(j+1,i-1),maxValue),minValue) + 0.5*qMax(qMin(slice.at(j-1,i+1),maxValue),minValue) + 0.5*qMax(qMin(slice.at(j-1,i-1),maxValue),minValue)),
								   255);
					*(line++) = qRgb(color,color,color);
				}
				line+=2;
			}
			break;
		case CANNY :
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
							cannyValue += gaussianMask.at(kj+radius,ki+radius)*qMax(qMin(slice.at(j+kj,i+ki),maxValue),minValue);
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
			for ( j=radius ; j<height-radius ; j++)
			{
				for ( i=radius ; i<width-radius ; i++)
				{
					grad = gradientMat.at(j,i);
					distX = qCos(directionMat.at(j,i));
					distY = qSin(directionMat.at(j,i));
					if ( grad > _cannyMinimumGradient && qAbs(qSqrt((center.x-i)*(center.x-i) + (center.y-j)*(center.y-j)) - qSqrt((center.x-i-distX)*(center.x-i-distX) + (center.y-j-distY)*(center.y-j-distY))) > _cannyMinimumDeviation ) {
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

	VectorsField *field = OpticalFlow::compute(billon,sliceNumber,_flowAlpha,_flowEpsilon,_flowMaximumIterations);

	QRgb * line =(QRgb *) image.bits();
	qreal angle, norme;
	QColor color;
	rCoord2D origin(0,0);
	rVec2D currentCoord;

	QVector< QVector< QVector2D > >::const_iterator iterLine;
	QVector< QVector2D >::const_iterator iterCol;
	for ( iterLine = (*field).constBegin() ; iterLine != (*field).constEnd() ; ++iterLine )
	{
		for ( iterCol = (*iterLine).constBegin() ; iterCol != (*iterLine).constEnd() ; ++iterCol )
		{
			currentCoord = rCoord2D( (*iterCol).x(), (*iterCol).y() );
			angle = (TWO_PI-origin.angle(currentCoord))*RAD_TO_DEG_FACT;
			while (angle>360.) angle -= 360.;
			norme = qMin(currentCoord.norm()*20.,255.);
			color.setHsv(angle,norme,norme);
			*(line++) = color.rgb();
		}
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

void SliceView::drawRestrictedArea( QImage &image, const Billon &billon, const iCoord2D &center, const int &sliceIndex, const int &intensityThreshold )
{
	const Slice &currentSlice = billon.slice(sliceIndex);
	const int width = billon.n_cols;
	const int height = billon.n_rows;

	QPolygon polygon(_restrictedAreaResolution);
	int polygonPoints[2*_restrictedAreaResolution+2];

	rCoord2D edge, rCenter;
	rVec2D direction;
	qreal orientation;
	int k;
	orientation = 0.;
	k = 0;
	rCenter.x = center.x;
	rCenter.y = center.y;
	while (orientation < TWO_PI)
	{
		orientation += (TWO_PI/static_cast<qreal>(_restrictedAreaResolution));
		direction = rVec2D(qCos(orientation),-qSin(orientation));
		edge = rCenter + direction*_restrictedAreaBeginRadius;
		while ( edge.x>0 && edge.y>0 && edge.x<width && edge.y<height && currentSlice.at(edge.y,edge.x) > intensityThreshold )
		{
			edge += direction;
		}
		polygonPoints[k++] = edge.x;
		polygonPoints[k++] = edge.y;
	}
	polygonPoints[k++] = polygonPoints[0];
	polygonPoints[k] = polygonPoints[1];

	polygon.setPoints(_restrictedAreaResolution+1,polygonPoints);

	QPainter painter(&image);
	painter.setPen(Qt::green);
	painter.drawPolygon(polygon);
}
