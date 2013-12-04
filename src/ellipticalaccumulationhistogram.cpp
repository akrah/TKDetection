#include "inc/ellipticalaccumulationhistogram.h"

#include "inc/billon.h"
#include "inc/globalfunctions.h"

EllipticalAccumulationHistogram::EllipticalAccumulationHistogram() : Histogram<qreal>()
{
}


EllipticalAccumulationHistogram::~EllipticalAccumulationHistogram()
{
}

uint EllipticalAccumulationHistogram::detectedRadius() const
{
	return _maximums.size()?_maximums[1]:0;
}

/**********************************
 * Public setters
 **********************************/

void EllipticalAccumulationHistogram::construct( const Slice &slice, const uiCoord2D &origin, const qreal &ellipticityRate,
												 const uint &smoothingRadius, const uint &minimumGap, const qreal &widthCoeff )
{
	const uint &width = slice.n_cols;
	const uint &height = slice.n_rows;
	const uint semiWidth = qFloor(width/2.);
	const uint firstX = qFloor(semiWidth*widthCoeff);
	const uint lastX = width-firstX;
	const qreal &pithCoordX = origin.x;
	const qreal &pithCoordY = origin.y;
	const uint nbEllipses = qMin(qMin(pithCoordX,width-pithCoordX),qMin(pithCoordY/ellipticityRate,(height-pithCoordY)/ellipticityRate));

	int x,y;
	qreal a,b,d1,d2,aSquare,bSquare, nbPixelOnEllipse ;

	resize(nbEllipses);

	if ( nbEllipses ) (*this)[0] = slice( 0,0 );
	for ( a=1 ; a<nbEllipses ; a++ )
	{
		nbPixelOnEllipse = 0;
		b = a*ellipticityRate;
		aSquare = a*a;
		bSquare = b*b;
		x = 0;
		y = b;
		d1 = bSquare - aSquare*b + aSquare/4. ;
		if ( pithCoordX+x<lastX )
		{
			(*this)[a] += slice( qRound(pithCoordY+y), qRound(pithCoordX+x) ) +
						  slice( qRound(pithCoordY-y), qRound(pithCoordX+x) );
			nbPixelOnEllipse += 2;
		}
		if ( pithCoordX-x>firstX )
		{
			(*this)[a] += slice( qRound(pithCoordY+y), qRound(pithCoordX-x) ) +
						  slice( qRound(pithCoordY-y), qRound(pithCoordX-x) );
			nbPixelOnEllipse += 2;
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
			if ( pithCoordX+x<lastX )
			{
				(*this)[a] += slice( qRound(pithCoordY+y), qRound(pithCoordX+x) ) +
							  slice( qRound(pithCoordY-y), qRound(pithCoordX+x) );
				nbPixelOnEllipse += 2;
			}
			if ( pithCoordX-x>firstX )
			{
				(*this)[a] += slice( qRound(pithCoordY+y), qRound(pithCoordX-x) ) +
							  slice( qRound(pithCoordY-y), qRound(pithCoordX-x) );
				nbPixelOnEllipse += 2;
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
			if ( pithCoordX+x<lastX )
			{
				(*this)[a] += slice( qRound(pithCoordY+y), qRound(pithCoordX+x) ) +
							  slice( qRound(pithCoordY-y), qRound(pithCoordX+x) );
				nbPixelOnEllipse += 2;
			}
			if ( pithCoordX-x>firstX )
			{
				(*this)[a] += slice( qRound(pithCoordY+y), qRound(pithCoordX-x) ) +
							  slice( qRound(pithCoordY-y), qRound(pithCoordX-x) );
				nbPixelOnEllipse += 2;
			}
		}
		(*this)[a] /= nbPixelOnEllipse;
	}

	if (smoothingRadius) TKD::meanSmoothing<qreal>( this->begin(), this->end(), smoothingRadius );
	findFirstMaximumAndNextMinimum( minimumGap );
}

void EllipticalAccumulationHistogram::findFirstMaximumAndNextMinimum( const uint &minimumGap )
{
	_maximums.clear();

	const uint &size = this->size();
	if ( size<4 ) return;

	_maximums.resize(2);

	// Recherche du premier maximum
	uint maximumIndex = 2;
	while ( maximumIndex<size && (*this)[maximumIndex] > (*this)[maximumIndex-2] ) maximumIndex++;
	if ( maximumIndex==size ) return;

	while ((*this)[maximumIndex]<(*this)[maximumIndex-1]) maximumIndex--;
	_maximums[0] = maximumIndex;

	// Recherche du x corespondant au f(x) median de f(maximumIndex) et f(minimumIndex)
	maximumIndex+=2;
	qreal oldSlope = (*this)[maximumIndex] - (*this)[maximumIndex-2];
	qreal currentSlope;
	bool increase = true;
	while ( increase )
	{
		maximumIndex++;
		currentSlope = (*this)[maximumIndex] - (*this)[maximumIndex-2];
		increase = currentSlope<oldSlope;
		oldSlope = currentSlope;
	}

	_maximums[1] = maximumIndex-2;
}

//void EllipticalAccumulationHistogram::findFirstMaximumAndNextMinimum( const uint &minimumGap )
//{
//	_maximums.clear();

//	const uint &size = this->size();
//	if ( size<4 ) return;

//	_maximums.resize(3);

//	// Recherche du premier maximum
//	uint maximumIndex = 2;
//	while ( maximumIndex<size && (*this)[maximumIndex] <= (*this)[maximumIndex-2] ) maximumIndex++;
//	while ( maximumIndex<size && (*this)[maximumIndex] > (*this)[maximumIndex-2] ) maximumIndex++;

//	if ( maximumIndex==size ) return;

//	maximumIndex--;
//	_maximums[0] = maximumIndex;

//	// Recherche du premier minimum après le premier maximum
//	uint minimumIndex = maximumIndex+1;
//	while ( minimumIndex<size && (*this)[minimumIndex] < (*this)[minimumIndex-2]-minimumGap ) minimumIndex++;
//	minimumIndex--;
//	minimumIndex = qMax(qMin(minimumIndex,size-1),maximumIndex);
//	_maximums[2] = minimumIndex;


//	// Recherche du x corespondant au f(x) median de f(maximumIndex) et f(minimumIndex)
//	//const qreal meanValue = (*this)[maximumIndex] - (qAbs((*this)[minimumIndex]-(*this)[maximumIndex]) / (minimumIndex<size-5?2.:8.));
//	const qreal meanValue = (*this)[maximumIndex] - (qAbs((*this)[minimumIndex]-(*this)[maximumIndex]) * 0.25);
//	//if ( minimumIndex<size-1 )
//		while ( maximumIndex<size-1 && (*this)[maximumIndex] > meanValue ) maximumIndex++;
//	//else maximumIndex += 1;

//	_maximums[1] = maximumIndex;
//}
