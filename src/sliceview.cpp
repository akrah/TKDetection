#include "inc/sliceview.h"

#include "def/def_opticalflow.h"
#include "inc/billon.h"
#include "inc/globalfunctions.h"
#include "inc/slicealgorithm.h"

#include "DGtal/io/colormaps/HueShadeColorMap.h"
#include "DGtal/io/colormaps/GrayscaleColorMap.h"
#include "DGtal/io/colormaps/GradientColorMap.h"
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
			  const uint &zMotionMin, const uint &angularResolution, const TKD::ProjectionType &axe, const TKD::ImageViewRender &imageRender, const qreal &ellipticityRate )
{
	switch (axe)
	{
		case TKD::Z_PROJECTION:
			switch (sliceType)
			{
				// Affichage de la coupe de mouvements
				case TKD::Z_MOTION :
					drawMovementSlice( image, billon, sliceIndex, intensityInterval, zMotionMin, angularResolution, axe );
					break;
				// Affichage de la coupe originale
				case TKD::CLASSIC:
				default :
				  drawCurrentSlice( image, billon, sliceIndex, intensityInterval, angularResolution, imageRender , axe );
					break;
			}
			break;
		case TKD::Y_PROJECTION:
			switch (sliceType)
			{
				// Affichage de la coupe originale
				case TKD::CLASSIC:
				default :
				  drawCurrentSlice( image, billon, sliceIndex, intensityInterval, angularResolution, imageRender, axe );
					break;
			}
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
				  drawCurrentSlice( image, billon, sliceIndex, intensityInterval, angularResolution, imageRender, axe );
					break;
			}
			break;
		case TKD::ELLIPTIC_PROJECTION:
			switch (sliceType)
			{
				// Affichage de la coupe originale
				case TKD::CLASSIC:
				default :
				  drawCurrentSlice( image, billon, sliceIndex, intensityInterval, angularResolution, imageRender, axe, ellipticityRate );
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

void SliceView::drawCurrentSlice( QImage &image, const Billon &billon,
				  const uint &sliceIndex, const Interval<int> &intensityInterval,
				  const uint &angularResolution, const TKD::ImageViewRender &aRender, const TKD::ProjectionType &axe,
				  const qreal &ellipticityRate )
{
	const uint &width = billon.n_cols;
	const uint &height = billon.n_rows;
	const uint &depth = billon.n_slices;
	const int &minIntensity = intensityInterval.min();
	const qreal fact = 255.0/intensityInterval.size();

	static DGtal::HueShadeColorMap<unsigned char> hueShade (0, 255);
	static DGtal::HueShadeColorMap<unsigned char> hueShadeLog (log(1), log(1+255));
	static DGtal::GrayscaleColorMap<unsigned char> grayShade (0, 255);
	DGtal::GradientColorMap<unsigned char> customShade(0,255);
	customShade.addColor( DGtal::Color::Blue );
	customShade.addColor( DGtal::Color::Red );
	customShade.addColor( DGtal::Color::Green );
	customShade.addColor( DGtal::Color::White );

	QRgb * line = (QRgb *) image.bits();
	int color;
	DGtal::Color dgtalColor;
	uint i,j, k;


	/********************************************************************/

//	const Slice &slice = billon.slice(sliceIndex);

//	// Calcul des orientations en chaque pixel avec les filtres de Sobel
//	arma::Mat<qreal> orientations( height, width );
//	arma::Mat<qreal> sobelNorm( height, width );
//	arma::Mat<char> hasContour( height, width );

//	const int semiWidth = qFloor(width/2.);
//	const int semiAdaptativeWidth = qFloor(semiWidth*(sliceIndex/static_cast<qreal>(depth)));
//	const int iMin = qMax(semiWidth-semiAdaptativeWidth+1,0);
//	const int iMax = semiWidth+semiAdaptativeWidth-1;

//	arma::Col<qreal> sobelNormVec(qMax((2*semiAdaptativeWidth-2)*(height-2),0));
//	arma::Col<qreal>::iterator sobelNormVecIt = sobelNormVec.begin();

//	qreal sobelX, sobelY;

//	const qreal &xDim = billon.voxelWidth();
//	const qreal &yDim = billon.voxelHeight();
//	const qreal voxelRatio = qPow(xDim/yDim,2);


//	orientations.fill(0);
//	sobelNorm.fill(0.);
//	for ( j=1 ; j<height-1 ; ++j )
//	{
//		for ( i=iMin ; i<iMax ; ++i )
//		{
//			sobelX = slice.at( j-1, i-1 ) - slice.at( j-1, i+1 ) +
//					 2* (slice.at( j, i-1 ) - slice.at( j, i+1 )) +
//					 slice.at( j+1, i-1 ) - slice.at( j+1, i+1 );
//			sobelY = slice.at( j+1, i-1 ) - slice.at( j-1, i-1 ) +
//					 2 * (slice.at( j+1, i ) - slice.at( j-1, i )) +
//					 slice.at( j+1, i+1 ) - slice.at( j-1, i+1 );
//			orientations.at(j,i) = qFuzzyIsNull(sobelX) ? 9999999999./1. : sobelY/sobelX*voxelRatio;
//			sobelNorm.at(j,i) = qSqrt( qPow(sobelX*xDim,2) + qPow(sobelY*yDim,2) )/4.;
//			*(sobelNormVecIt++) = sobelNorm.at(j,i);
//		}
//	}

//	const arma::Col<qreal> sobelNormSort = arma::sort( sobelNormVec );
//	const qreal &medianVal = sobelNormSort.n_elem?sobelNormSort.at( sobelNormSort.n_elem/2 ):0;

//	uint nbContourPoints = 0;
//	hasContour.fill(false);
//	for ( j=1 ; j<height-1 ; ++j )
//	{
//		for ( i=iMin ; i<iMax ; ++i )
//		{
//			if ( sobelNorm.at(j,i) > medianVal )
//			{
//				nbContourPoints++;
//				hasContour.at(j,i) = true;
//			}
//		}
//	}

//	// Calcul des accumulation des droites suivant les orientations
//	arma::Mat<int> accuSlice( height, width );
//	accuSlice.fill(0);

//	qreal x, y;
//	for ( j=1 ; j<height-1 ; ++j )
//	{
//		for ( i=iMin ; i<iMax ; ++i )
//		{
//			if ( hasContour.at(j,i) )
//			{
//				const int originX = i;
//				const int originY = j;
//				const qreal orientation = -orientations.at(j,i);
//				const qreal orientationInv = 1./orientation;

//				if ( orientation >= 1. )
//				{
//					for ( x = originX , y=originY; x<width && y<height ; x += orientationInv, y += 1. )
//					{
//						accuSlice.at(y,x) += 1;
//					}
//					for ( x = originX-orientationInv , y=originY-1; x>=0. && y>=0. ; x -= orientationInv, y -= 1. )
//					{
//						accuSlice.at(y,x) += 1;
//					}
//				}
//				else if ( orientation > 0. )
//				{
//					for ( x = originX, y=originY ; x<width && y<height ; x += 1., y += orientation )
//					{
//						accuSlice.at(y,x) += 1;
//					}
//					for ( x = originX-1., y=originY-orientation ; x>=0 && y>=0 ; x -= 1., y -= orientation )
//					{
//						accuSlice.at(y,x) += 1;
//					}
//				}
//				else if ( orientation > -1. )
//				{
//					for ( x = originX, y=originY ; x<width && y>=0 ; x += 1., y += orientation )
//					{
//						accuSlice.at(y,x) += 1;
//					}
//					for ( x = originX-1., y=originY-orientation ; x>=0 && y<height ; x -= 1., y -= orientation )
//					{
//						accuSlice.at(y,x) += 1;
//					}
//				}
//				else
//				{
//					for ( x = originX , y=originY; x>=0 && y<height ; x += orientationInv, y += 1. )
//					{
//						accuSlice.at(y,x) += 1;
//					}
//					for ( x = originX-orientationInv , y=originY-1.; x<width && y>=0 ; x -= orientationInv, y -= 1. )
//					{
//						accuSlice.at(y,x) += 1;
//					}
//				}
//			}
//		}
//	}

//	const qreal fact = 255.0/(sobelNormSort.n_elem?sobelNormSort.at((sobelNormSort.n_elem-1)*0.97):1.);


	/********************************************************************/


	if ( axe == TKD::Y_PROJECTION )
	{
		for ( k=0 ; k<depth ; ++k)
		{
			for ( i=0 ; i<width ; ++i)
			{
				color = (TKD::restrictedValue(billon.at(sliceIndex,i,depth-k-1),intensityInterval)-minIntensity)*fact;
				dgtalColor = ((aRender== TKD::HueScale) ? hueShade( color): (aRender==TKD::GrayScale)? grayShade(color): customShade(color));
				*(line++) = qRgb(dgtalColor.red(),dgtalColor.green(),dgtalColor.blue());
			}
		}
	}
	else if ( axe == TKD::Z_PROJECTION )
	{
		const Slice &slice = billon.slice(sliceIndex);
		for ( j=0 ; j<height ; ++j)
		{
			for ( i=0 ; i<width ; ++i)
			{
				color = (TKD::restrictedValue(slice.at(j,i),intensityInterval)-minIntensity)*fact;
				//color = accuSlice.at(j,i)*fact;
				dgtalColor= ((aRender== TKD::HueScale) ? hueShade( color): (aRender==TKD::GrayScale)? grayShade(color): (aRender==TKD::HueScaleLog)? hueShadeLog(log(1+color)):customShade(color));
				*(line++) = qRgb(dgtalColor.red(),dgtalColor.green(),dgtalColor.blue());
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
				color = (TKD::restrictedValue(slice.at(y,x),intensityInterval)-minIntensity)*fact;
				dgtalColor= ((aRender== TKD::HueScale) ? hueShade( color): (aRender==TKD::GrayScale)? grayShade(color): (aRender==TKD::HueScaleLog)? hueShadeLog(log(1+color)):customShade(color));
				*(line++) = qRgb(dgtalColor.red(),dgtalColor.green(),dgtalColor.blue());
			}
		}
	}
	else if ( axe == TKD::ELLIPTIC_PROJECTION )
	{
		const Slice &slice = billon.slice(sliceIndex);
		const rCoord2D &pithCoord = billon.hasPith()?billon.pithCoord(sliceIndex):rCoord2D(width/2,height/2);
		const uint radialResolution = qMin(qMin(pithCoord.x,width-pithCoord.x),qMin(pithCoord.y/ellipticityRate,(height-pithCoord.y)/ellipticityRate));
		const qreal angularIncrement = TWO_PI/(qreal)(angularResolution);

		int x, y;
		for ( j=0 ; j<radialResolution ; ++j)
		{
			for ( i=0 ; i<angularResolution ; ++i )
			{
				x = pithCoord.x + j * qCos(i*angularIncrement);
				y = pithCoord.y + j * qSin(i*angularIncrement) * ellipticityRate;
				color = (TKD::restrictedValue(slice.at(y,x),intensityInterval)-minIntensity)*fact;
				dgtalColor= ((aRender== TKD::HueScale) ? hueShade( color): (aRender==TKD::GrayScale)? grayShade(color): (aRender==TKD::HueScaleLog)? hueShadeLog(log(1+color)):customShade(color));
				*(line++) = qRgb(dgtalColor.red(),dgtalColor.green(),dgtalColor.blue());
			}
		}
	}
}

void SliceView::drawMovementSlice( QImage &image, const Billon &billon, const uint &sliceIndex, const Interval<int> &intensityInterval,
								   const uint &zMotionMin, const uint &angularResolution, const TKD::ProjectionType &axe )
{
	const Slice &currentSlice = billon.slice(sliceIndex);
	const Slice &previousSlice = billon.previousSlice(sliceIndex);

	const uint &width = billon.n_cols;
	const uint &height = billon.n_rows;

	QRgb * line = (QRgb *) image.bits();
	uint color, i, j;
	const QRgb white = qRgb(255,255,255);

	if ( axe == TKD::Z_PROJECTION )
	{
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( intensityInterval.containsClosed(currentSlice.at(j,i)) && intensityInterval.containsClosed(previousSlice.at(j,i)) )
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
	else if ( axe == TKD::POLAR_PROJECTION )
	{
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
				if ( intensityInterval.containsClosed(currentSlice.at(y,x)) && intensityInterval.containsClosed(previousSlice.at(y,x)) )
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
