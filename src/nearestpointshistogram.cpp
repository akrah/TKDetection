#include "inc/nearestpointshistogram.h"

#include "inc/billon.h"
#include "inc/slicealgorithm.h"
#include "inc/connexcomponentextractor.h"

NearestPointsHistogram::NearestPointsHistogram() : Histogram<qreal>()
{
}

NearestPointsHistogram::~NearestPointsHistogram()
{
}


/**********************************
 * Public setters
 **********************************/

void NearestPointsHistogram::construct( const Billon &billon )
{
	this->clear();
	this->resize( billon.n_slices );

	iCoord2D nearestPoint;
	for ( uint i=0 ; i<billon.n_slices ; ++i )
	{
		nearestPoint = SliceAlgorithm::findNearestPointOfThePith( billon.slice(i), billon.pithCoord(i), billon.minValue() );
		(*this)[i] = nearestPoint.euclideanDistance( billon.pithCoord(i) );
	}
}

void NearestPointsHistogram::computeMaximumsAndIntervals( const uint & comparisonShift, const qreal &comparisonValue )
{
	if ( !this->isEmpty() )
	{
		const int histoSize = this->size();
		const int iComparisonShift = comparisonShift;

		// Calcul du maximum
		uint minIndex = 0;
		qreal minVal = (*this)[0];
		for ( int i=1 ; i<histoSize ; ++i )
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
		int upperIndex, lowerIndex;

		upperIndex = minIndex;
		while ( upperIndex < histoSize-1 && ((*this)[qMin(upperIndex+iComparisonShift,histoSize)] - (*this)[upperIndex]) > comparisonValue ) upperIndex++;

		lowerIndex = minIndex;
		while ( lowerIndex > 0 && (*this)[qMax(lowerIndex-iComparisonShift,0)] - (*this)[lowerIndex] > comparisonValue ) lowerIndex--;

		_intervals.resize(1);
		_intervals[0] = Interval<uint>(lowerIndex+1,upperIndex-1);

		if ( _intervals[0].min() >= _intervals[0].max() ) _intervals.clear();
	}
}
