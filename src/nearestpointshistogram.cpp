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

void NearestPointsHistogram::construct( const Billon &billon, const int &treeRadius )
{
	this->clear();
	this->resize( billon.n_slices );

	uint i;
	iCoord2D nearestPoint;
	qreal minValue = qMax(billon.n_cols,billon.n_rows);
	for ( i=0 ; i<billon.n_slices ; ++i )
	{
		nearestPoint = SliceAlgorithm::findNearestPointOfThePith( billon.slice(i), billon.pithCoord(i), billon.minValue() );
		(*this)[i] = treeRadius - nearestPoint.euclideanDistance( billon.pithCoord(i) );
		minValue = qMin(minValue,(*this)[i]);
	}
	for ( i=0 ; i<billon.n_slices ; ++i )
	{
		(*this)[i] -= minValue;
	}
}

void NearestPointsHistogram::computeMaximumsAndIntervals( const uint &smoothingRadius, const int & minimumHeightPercentageOfMaximum,
														  const int & derivativesPercentage, const int &minimumWidthOfIntervals, const bool & loop )
{
	Histogram<qreal>::computeMaximumsAndIntervals( smoothingRadius, minimumHeightPercentageOfMaximum,
											derivativesPercentage, minimumWidthOfIntervals , loop );


	QVector< Interval<uint> >::Iterator intervalsIter = _intervals.begin();
	while ( intervalsIter != _intervals.end() )
	{
		Interval<uint> &currentInterval = *intervalsIter++;
		currentInterval.setMin( currentInterval.min() + smoothingRadius );
		currentInterval.setMax( currentInterval.max() - smoothingRadius );
	}
}

//void NearestPointsHistogram::computeMaximumsAndIntervals( const uint & comparisonShift, const qreal &comparisonValue )
//{
//	if ( this->isEmpty() ) return;

//	const int histoSize = this->size();
//	const int iComparisonShift = comparisonShift;

//	// Calcul du maximum
//	uint minIndex = 0;
//	qreal minVal = (*this)[0];
//	for ( int i=1 ; i<histoSize ; ++i )
//	{
//		if ( minVal > (*this)[i] )
//		{
//			minVal = (*this)[i];
//			minIndex = i;
//		}
//	}

//	_maximums.resize(1);
//	_maximums[0] = minIndex;

//	// Calcul de l'intervalle
//	int upperIndex, lowerIndex;

//	upperIndex = minIndex;
//	while ( upperIndex < histoSize-1 && ((*this)[qMin(upperIndex+iComparisonShift,histoSize)] - (*this)[upperIndex]) > comparisonValue ) upperIndex++;

//	lowerIndex = minIndex;
//	while ( lowerIndex > 0 && (*this)[qMax(lowerIndex-iComparisonShift,0)] - (*this)[lowerIndex] > comparisonValue ) lowerIndex--;

//	_intervals.resize(1);
//	_intervals[0] = Interval<uint>(lowerIndex+1,upperIndex-1);

//	if ( _intervals[0].min() >= _intervals[0].max() ) _intervals.clear();
//}
