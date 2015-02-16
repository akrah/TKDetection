#include "inc/sliceview.h"

#include "def/def_opticalflow.h"
#include "inc/billon.h"
#include "inc/globalfunctions.h"
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

void SliceView::drawSlice(QImage &image, const Billon &billon, const TKD::ViewType &sliceType, const uint &sliceIndex, const Interval<int> &intensityInterval,
						  const uint &zMotionMin, const uint &angularResolution, const TKD::ProjectionType &axe, const qreal &ellipticityRate )
{
	switch (axe)
	{
		case TKD::Z_PROJECTION:
			switch (sliceType)
			{
				case TKD::HOUGH :
					drawHoughSlice( image, billon, sliceIndex, axe );
					break;
				// Affichage de la coupe de mouvements
				case TKD::Z_MOTION :
					drawMovementSlice( image, billon, sliceIndex, intensityInterval, zMotionMin, angularResolution, axe );
					break;
				// Affichage de la coupe originale
				case TKD::CLASSIC:
				default :
					drawCurrentSlice( image, billon, sliceIndex, intensityInterval, angularResolution, axe );
					break;
			}
			break;
		case TKD::Y_PROJECTION:
			switch (sliceType)
			{
				// Affichage de la coupe de mouvements
				case TKD::Z_MOTION :
					drawMovementSlice( image, billon, sliceIndex, intensityInterval, zMotionMin, angularResolution, axe );
					break;
				// Affichage de la coupe originale
				case TKD::CLASSIC:
				default :
					drawCurrentSlice( image, billon, sliceIndex, intensityInterval, angularResolution, axe );
					break;
			}
			break;
		case TKD::POLAR_PROJECTION:
			switch (sliceType)
			{
				// Affichage de la coupe de mouvements
				case TKD::Z_MOTION :
					drawMovementSlice( image, billon, sliceIndex, intensityInterval, zMotionMin, angularResolution, axe );
					break;
				// Affichage de la coupe originale
				case TKD::CLASSIC:
				default :
					drawCurrentSlice( image, billon, sliceIndex, intensityInterval, angularResolution, axe );
					break;
			}
			break;
		case TKD::CYLINDRIC_PROJECTION:
			switch (sliceType)
			{
				// Affichage de la coupe originale
				case TKD::CLASSIC:
				default :
					drawCurrentSlice( image, billon, sliceIndex, intensityInterval, angularResolution, axe );
					break;
			}
			break;
		case TKD::ELLIPTIC_PROJECTION:
			switch (sliceType)
			{
				// Affichage de la coupe originale
				case TKD::CLASSIC:
				default :
					drawCurrentSlice( image, billon, sliceIndex, intensityInterval, angularResolution, axe, ellipticityRate );
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

void SliceView::drawCurrentSlice(QImage &image, const Billon &billon, const uint &sliceIndex, const Interval<int> &intensityInterval,
								  const uint &angularResolution, const TKD::ProjectionType &axe, const qreal &ellipticityRate )
{
	const uint &width = billon.n_cols;
	const uint &height = billon.n_rows;
	const uint &depth = billon.n_slices;
	const int &minIntensity = intensityInterval.min();
	const qreal fact = 255.0/intensityInterval.size();

	QRgb * line = (QRgb *) image.bits();
	int color;
	uint i,j, k;


	/********************************************************************/


	if ( axe == TKD::Z_PROJECTION )
	{
		const Slice &slice = billon.slice(sliceIndex);
		for ( j=0 ; j<height ; ++j)
		{
			for ( i=0 ; i<width ; ++i)
			{
				color = (TKD::restrictedValue(slice(j,i),intensityInterval)-minIntensity)*fact;
				//color = accuSlice(j,i)*fact;
				*(line++) = qRgb(color,color,color);
			}
		}
	}
	else if ( axe == TKD::Y_PROJECTION )
	{
		for ( k=0 ; k<depth ; ++k)
		{
			for ( i=0 ; i<width ; ++i)
			{
				color = (TKD::restrictedValue(billon(sliceIndex,i,depth-k-1),intensityInterval)-minIntensity)*fact;
				*(line++) = qRgb(color,color,color);
			}
		}
	}
	else if ( axe == TKD::POLAR_PROJECTION )
	{
		const Slice &slice = billon.slice(sliceIndex);
		const rCoord2D &pithCoord = billon.hasPith()?billon.pithCoord(sliceIndex):rCoord2D(width/2,height/2);
		const uint radialResolution = qMin(qMin(pithCoord.x,width-pithCoord.x),qMin(pithCoord.y,height-pithCoord.y));
		const qreal angularIncrement = TWO_PI/(qreal)(angularResolution);

		int x, y;
		for ( j=0 ; j<radialResolution ; ++j)
		{
			for ( i=0 ; i<angularResolution ; ++i )
			{
				x = qRound(pithCoord.x + j * qCos(i*angularIncrement));
				y = qRound(pithCoord.y + j * qSin(i*angularIncrement));
				color = (TKD::restrictedValue(slice(y,x),intensityInterval)-minIntensity)*fact;
				*(line++) = qRgb(color,color,color);
			}
		}
	}
	else if ( axe == TKD::ELLIPTIC_PROJECTION )
	{
		const Slice &slice = billon.slice(sliceIndex);
		const rCoord2D &pithCoord = billon.hasPith()?billon.pithCoord(sliceIndex):rCoord2D(width/2,height/2);
		const uint radialResolution = qMin(qMin(pithCoord.x,width-pithCoord.x),qMin(pithCoord.y,height-pithCoord.y)/ellipticityRate);
		const qreal angularIncrement = TWO_PI/(qreal)(angularResolution);

		int x, y;
		for ( j=0 ; j<radialResolution ; ++j)
		{
			for ( i=0 ; i<angularResolution ; ++i )
			{
				x = qRound(pithCoord.x + j * qCos(i*angularIncrement));
				y = qRound(pithCoord.y + j * qSin(i*angularIncrement) * ellipticityRate);
				color = (TKD::restrictedValue(slice(y,x),intensityInterval)-minIntensity)*fact;
				*(line++) = qRgb(color,color,color);
			}
		}
	}
	else if ( axe == TKD::CYLINDRIC_PROJECTION )
	{
		const qreal angularIncrement = TWO_PI/(qreal)(angularResolution);
		const qreal radius = qMax(5,qMin(200,(int)sliceIndex));

		rCoord2D center, edge;
		rVec2D direction;
		int nbCircularPoints = 0;

		for ( k=0 ; k<depth ; ++k)
		{
			const Slice &currentSlice = billon.slice(k);
			center = billon.hasPith()?billon.pithCoord(k):rCoord2D(width/2,height/2);
			nbCircularPoints = 0;
			do
			{
				direction = rVec2D(qCos(nbCircularPoints*angularIncrement),qSin(nbCircularPoints*angularIncrement));
				edge = center + direction*radius;
				color = (TKD::restrictedValue(currentSlice(edge.x,edge.y),intensityInterval)-minIntensity)*fact;
				*(line++) = qRgb(color,color,color);
			}
			while (nbCircularPoints++ < angularResolution);
		}
	}
}

void SliceView::drawMovementSlice( QImage &image, const Billon &billon, const uint &sliceIndex, const Interval<int> &intensityInterval,
								   const uint &zMotionMin, const uint &angularResolution, const TKD::ProjectionType &axe )
{
	const uint &width = billon.n_cols;
	const uint &height = billon.n_rows;
	const uint &depth = billon.n_slices;

	QRgb * line = (QRgb *) image.bits();
	uint color, i, j, k;
	const QRgb white = qRgb(255,255,255);

	if ( axe == TKD::Z_PROJECTION )
	{
		const Slice &currentSlice = billon.slice(sliceIndex);
		const Slice &previousSlice = billon.previousSlice(sliceIndex);
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( intensityInterval.containsClosed(currentSlice(j,i)) && intensityInterval.containsClosed(previousSlice(j,i)) )
				{
					color = billon.zMotion(i,j,sliceIndex);
					if ( color > zMotionMin )
					{
						*line = white;
					}
				}
				++line;
			}
		}
	}
	else if ( axe == TKD::Y_PROJECTION )
	{
		for ( k=0 ; k<depth ; ++k )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( intensityInterval.containsClosed(billon(sliceIndex,i,depth-k-1)) && intensityInterval.containsClosed(billon.previousSlice(depth-k-1)(sliceIndex,i)) )
				{
					color = billon.zMotion(i,sliceIndex,depth-k-1);
					if ( color > zMotionMin )
					{
						*line = white;
					}
				}
				++line;
			}
		}
	}
	else if ( axe == TKD::POLAR_PROJECTION )
	{
		const Slice &currentSlice = billon.slice(sliceIndex);
		const Slice &previousSlice = billon.previousSlice(sliceIndex);
		const rCoord2D &pithCoord = billon.hasPith()?billon.pithCoord(sliceIndex):rCoord2D(width/2,height/2);
		const uint radialResolution = qMin(qMin(pithCoord.x,width-pithCoord.x),qMin(pithCoord.y,height-pithCoord.y));
		const qreal angularIncrement = TWO_PI/(qreal)(angularResolution);

		int x, y;
		for ( j=0 ; j<radialResolution ; ++j)
		{
			for ( i=0 ; i<angularResolution ; ++i )
			{
				x = pithCoord.x + j * qCos(i*angularIncrement);
				y = pithCoord.y + j * qSin(i*angularIncrement);
				if ( intensityInterval.containsClosed(currentSlice(y,x)) && intensityInterval.containsClosed(previousSlice(y,x)) )
				{
					color = billon.zMotion(x,y,sliceIndex);
					if ( color > zMotionMin )
					{
						*line = white;
					}
				}
				++line;
			}
		}
	}
}

void SliceView::drawHoughSlice( QImage &image, const Billon &billon, const uint &sliceIndex, const TKD::ProjectionType &axe )
{
	if ( axe != TKD::Z_PROJECTION ) return;

	const int &width = billon.n_cols;
	const int &height = billon.n_rows;
	const int &depth = billon.n_slices;

	QRgb * line = (QRgb *) image.bits();
	int color;
	int i,j;


	/********************************************************************/

	const Slice &slice = billon.slice(sliceIndex);

	// Calcul des orientations en chaque pixel avec les filtres de Sobel
	arma::Mat<qreal> orientations( height, width );
	arma::Mat<qreal> sobelNorm( height, width );

	const int semiWidth = qFloor(width/2.);
	const int semiAdaptativeWidth = qFloor(semiWidth*(sliceIndex/static_cast<qreal>(depth)));
	const int iMin = qMax(semiWidth-semiAdaptativeWidth+1,0);
	const int iMax = semiWidth+semiAdaptativeWidth-1;

	if ( semiAdaptativeWidth<3 || height<3 ) return;

	arma::Col<qreal> sobelNormVec(qMax((2*semiAdaptativeWidth-2)*(height-2),0));
	arma::Col<qreal>::iterator sobelNormVecIt = sobelNormVec.begin();

	qreal sobelX, sobelY, norm;

	const qreal &xDim = billon.voxelWidth();
	const qreal &yDim = billon.voxelHeight();
	const qreal voxelRatio = qPow(xDim/yDim,2);

	int nbNegativeNorm;

	orientations.fill(0);
	sobelNorm.fill(0.);
	nbNegativeNorm = 0;
	if (billon.hasPith())
	{
		for ( j=1 ; j<height-1 ; ++j )
		{
			for ( i=iMin ; i<iMax ; ++i )
			{
				if ( j>=qMax(1.,billon.pithCoord(sliceIndex).y-30) && j<qMin(height-1.,billon.pithCoord(sliceIndex).y+30) &&
					 i>=qMax((double)iMin,billon.pithCoord(sliceIndex).x-30) && i<qMin((double)iMax,billon.pithCoord(sliceIndex).x+30) )
				{
					sobelX = slice( j-1, i-1 ) - slice( j-1, i+1 ) +
							 2* (slice( j, i-1 ) - slice( j, i+1 )) +
							 slice( j+1, i-1 ) - slice( j+1, i+1 );
					sobelY = slice( j+1, i-1 ) - slice( j-1, i-1 ) +
							 2 * (slice( j+1, i ) - slice( j-1, i )) +
							 slice( j+1, i+1 ) - slice( j-1, i+1 );
					orientations(j,i) = qFuzzyIsNull(sobelX) ? 9999999999./1. : sobelY/sobelX*voxelRatio;
					norm = qPow(sobelX,2) + qPow(sobelY,2);
					*sobelNormVecIt++ = norm;
					nbNegativeNorm += qFuzzyIsNull(norm);
				}
				else
				{
					orientations(j,i) = 0.;
					*sobelNormVecIt++ = 0.;
					nbNegativeNorm++;
				}
			}
		}
	}
	else
	{
		for ( j=1 ; j<height-1 ; ++j )
		{
			for ( i=iMin ; i<iMax ; ++i )
			{
				sobelX = slice( j-1, i-1 ) - slice( j-1, i+1 ) +
						 2* (slice( j, i-1 ) - slice( j, i+1 )) +
						 slice( j+1, i-1 ) - slice( j+1, i+1 );
				sobelY = slice( j+1, i-1 ) - slice( j-1, i-1 ) +
						 2 * (slice( j+1, i ) - slice( j-1, i )) +
						 slice( j+1, i+1 ) - slice( j-1, i+1 );
				orientations(j,i) = qFuzzyIsNull(sobelX) ? 9999999999./1. : sobelY/sobelX*voxelRatio;
				norm = qPow(sobelX,2) + qPow(sobelY,2);
				*sobelNormVecIt++ = norm;
				nbNegativeNorm += qFuzzyIsNull(norm);
			}
		}
	}

	const arma::Col<qreal> sobelNormSort = arma::sort( sobelNormVec );
	const qreal &medianVal = sobelNormSort( (sobelNormSort.n_elem + nbNegativeNorm)*0.4 );

	// Calcul des accumulation des droites suivant les orientations
	arma::Mat<int> accuSlice( height, width );
	accuSlice.fill(0);

	qreal x, y, orientation, orientationInv;
	sobelNormVecIt = sobelNormVec.begin();

	for ( j=1 ; j<height-1 ; ++j )
	{
		for ( i=iMin ; i<iMax ; ++i )
		{
			if ( *sobelNormVecIt++ > medianVal )
			{
				orientation = -orientations(j,i);
				orientationInv = 1./orientation;

				if ( orientation >= 1. )
				{
					for ( x = i , y=j; x<width && y<height ; x += orientationInv, y += 1. )
					{
						accuSlice(y,x) += 1;
					}
					for ( x = i-orientationInv , y=j-1; x>=0. && y>=0. ; x -= orientationInv, y -= 1. )
					{
						accuSlice(y,x) += 1;
					}
				}
				else if ( orientation > 0. )
				{
					for ( x = i, y=j ; x<width && y<height ; x += 1., y += orientation )
					{
						accuSlice(y,x) += 1;
					}
					for ( x = i-1., y=j-orientation ; x>=0 && y>=0 ; x -= 1., y -= orientation )
					{
						accuSlice(y,x) += 1;
					}
				}
				else if ( orientation > -1. )
				{
					for ( x = i, y=j ; x<width && y>=0 ; x += 1., y += orientation )
					{
						accuSlice(y,x) += 1;
					}
					for ( x = i-1., y=j-orientation ; x>=0 && y<height ; x -= 1., y -= orientation )
					{
						accuSlice(y,x) += 1;
					}
				}
				else
				{
					for ( x = i , y=j; x>=0 && y<height ; x += orientationInv, y += 1. )
					{
						accuSlice(y,x) += 1;
					}
					for ( x = i-orientationInv , y=j-1.; x<width && y>=0 ; x -= orientationInv, y -= 1. )
					{
						accuSlice(y,x) += 1;
					}
				}
			}
		}
	}



	const qreal fact = 255.0/(sobelNormSort.n_elem?accuSlice.max():1.);

	/********************************************************************/

	for ( j=0 ; j<height ; ++j)
	{
		for ( i=0 ; i<width ; ++i)
		{
			color = accuSlice(j,i)*fact;
			*(line++) = qRgb(color,color,color);
		}
	}
}
