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
			  const uint &zMotionMin, const uint &angularResolution, const TKD::ProjectionType &axe, const TKD::ImageViewRender &imageRender )
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
				  drawCurrentSlice( image, billon, sliceIndex, intensityInterval, angularResolution, imageRender, axe  );
					break;
			}
		case TKD::CARTESIAN_PROJECTION:
			switch (sliceType)
			{
				// Affichage de la coupe de mouvements
				case TKD::Z_MOTION :
					drawMovementSlice( image, billon, sliceIndex, intensityInterval, zMotionMin, angularResolution, axe );
					break;
				// Affichage de la coupe originale
				case TKD::CLASSIC:
				default :
				  drawCurrentSlice( image, billon, sliceIndex, intensityInterval, angularResolution, imageRender, axe  );
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
				  const uint &angularResolution, const TKD::ImageViewRender &aRender, const TKD::ProjectionType &axe)
{
	const Slice &slice = billon.slice(sliceIndex);

	const uint &width = billon.n_cols;
	const uint &height = billon.n_rows;
	const uint &depth = billon.n_slices;
	const int &minIntensity = intensityInterval.min();
	const qreal fact = 255.0/intensityInterval.size();

	DGtal::HueShadeColorMap<unsigned char> hueShade (0, 255);
	DGtal::HueShadeColorMap<unsigned char> hueShadeLog (log(1), log(1+255));
	DGtal::GrayscaleColorMap<unsigned char> grayShade (0, 255);
	DGtal::GradientColorMap<unsigned char> customShade(0,255);
	customShade.addColor( DGtal::Color::Blue );
	customShade.addColor( DGtal::Color::Red );
	customShade.addColor( DGtal::Color::Green );
	customShade.addColor( DGtal::Color::White );


	QRgb * line = (QRgb *) image.bits();
	int color;
	uint i,j,k;

	if ( axe == TKD::Y_PROJECTION )
	{
		for ( k=0 ; k<depth ; ++k)
		{
			for ( i=0 ; i<width ; ++i)
			{
				color = (TKD::restrictedValue(billon.at(sliceIndex,i,k),intensityInterval)-minIntensity)*fact;
				DGtal::Color col= ((aRender== TKD::HueScale) ? hueShade( color): (aRender==TKD::GrayScale)? grayShade(color): customShade(color));
				*(line++) = qRgb(col.red(),col.green(),col.blue());
			}
		}
	}
	else if ( axe == TKD::Z_PROJECTION )
	{
		for ( j=0 ; j<height ; ++j)
		{
			for ( i=0 ; i<width ; ++i)
			{
				color = (TKD::restrictedValue(slice.at(j,i),intensityInterval)-minIntensity)*fact;
				DGtal::Color col= ((aRender== TKD::HueScale) ? hueShade( color): (aRender==TKD::GrayScale)? grayShade(color): (aRender==TKD::HueScaleLog)? hueShadeLog(log(1+color)):customShade(color));
				*(line++) = qRgb(col.red(),col.green(),col.blue());
			}
		}
	}
	else if ( axe == TKD::CARTESIAN_PROJECTION )
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
				color = (TKD::restrictedValue(slice.at(y,x),intensityInterval)-minIntensity)*fact;
				DGtal::Color col= ((aRender== TKD::HueScale) ? hueShade( color): (aRender==TKD::GrayScale)? grayShade(color): (aRender==TKD::HueScaleLog)? hueShadeLog(log(1+color)):customShade(color));
				*(line++) = qRgb(col.red(),col.green(),col.blue());

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
	else if ( axe == TKD::CARTESIAN_PROJECTION )
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
