#include "inc/pithextractorboukadida.h"

#include "inc/billon.h"
#include "inc/coordinate.h"

PithExtractorBoukadida::PithExtractorBoukadida( const int &subWindowWidth, const int &subWindowHeight, const qreal &pithShift, const uint smoothingRadius ) :
	_subWindowWidth(subWindowWidth), _subWindowHeight(subWindowHeight), _pithShift(pithShift), _smoothingRadius(smoothingRadius)
{
}

PithExtractorBoukadida::~PithExtractorBoukadida()
{
}

int PithExtractorBoukadida::subWindowWidth() const
{
	return _subWindowWidth;
}

int PithExtractorBoukadida::subWindowHeight() const
{
	return _subWindowHeight;
}

qreal PithExtractorBoukadida::pithShift() const
{
	return _pithShift;
}

uint PithExtractorBoukadida::smoothingRadius() const
{
	return _smoothingRadius;
}

void PithExtractorBoukadida::setSubWindowWidth( const int &width )
{
	_subWindowWidth = width;
}

void PithExtractorBoukadida::setSubWindowHeight( const int &height )
{
	_subWindowHeight = height;
}

void PithExtractorBoukadida::setPithShift( const qreal & shift )
{
	_pithShift = shift;
}

void PithExtractorBoukadida::setSmoothingRadius( const uint &radius )
{
	_smoothingRadius = radius;
}

void PithExtractorBoukadida::process( Billon &billon ) const
{
	const int &width = billon.n_cols-1;
	const int &height = billon.n_rows-1;
	const uint &depth = billon.n_slices;
	Pith &pith = billon._pith;

	uint k;

	pith.clear();
	pith.resize(depth);

	QVector<qreal> nbLineByMaxRatio( depth, 0. );

	Slice sliceFillBackground(billon.slice(0));
	Slice::iterator startSlice = sliceFillBackground.begin();
	const Slice::iterator endSlice = sliceFillBackground.end();
	while ( startSlice != endSlice )
	{
		*startSlice = qMax(qMin( *startSlice, 530 ), -900 );
		startSlice++;
	}

	// Calcul de la moelle sur la première coupe
	pith[0] = transHough( sliceFillBackground, nbLineByMaxRatio[0] );

	// Calcul de la moelle sur les coupes suivantes
	const int semiSubWindowWidth = _subWindowWidth/2;
	const int semiSubWindowHeight = _subWindowHeight/2;
	uiCoord2D currentPithCoord;
	uiCoord2D subWindowStart, subWindowEnd;
	for ( k=1 ; k<depth ; ++k )
	{
		sliceFillBackground = billon.slice(k);
		startSlice = sliceFillBackground.begin();
		while ( startSlice != endSlice )
		{
			*startSlice = qMax(qMin( *startSlice, 530 ), -900 );
			startSlice++;
		}

		const iCoord2D &previousPith = pith[k-1];

		subWindowStart.x = qMax(qMin(previousPith.x-semiSubWindowWidth,width),0);
		subWindowEnd.x = qMax(qMin(previousPith.x+semiSubWindowWidth,width),0);
		subWindowStart.y = qMax(qMin(previousPith.y-semiSubWindowHeight,height),0);
		subWindowEnd.y = qMax(qMin(previousPith.y+semiSubWindowHeight,height),0);
		currentPithCoord = transHough( sliceFillBackground.submat( subWindowStart.y, subWindowStart.x, subWindowEnd.y, subWindowEnd.x ), nbLineByMaxRatio[k] )
						   + subWindowStart;

		if ( currentPithCoord.euclideanDistance(previousPith) > _pithShift )
			currentPithCoord = transHough( sliceFillBackground, nbLineByMaxRatio[k] );

		pith[k] = currentPithCoord;
	}

	// Interpolation
	QVector<qreal> nbLineByMaxRatioSorting( nbLineByMaxRatio );
	qSort(nbLineByMaxRatioSorting);

	const qreal &quartile1 = nbLineByMaxRatioSorting[ 0.25 * nbLineByMaxRatioSorting.size() ];
	const qreal &quartile3 = nbLineByMaxRatioSorting[ 0.75 * nbLineByMaxRatioSorting.size() ];
	const qreal interpolationThreshold = quartile1 - 0.5 * ( quartile3 - quartile1 );

	uint startSliceIndex, newK;
	rCoord2D interpolationStep;
	for ( k=0 ; k<depth ; ++k )
	{
		if ( nbLineByMaxRatio[k] < interpolationThreshold )
		{
			startSliceIndex = k++;
			while ( nbLineByMaxRatio[k++] < interpolationThreshold );

			interpolationStep.x = (pith[k].x - pith[startSliceIndex-(startSliceIndex?1:0)].x) / static_cast<qreal>( k-startSliceIndex+1 );
			interpolationStep.y = (pith[k].y - pith[startSliceIndex-(startSliceIndex?1:0)].y) / static_cast<qreal>( k-startSliceIndex+1 );

			for ( newK = startSliceIndex?startSliceIndex:1 ; newK < k ; ++newK )
			{
				pith[newK].x = pith[newK-1].x + interpolationStep.x;
				pith[newK].y = pith[newK-1].y + interpolationStep.y;
			}
		}
	}

//	// Lissage
//	rCoord2D smoothingSum( 0., 0. );
//	qreal smoothingMaskSize = 2.*_smoothingRadius+1.;
//	for ( k=0 ; k<2*_smoothingRadius+1 ; ++k )
//	{
//		smoothingSum += pith[k];
//	}
//	pith[_smoothingRadius] = smoothingSum / smoothingMaskSize;
//	for ( k=_smoothingRadius+1 ; k<depth-_smoothingRadius ; ++k )
//	{
//		smoothingSum += pith[k+_smoothingRadius] -  pith[k-smoothingRadius()-1];
//		pith[k] = smoothingSum / smoothingMaskSize;
//	}
}


uiCoord2D PithExtractorBoukadida::transHough( const Slice &slice, qreal &lineOnMaxRatio ) const
{
	const uint &width = slice.n_cols;
	const uint &height = slice.n_rows;

	// Calcul des orientations en chaque pixel avec les filtres de Sobel
	arma::Mat<qreal> orientations( height, width );
	arma::Mat<char> hasContour( height, width );
	uint nbContourPoints = contour( slice, orientations, hasContour );

	// Calcul des accumulation des droites suivant les orientations
	arma::Mat<int> accuSlice( height, width );
	accuSlice.fill(0);

	uint i, j;
	for ( j=1 ; j<height-1 ; ++j )
	{
		for ( i=1 ; i<width-1 ; ++i )
		{
			if ( hasContour.at(j,i) )
				drawLine( accuSlice, uiCoord2D(i,j), orientations.at(j,i) );
		}
	}

	// Valeur et coordonnée du maximum de accuSlice
	uiCoord2D pithCoord;
	lineOnMaxRatio = accuSlice.max(pithCoord.y,pithCoord.x);
	lineOnMaxRatio = nbContourPoints ? lineOnMaxRatio/static_cast<qreal>(nbContourPoints) : 0.;

	return pithCoord;
}

uint PithExtractorBoukadida::contour( const Slice &slice, arma::Mat<qreal> & orientations, arma::Mat<char> &hasContour ) const
{
	const uint &width = slice.n_cols-1;
	const uint &height = slice.n_rows-1;

	arma::Mat<qreal> sobelNorm(slice.n_rows,slice.n_cols);
	sobelNorm.fill(0.);

	arma::Col<qreal> sobelNormVec((width-1)*(height-1));
	arma::Col<qreal>::iterator sobelNormVecIt = sobelNormVec.begin();

	uint i, j;
	qreal sobelX, sobelY;

	orientations.fill(0);
	for ( j=1 ; j<height ; ++j )
	{
		for ( i=1 ; i<width ; ++i )
		{
			sobelX = slice.at( j-1, i-1 ) - slice.at( j-1, i+1 ) +
					 2* (slice.at( j, i-1 ) - slice.at( j, i+1 )) +
					 slice.at( j+1, i-1 ) - slice.at( j+1, i+1 );
			sobelY = slice.at( j+1, i-1 ) - slice.at( j-1, i-1 ) +
					 2 * (slice.at( j+1, i ) - slice.at( j-1, i )) +
					 slice.at( j+1, i+1 ) - slice.at( j-1, i+1 );
			orientations.at(j,i) = qFuzzyIsNull(sobelX) ? 9999999999./1. : sobelY/sobelX;
			sobelNorm.at(j,i) = qSqrt( qPow(sobelX,2) + qPow(sobelY,2) )/4.;
			*(sobelNormVecIt++) = sobelNorm.at(j,i);
		}
	}

	const arma::Col<qreal> sobelNormSort = arma::sort( sobelNormVec );
	const qreal &medianVal = sobelNormSort.at( sobelNormSort.n_elem/2 );

	uint nbContourPoints = 0;
	hasContour.fill(false);
	for ( j=1 ; j<height ; ++j )
	{
		for ( i=1 ; i<width ; ++i )
		{
			if ( sobelNorm.at(j,i) > medianVal )
			{
				nbContourPoints++;
				hasContour.at(j,i) = true;
			}
		}
	}

	return nbContourPoints;
}

void PithExtractorBoukadida::drawLine( arma::Mat<int> &slice, const uiCoord2D &origin, const qreal &orientationOrig ) const
{
	const int &width = slice.n_cols;
	const int &height = slice.n_rows;
	const qreal orientation = -orientationOrig;

	qreal x, y;

	if ( orientation >= 1. )
	{
		for ( x = origin.x , y=origin.y; x<width && y<height ; x += (1./orientation), y += 1. )
		{
			slice.at(y,x) += 1;
		}
		for ( x = origin.x-(1./orientation) , y=origin.y-1; x>=0. && y>=0. ; x -= (1./orientation), y -= 1. )
		{
			slice.at(y,x) += 1;
		}
	}
	else if ( orientation > 0. )
	{
		for ( x = origin.x, y=origin.y ; x<width && y<height ; x += 1., y += orientation )
		{
			slice.at(y,x) += 1;
		}
		for ( x = origin.x-1., y=origin.y-orientation ; x>=0 && y>=0 ; x -= 1., y -= orientation )
		{
			slice.at(y,x) += 1;
		}
	}
	else if ( orientation > -1. )
	{
		for ( x = origin.x, y=origin.y ; x<width && y>=0 ; x += 1., y += orientation )
		{
			slice.at(y,x) += 1;
		}
		for ( x = origin.x-1., y=origin.y-orientation ; x>=0 && y<height ; x -= 1., y -= orientation )
		{
			slice.at(y,x) += 1;
		}
	}
	else
	{
		for ( x = origin.x , y=origin.y; x>=0 && y<height ; x += (1./orientation), y += 1. )
		{
			slice.at(y,x) += 1;
		}
		for ( x = origin.x-(1/orientation) , y=origin.y-1.; x<width && y>=0 ; x -= (1./orientation), y -= 1. )
		{
			slice.at(y,x) += 1;
		}
	}
}
