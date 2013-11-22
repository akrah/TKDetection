#include "inc/ellipticalaccumulationhistogram.h"

#include "inc/billon.h"

EllipticalAccumulationHistogram::EllipticalAccumulationHistogram() : Histogram<qreal>()
{
}


EllipticalAccumulationHistogram::~EllipticalAccumulationHistogram()
{
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

	int x,y,nbPixelOnEllipse ;
	qreal a,b,d1,d2,aSquare,bSquare ;

	resize(nbEllipses);

	for ( a=0 ; a<nbEllipses ; a++ )
	{
		nbPixelOnEllipse = 0;
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
		nbPixelOnEllipse += 4;
		while ( aSquare*(y-.5) > bSquare*(x+1) )
		{
			if ( d1 < 0 )
			{
				d1 += bSquare*(2*x+3) ;
				x++ ;
			}
			else
			{
				d1 += bSquare*(2*x+3) + aSquare*(-2*y+2) ;
				x++ ;
				y-- ;
			}
			(*this)[a] += slice.at( qRound(pithCoordY+y), qRound(pithCoordX+x) ) +
						  slice.at( qRound(pithCoordY+y), qRound(pithCoordX-x) ) +
						  slice.at( qRound(pithCoordY-y), qRound(pithCoordX-x) ) +
						  slice.at( qRound(pithCoordY-y), qRound(pithCoordX+x) );
			nbPixelOnEllipse += 4;
		}
		d2 = bSquare*(x+.5)*(x+.5) + aSquare*(y-1)*(y-1) - aSquare*bSquare ;
		while ( y > 0 )
		{
			if ( d2 < 0 )
			{
				d2 += bSquare*(2*x+2) + aSquare*(-2*y+3) ;
				y-- ;
				x++ ;
			}
			else
			{
				d2 += aSquare*(-2*y+3) ;
				y-- ;
			}
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

	if ( !size ) return;

	uint maximumIndex = 3;
	while ( maximumIndex<size && (*this)[maximumIndex] >= (*this)[maximumIndex-2] ) maximumIndex++;

	if ( maximumIndex==size )
	{
		_maximums.append(0);
		_maximums.append(0);
		_maximums.append(0);
		return;
	}

	maximumIndex--;
	_maximums.append(maximumIndex);

	uint minimumIndex = maximumIndex+2;
	while ( minimumIndex<size && (*this)[minimumIndex] <= (*this)[minimumIndex-2]-3 ) minimumIndex++;
	minimumIndex--;
	minimumIndex = qMax(qMin(minimumIndex,size-1),maximumIndex);
	_maximums.append(minimumIndex);


	const qreal &maximumValue = (*this)[maximumIndex];
	const qreal &minimumValue = (*this)[minimumIndex];
	const qreal meanValue = ( maximumValue+minimumValue )/2.;

	while ( (*this)[maximumIndex++] > meanValue );

	_maximums.append(maximumIndex-1);
}






















