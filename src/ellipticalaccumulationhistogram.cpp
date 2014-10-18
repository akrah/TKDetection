#include "inc/ellipticalaccumulationhistogram.h"

#include "inc/billon.h"
#include "inc/globalfunctions.h"

EllipticalAccumulationHistogram::EllipticalAccumulationHistogram() : Histogram<qreal>()
{
}


EllipticalAccumulationHistogram::~EllipticalAccumulationHistogram()
{
}

uint EllipticalAccumulationHistogram::detectedMaximum() const
{
	return _maximums.size()?_maximums[0]:0;
}

uint EllipticalAccumulationHistogram::detectedRadius() const
{
	return _maximums.size()?_maximums[1]:0;
}

/**********************************
 * Public setters
 **********************************/

void EllipticalAccumulationHistogram::construct( const Slice &slice, const uiCoord2D &origin, const qreal &ellipticityRate,
												 const uint &smoothingRadius, const qreal &widthCoeff )
{
	/* Il faudrait penser la coupe slice intégralement, c'est à dire passer à construct() seulement les colonnes utiles.
	 * En effet, la variables nbEllipse est mal définie comme cela.
	 */
	const int &width = slice.n_cols;
	const int &height = slice.n_rows;

	if ( !width || !height )
	{
		resize(0);
		findFirstMaximumAndNextMinimum();
		return;
	}

	const int semiWidth = qFloor((width-1)/2.);
	const int firstX = qFloor(semiWidth-semiWidth*widthCoeff);
	const int lastX = qMin(semiWidth+semiWidth*widthCoeff,width-1.);
	const qreal &pithCoordX = origin.x;
	const qreal &pithCoordY = origin.y;
	const uint nbEllipses = qMin(semiWidth,qFloor(qMin(height-pithCoordY,pithCoordY)/ellipticityRate));

	int x,y;
	qreal a,b,d1,d2,aSquare,bSquare, nbPixelOnEllipse ;

	resize(nbEllipses);

	if ( nbEllipses ) (*this)[0] = slice( pithCoordY, pithCoordX );
	for ( a=1 ; a<nbEllipses ; ++a )
	{
		nbPixelOnEllipse = 0;
		b = a*ellipticityRate;
		aSquare = a*a;
		bSquare = b*b;
		x = 0;
		y = b;
		d1 = bSquare - aSquare*b + aSquare/4. ;
		if ( qRound(pithCoordX+x)<=lastX )
		{
			if ( qRound(pithCoordY+y)<height )
			{
				(*this)[a] += slice( qRound(pithCoordY+y), qRound(pithCoordX+x) );
				++nbPixelOnEllipse;
			}
			if ( qRound(pithCoordY-y)>=0 )
			{
				(*this)[a] += slice( qRound(pithCoordY-y), qRound(pithCoordX+x) );
				++nbPixelOnEllipse;
			}
		}
		if ( qRound(pithCoordX-x)>=firstX )
		{
			if ( qRound(pithCoordY+y)<height )
			{
				(*this)[a] += slice( qRound(pithCoordY+y), qRound(pithCoordX-x) );
				++nbPixelOnEllipse;
			}
			if ( qRound(pithCoordY-y)>=0 )
			{
				(*this)[a] += slice( qRound(pithCoordY-y), qRound(pithCoordX-x) );
				++nbPixelOnEllipse;
			}
		}
		while ( aSquare*(y-.5) > bSquare*(x+1) )
		{
			if ( d1 >= 0 )
			{
				d1 += aSquare*(-2*y+2) ;
				y-- ;
			}
			d1 += bSquare*(2*x+3) ;
			x++ ;
			if ( qRound(pithCoordX+x)<=lastX )
			{
				if ( qRound(pithCoordY+y)<height )
				{
					(*this)[a] += slice( qRound(pithCoordY+y), qRound(pithCoordX+x) );
					++nbPixelOnEllipse;
				}
				if ( qRound(pithCoordY-y)>=0 )
				{
					(*this)[a] += slice( qRound(pithCoordY-y), qRound(pithCoordX+x) );
					++nbPixelOnEllipse;
				}
			}
			if ( qRound(pithCoordX-x)>=firstX )
			{
				if ( qRound(pithCoordY+y)<height )
				{
					(*this)[a] += slice( qRound(pithCoordY+y), qRound(pithCoordX-x) );
					++nbPixelOnEllipse;
				}
				if ( qRound(pithCoordY-y)>=0 )
				{
					(*this)[a] += slice( qRound(pithCoordY-y), qRound(pithCoordX-x) );
					++nbPixelOnEllipse;
				}
			}
		}
		d2 = bSquare*qPow(x+.5,2) + aSquare*qPow(y-1,2) - aSquare*bSquare ;
		while ( y > 0 )
		{
			if ( d2 < 0 )
			{
				d2 += bSquare*(2*x+2);
				x++ ;
			}
			d2 += aSquare*(-2*y+3);
			y-- ;
			if ( qRound(pithCoordX+x)<=lastX )
			{
				if ( qRound(pithCoordY+y)<height )
				{
					(*this)[a] += slice( qRound(pithCoordY+y), qRound(pithCoordX+x) );
					++nbPixelOnEllipse;
				}
				if ( qRound(pithCoordY-y)>=0 )
				{
					(*this)[a] += slice( qRound(pithCoordY-y), qRound(pithCoordX+x) );
					++nbPixelOnEllipse;
				}
			}
			if ( qRound(pithCoordX-x)>=firstX )
			{
				if ( qRound(pithCoordY+y)<height )
				{
					(*this)[a] += slice( qRound(pithCoordY+y), qRound(pithCoordX-x) );
					++nbPixelOnEllipse;
				}
				if ( qRound(pithCoordY-y)>=0 )
				{
					(*this)[a] += slice( qRound(pithCoordY-y), qRound(pithCoordX-x) );
					++nbPixelOnEllipse;
				}
			}
		}
		(*this)[a] /= nbPixelOnEllipse;
	}

	if (smoothingRadius) TKD::meanSmoothing<qreal>( this->begin(), this->end(), smoothingRadius, false );
	findFirstMaximumAndNextMinimum();
}

void EllipticalAccumulationHistogram::findFirstMaximumAndNextMinimum()
{
	_maximums.clear();

	const uint &size = this->size();
	uint lag = 2;

	if ( size<2*lag+1 ) return;

	_maximums.resize(2);

	// Recherche du premier maximum
	uint maximumIndex = lag;
	while ( maximumIndex<size-lag && (*this)[maximumIndex-lag] >= (*this)[maximumIndex+lag] ) ++maximumIndex;
	if ( maximumIndex>size*0.3 ) maximumIndex=lag;
	while ( maximumIndex<size-lag && (*this)[maximumIndex-lag] <= (*this)[maximumIndex+lag] ) ++maximumIndex;
	if ( maximumIndex>size*0.5 ) maximumIndex=lag;
	while ( maximumIndex>=lag && (*this)[maximumIndex] < (*this)[maximumIndex-1] ) --maximumIndex;

	_maximums[0] = maximumIndex;

	lag = 1;

	maximumIndex += lag;
	qreal comparedSlope = (*this)[maximumIndex-lag]+(*this)[maximumIndex-lag-1]-(*this)[maximumIndex+lag]-(*this)[maximumIndex+lag+1];
	++maximumIndex;
	qreal slope = (*this)[maximumIndex-lag]+(*this)[maximumIndex-lag-1]-(*this)[maximumIndex+lag]-(*this)[maximumIndex+lag+1];
	while (maximumIndex<size-lag-2 && slope>=comparedSlope)
	{
		comparedSlope = slope;
		++maximumIndex;
		slope = (*this)[maximumIndex-lag]+(*this)[maximumIndex-lag-1]-(*this)[maximumIndex+lag]-(*this)[maximumIndex+lag+1];
	}

	_maximums[1] = qMin(maximumIndex-lag+1,size-1);
}

//void EllipticalAccumulationHistogram::findFirstMaximumAndNextMinimum()
//{
//	_maximums.clear();

//	const uint &size = this->size();
//	const uint lag = 3;

//	if ( size<2*lag+1 ) return;

//	_maximums.resize(2);

//	// Recherche du premier maximum
//	uint maximumIndex = lag;
//	while ( maximumIndex<size && (*this)[maximumIndex] < (*this)[maximumIndex-lag] ) ++maximumIndex;
//	if ( maximumIndex>size/4 ) maximumIndex=lag;
//	while ( maximumIndex<size && (*this)[maximumIndex] >= (*this)[maximumIndex-lag] ) ++maximumIndex;
//	if ( maximumIndex>2*size/3 ) maximumIndex=lag;

//	_maximums[0] = maximumIndex-lag+1;

//	maximumIndex += lag;
//	qreal comparedSlope = (*this)[maximumIndex-lag]-(*this)[maximumIndex];
//	++maximumIndex;
//	qreal slope = (*this)[maximumIndex-lag]-(*this)[maximumIndex];
//	while (maximumIndex<size-1 && slope>=comparedSlope)
//	{
//		comparedSlope = slope;
//		++maximumIndex;
//		slope = (*this)[maximumIndex-lag]-(*this)[maximumIndex];
//	}

//	_maximums[1] = qMin(maximumIndex-lag+1,size-1);
//}

