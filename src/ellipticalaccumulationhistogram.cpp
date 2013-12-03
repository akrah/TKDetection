#include "inc/ellipticalaccumulationhistogram.h"

#include "inc/billon.h"
#include "inc/globalfunctions.h"

EllipticalAccumulationHistogram::EllipticalAccumulationHistogram() : Histogram<qreal>()
{
	_maximums.resize(3);
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

void EllipticalAccumulationHistogram::construct( const Slice &slice, const uiCoord2D &origin, const qreal &ellipticityRate, const uint &smoothingRadius, const uint &minimumGap )
{
	const uint &width = slice.n_cols;
	const uint &height = slice.n_rows;
	const qreal &pithCoordX = origin.x;
	const qreal &pithCoordY = origin.y;
	const uint nbEllipses = qMin(qMin(pithCoordX,width-pithCoordX),qMin(pithCoordY/ellipticityRate,(height-pithCoordY)/ellipticityRate));

	int x,y;
	qreal a,b,d1,d2,aSquare,bSquare, nbPixelOnEllipse ;

	resize(nbEllipses);

	if ( nbEllipses ) (*this)[0] = slice.at( 0,0 );
	for ( a=1 ; a<nbEllipses ; a++ )
	{
		b = a*ellipticityRate;
		aSquare = a*a;
		bSquare = b*b;
		x = 0;
		y = b;
		d1 = bSquare - aSquare*b + aSquare/4. ;
		(*this)[a] += slice.at( qRound(pithCoordY+y), qRound(pithCoordX+x) ) +
					  slice.at( qRound(pithCoordY+y), qRound(pithCoordX-x) ) +
					  slice.at( qRound(pithCoordY-y), qRound(pithCoordX-x) ) +
					  slice.at( qRound(pithCoordY-y), qRound(pithCoordX+x) );
		nbPixelOnEllipse = 4;
		while ( aSquare*(y-.5) > bSquare*(x+1) )
		{
			if ( d1 >= 0 )
			{
				d1 += aSquare*(-2*y+2) ;
				y-- ;
			}
			d1 += bSquare*(2*x+3) ;
			x++ ;
			(*this)[a] += slice.at( qRound(pithCoordY+y), qRound(pithCoordX+x) ) +
						  slice.at( qRound(pithCoordY+y), qRound(pithCoordX-x) ) +
						  slice.at( qRound(pithCoordY-y), qRound(pithCoordX-x) ) +
						  slice.at( qRound(pithCoordY-y), qRound(pithCoordX+x) );
			nbPixelOnEllipse += 4;
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
			(*this)[a] += slice.at( qRound(pithCoordY+y), qRound(pithCoordX+x) ) +
						  slice.at( qRound(pithCoordY+y), qRound(pithCoordX-x) ) +
						  slice.at( qRound(pithCoordY-y), qRound(pithCoordX-x) ) +
						  slice.at( qRound(pithCoordY-y), qRound(pithCoordX+x) );
			nbPixelOnEllipse += 4;
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

	_maximums.resize(3);

	// Recherche du premier maximum
	uint maximumIndex = 2;
	while ( maximumIndex<size && (*this)[maximumIndex] <= (*this)[maximumIndex-2] ) maximumIndex++;
	while ( maximumIndex<size && (*this)[maximumIndex] > (*this)[maximumIndex-2] ) maximumIndex++;

	if ( maximumIndex==size ) return;

	maximumIndex--;
	_maximums[0] = maximumIndex;

	// Recherche du premier minimum après le premier maximum
	uint minimumIndex = maximumIndex+1;
	while ( minimumIndex<size && (*this)[minimumIndex] < (*this)[minimumIndex-2]-minimumGap ) minimumIndex++;
	minimumIndex--;
	minimumIndex = qMax(qMin(minimumIndex,size-1),maximumIndex);
	_maximums[2] = minimumIndex;


	// Recherche du x corespondant au f(x) median de f(maximumIndex) et f(minimumIndex)
	const qreal meanValue = (*this)[maximumIndex] - (qAbs((*this)[minimumIndex]-(*this)[maximumIndex]) / (minimumIndex<size-5?2.:8.));
	//if ( minimumIndex<size-1 )
		while ( maximumIndex<size-1 && (*this)[maximumIndex] > meanValue ) maximumIndex++;
	//else maximumIndex += 1;

	_maximums[1] = maximumIndex;
}
