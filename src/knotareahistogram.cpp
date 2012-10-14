#include "inc/knotareahistogram.h"

#include "inc/billon.h"
#include "inc/billonalgorithms.h"
#include "inc/connexcomponentextractor.h"

KnotAreaHistogram::KnotAreaHistogram() : Histogram<qreal>()
{
}

KnotAreaHistogram::~KnotAreaHistogram()
{
}


/**********************************
 * Public setters
 **********************************/

void KnotAreaHistogram::construct( const Billon &billon, const int &minimumSideSizeOfComponent )
{
	this->clear();
	this->resize( billon.n_slices );

	Slice *biggestComponents;
	iCoord2D nearestPoint;
	for ( uint i=0 ; i<billon.n_slices ; ++i )
	{
		biggestComponents = ConnexComponentExtractor::extractConnexComponents( billon.slice(i), qPow(minimumSideSizeOfComponent,2), billon.minValue() );
		nearestPoint = BillonAlgorithms::findNearestPointOfThePith( *biggestComponents, billon.pithCoord(i), billon.minValue() );
		(*this)[i] = nearestPoint.euclideanDistance( billon.pithCoord(i) );
		delete biggestComponents;
		biggestComponents = 0;
	}
}

void KnotAreaHistogram::computeMaximumsAndIntervals( const uint & comparisonShift, const qreal &comparisonValue )
{
	if ( !this->isEmpty() )
	{
		const uint histoSize = this->size();

		// Calcul du maximum
		uint minIndex = 0;
		qreal minVal = (*this)[0];
		for ( uint i=1 ; i<histoSize ; ++i )
		{
			if ( minVal > (*this)[i] )
			{
				minVal = (*this)[i];
				minIndex = i;
			}
		}

		_maximums.resize(1);
		_maximums[0] = minIndex;

		// Calcul de l'intervalle
		uint upperIndex, lowerIndex;

		upperIndex = qMin(histoSize-comparisonShift,minIndex);
		while ( upperIndex <= histoSize-comparisonShift && ((*this)[upperIndex+comparisonShift] - (*this)[upperIndex]) > comparisonValue ) upperIndex++;

		lowerIndex = qMax(comparisonShift,minIndex);
		while ( lowerIndex >= comparisonShift && (*this)[lowerIndex-comparisonShift] - (*this)[lowerIndex] > comparisonValue ) lowerIndex--;

		_intervals.resize(1);
		_intervals[0] = Interval<uint>(lowerIndex+1,upperIndex-1);
	}
}