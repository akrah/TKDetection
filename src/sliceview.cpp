#include "inc/sliceview.h"

#include "def/def_opticalflow.h"
#include "inc/billon.h"
#include "inc/globalfunctions.h"
#include "inc/opticalflow.h"
#include "inc/slicealgorithm.h"

#include <QColor>
#include <QImage>
#include <QPainter>
#include <QVector2D>

SliceView::SliceView()
{
}

/*******************************
 * Public setters
 *******************************/

void SliceView::drawSlice(QImage &image, const Billon &billon, const TKD::SliceType &sliceType, const uiCoord2D &center, const uint &sliceIndex, const Interval<int> &intensityInterval,
						   const Interval<int> &motionInterval, const uint &angularResolution, const TKD::ViewType &axe, const TKD::OpticalFlowParameters &opticalFlowParameters,
						  const TKD::EdgeDetectionParameters &edgeDetectionParameters )
{
	switch (axe)
	{
		case TKD::Z_VIEW:
			switch (sliceType)
			{
				// Affichage de la coupe de mouvements
				case TKD::MOVEMENT :
					drawMovementSlice( image, billon, sliceIndex, intensityInterval, motionInterval.min(), angularResolution, axe );
					break;
					// Affichage de la coupe de détection de mouvements
				case TKD::EDGE_DETECTION :
					drawEdgeDetectionSlice( image, billon, center, sliceIndex, intensityInterval, edgeDetectionParameters );
					break;
					// Affichage de la coupe de flot optique
				case TKD::FLOW :
					drawFlowSlice( image, billon, sliceIndex, opticalFlowParameters );
					break;
					// Affichage de la coupe originale
				case TKD::CURRENT:
				default :
					// Affichage de la coupe courante
					drawCurrentSlice( image, billon, sliceIndex, intensityInterval, angularResolution, axe );
					break;
			}
			break;
		case TKD::Y_VIEW:
		case TKD::CARTESIAN_VIEW:
			switch (sliceType)
			{
				// Affichage de la coupe de mouvements
				case TKD::MOVEMENT :
					drawMovementSlice( image, billon, sliceIndex, intensityInterval, motionInterval.min(), angularResolution, axe );
					break;
					// Affichage de la coupe originale
				case TKD::CURRENT:
				default :
					// Affichage de la coupe courante
					drawCurrentSlice( image, billon, sliceIndex, intensityInterval, angularResolution, axe );
					break;
			}
			break;
		default:
			break;
	}
}


/*******************************
 * Private functions
 *******************************/

void SliceView::drawCurrentSlice( QImage &image, const Billon &billon, const uint &sliceIndex, const Interval<int> &intensityInterval, const uint &angularResolution, const TKD::ViewType &axe )
{
	const Slice &slice = billon.slice(sliceIndex);

	const uint &width = billon.n_cols;
	const uint &height = billon.n_rows;
	const uint &depth = billon.n_slices;
	const int &minIntensity = intensityInterval.min();
	const qreal fact = 255.0/intensityInterval.size();

	QRgb * line = (QRgb *) image.bits();
	int color;
	uint i,j,k;

	if ( axe == TKD::Y_VIEW )
	{
		for ( k=0 ; k<depth ; ++k)
		{
			for ( i=0 ; i<width ; ++i)
			{
				color = (TKD::restrictedValue(billon.at(sliceIndex,i,k),intensityInterval)-minIntensity)*fact;
				*(line++) = qRgb(color,color,color);
			}
		}
	}
	else if ( axe == TKD::Z_VIEW )
	{
		for ( j=0 ; j<height ; ++j)
		{
			for ( i=0 ; i<width ; ++i)
			{
				color = (TKD::restrictedValue(slice.at(j,i),intensityInterval)-minIntensity)*fact;
				*(line++) = qRgb(color,color,color);
			}
		}
	}
	else if ( axe == TKD::CARTESIAN_VIEW )
	{
		const uiCoord2D &pithCoord = billon.hasPith()?billon.pithCoord(sliceIndex):uiCoord2D(width/2,height/2);
		const uint radialResolution = qMin(qMin(pithCoord.x,width-pithCoord.x),qMin(pithCoord.y,height-pithCoord.y));
		const qreal angularIncrement = TWO_PI/(qreal)(angularResolution);

		int x, y;
		for ( j=0 ; j<radialResolution ; ++j)
		{
			for ( i=0 ; i<angularResolution ; ++i )
			{
				x = pithCoord.x + j * qCos(i*angularIncrement);
				y = pithCoord.y + j * qSin(i*angularIncrement);
				color = (TKD::restrictedValue(slice.at(y,x),intensityInterval)-minIntensity)*fact;
				*(line++) = qRgb(color,color,color);
			}
		}
	}
}

void SliceView::drawMovementSlice( QImage &image, const Billon &billon, const uint &sliceIndex, const Interval<int> &intensityInterval,
								   const int &zMotionMin, const uint &angularResolution, const TKD::ViewType &axe )
{
	const Slice &currentSlice = billon.slice(sliceIndex);
	const Slice &previousSlice = billon.previousSlice(sliceIndex);

	const uint &width = billon.n_cols;
	const uint &height = billon.n_rows;
//	const qreal fact = 255./zMotionMin.width();

	QRgb * line = (QRgb *) image.bits();
	int color;
	const QRgb white = qRgb(255,255,255);
	uint i,j,k;

	if ( axe == TKD::Y_VIEW )
	{
		const uint &depth = billon.n_slices;
		for ( k=0 ; k<depth ; ++k )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( intensityInterval.containsClosed(billon.at(sliceIndex,i,k)) && intensityInterval.containsClosed(billon.at(sliceIndex>0 ? sliceIndex-1 : sliceIndex+1,i,k)) )
				{
					color = qAbs( billon.at(sliceIndex,i,k) - billon.at(sliceIndex>0 ? sliceIndex-1 : sliceIndex+1,i,k));
//					if ( motionInterval.containsClosed(color) )
					if ( color > zMotionMin )
					{
//						color *= fact;
//						color = qMin(255.,color*fact);
//						*line = qRgb(color,color,color);
						*line = white;
					}
				}
				++line;
			}
		}
	}
	else if ( axe == TKD::Z_VIEW )
	{
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( intensityInterval.containsClosed(currentSlice.at(j,i)) && intensityInterval.containsClosed(previousSlice.at(j,i)) )
				{
					color = billon.zMotion(i,j,sliceIndex);
//					if ( motionInterval.containsClosed(color) )
					if ( color > zMotionMin )
					{
//						color *= fact;
//						color = qMin(255.,color*fact);
//						*line = qRgb(color,color,color);
						*line = white;
					}
				}
				++line;
			}
		}
	}
	else if ( axe == TKD::CARTESIAN_VIEW )
	{
		const uiCoord2D &pithCoord = billon.hasPith()?billon.pithCoord(sliceIndex):uiCoord2D(width/2,height/2);
		const uint radialResolution = qMin(qMin(pithCoord.x,width-pithCoord.x),qMin(pithCoord.y,height-pithCoord.y));
		const qreal angularIncrement = TWO_PI/(qreal)(angularResolution);

		int x, y;
		for ( j=0 ; j<radialResolution ; ++j)
		{
			for ( i=0 ; i<angularResolution ; ++i )
			{
				x = pithCoord.x + j * qCos(i*angularIncrement);
				y = pithCoord.y + j * qSin(i*angularIncrement);
				if ( intensityInterval.containsClosed(currentSlice.at(y,x)) && intensityInterval.containsClosed(previousSlice.at(y,x)) )
				{
					color = billon.zMotion(x,y,sliceIndex);
//					if ( motionInterval.containsClosed(color) )
					if ( color > zMotionMin )
					{
//						color *= fact;
//						color = qMin(255.,color*fact);
						*line = qRgb(color,color,color);
						*line = white;
					}
				}
				++line;
			}
		}
	}
}

void SliceView::drawEdgeDetectionSlice(QImage &image, const Billon &billon, const iCoord2D &center, const uint &sliceIndex,
									   const Interval<int> &intensityInterval , const TKD::EdgeDetectionParameters &edgeDetectionParameters)
{
	const Slice &slice = billon.slice(sliceIndex);
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
	const qreal radius = edgeDetectionParameters.radiusOfGaussianMask;
	const qreal diameter = 2*radius+1;
	const qreal sigma = edgeDetectionParameters.sigmaOfGaussianMask;
	const qreal sigmaDiv = 2*sigma*sigma;
	qreal gaussianFactor, e, cannyValue, gx, gy, grad, distX, distY;
	QPainter painter;

	switch (edgeDetectionParameters.type) {
		case TKD::SOBEL :
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
					gx += 2.*qMax(qMin(gaussianMat.at(j,i-1),maxValue),minValue) + qMax(qMin(gaussianMat.at(j+1,i-1),maxValue),minValue)
						  - qMax(qMin(gaussianMat.at(j-1,i+1),maxValue),minValue) - 2.*qMax(qMin(gaussianMat.at(j,i+1),maxValue),minValue);
					gy += 2.*qMax(qMin(gaussianMat.at(j-1,i),maxValue),minValue) + qMax(qMin(gaussianMat.at(j-1,i+1),maxValue),minValue)
						  - qMax(qMin(gaussianMat.at(j+1,i-1),maxValue),minValue) - 2.*qMax(qMin(gaussianMat.at(j+1,i),maxValue),minValue);
					gNorme = qSqrt(gx*gx+gy*gy);
					color = qBound(0.,gNorme*fact,255.);
					*(line++) = qRgb(color,color,color);
				}
				line+=2;
			}
			break;
		case TKD::LAPLACIAN :
			nbValues = intensityInterval.size()*8;
			fact = 255./nbValues;
			line += width + 1;
			for ( j=1 ; j<height-1 ; j++)
			{
				for ( i=1 ; i<width-1 ; i++)
				{
					color = qBound(0,
								   static_cast<int>(-6*qMax(qMin(slice.at(j,i),maxValue),minValue) + qMax(qMin(slice.at(j-1,i),maxValue),minValue)
													+ qMax(qMin(slice.at(j+1,i),maxValue),minValue) +
								   qMax(qMin(slice.at(j,i-1),maxValue),minValue) + qMax(qMin(slice.at(j,i+1),maxValue),minValue) + 0.5*qMax(qMin(slice.at(j+1,i+1),maxValue),minValue) +
								   0.5*qMax(qMin(slice.at(j+1,i-1),maxValue),minValue) + 0.5*qMax(qMin(slice.at(j-1,i+1),maxValue),minValue)
													+ 0.5*qMax(qMin(slice.at(j-1,i-1),maxValue),minValue)),
								   255);
					*(line++) = qRgb(color,color,color);
				}
				line+=2;
			}
			break;
		case TKD::CANNY :
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
					if ( grad > edgeDetectionParameters.minimumGradient &&
						 qAbs(qSqrt((center.x-i)*(center.x-i) + (center.y-j)*(center.y-j)) -
							  qSqrt((center.x-i-distX)*(center.x-i-distX) + (center.y-j-distY)*(center.y-j-distY))) > edgeDetectionParameters.minimumDeviation ) {
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

void SliceView::drawFlowSlice( QImage &image, const Billon &billon, const uint &sliceIndex, const TKD::OpticalFlowParameters &opticalFlowParameters )
{
	VectorsField *field = OpticalFlow::compute(billon,sliceIndex,opticalFlowParameters.alpha,opticalFlowParameters.epsilon,opticalFlowParameters.maximumIterations);

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
