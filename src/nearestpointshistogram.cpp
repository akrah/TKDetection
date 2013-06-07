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
		currentInterval.setMin( currentInterval.min() + smoothingRadius + 1 );
		currentInterval.setMax( currentInterval.max() - smoothingRadius - 1 );
	}
}
