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

//		upperIndex = qMin(histoSize-comparisonShift,minIndex);
//		while ( upperIndex < histoSize-comparisonShift && ((*this)[upperIndex+comparisonShift] - (*this)[upperIndex]) > comparisonValue ) upperIndex++;

		upperIndex = minIndex;
		while ( upperIndex < histoSize-1 && ((*this)[qMin(upperIndex+comparisonShift,histoSize)] - (*this)[upperIndex]) > comparisonValue ) upperIndex++;

//		lowerIndex = qMax(comparisonShift,minIndex);
//		while ( lowerIndex >= comparisonShift && (*this)[lowerIndex-comparisonShift] - (*this)[lowerIndex] > comparisonValue ) lowerIndex--;

		lowerIndex = minIndex;
		while ( lowerIndex > 0 && (*this)[qMin(lowerIndex-comparisonShift,static_cast<uint>(0))] - (*this)[lowerIndex] > comparisonValue ) lowerIndex--;

		_intervals.resize(1);
		_intervals[0] = Interval<uint>(lowerIndex+1,upperIndex-1);

		if ( _intervals[0].min() >= _intervals[0].max() ) _intervals.clear();
	}
}
