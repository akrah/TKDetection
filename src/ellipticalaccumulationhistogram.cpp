#include "inc/ellipticalaccumulationhistogram.h"

#include "inc/billon.h"

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

void EllipticalAccumulationHistogram::construct( const Slice &slice, const uiCoord2D &origin, const qreal &ellipticityRate )
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

	findFirstMaximumAndNextMinimum();
}

void EllipticalAccumulationHistogram::findFirstMaximumAndNextMinimum()
{
	_maximums.clear();

	const uint &size = this->size();
	if ( size<4 ) return;

	_maximums.resize(3);

	uint maximumIndex = 2;
	while ( maximumIndex<size && (*this)[maximumIndex] <= (*this)[maximumIndex-2] ) maximumIndex++;
	while ( maximumIndex<size && (*this)[maximumIndex] > (*this)[maximumIndex-2] ) maximumIndex++;

	if ( maximumIndex==size ) return;

	maximumIndex--;
	_maximums[0] = maximumIndex;

	uint minimumIndex = maximumIndex+2;
	while ( minimumIndex<size && (*this)[minimumIndex] < (*this)[minimumIndex-2]-3 ) minimumIndex++;
	minimumIndex--;
	minimumIndex = qMax(qMin(minimumIndex,size-1),maximumIndex);
	_maximums[2] = minimumIndex;


	const qreal &maximumValue = (*this)[maximumIndex];
	const qreal &minimumValue = (*this)[minimumIndex];
	const qreal meanValue = ( maximumValue+minimumValue )/2.;

	while ( (*this)[maximumIndex++] > meanValue );

	_maximums[1] = maximumIndex-1;
}
