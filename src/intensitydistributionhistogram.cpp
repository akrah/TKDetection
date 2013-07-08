#include "inc/intensitydistributionhistogram.h"

#include "inc/billon.h"
#include "inc/piechart.h"
#include "inc/coordinate.h"

IntensityDistributionHistogram::IntensityDistributionHistogram() : Histogram<qreal>()
{
}

IntensityDistributionHistogram::~IntensityDistributionHistogram()
{
}

void IntensityDistributionHistogram::construct( const Billon &billon, const Interval<uint> &sliceInterval, const Interval<int> &intensityInterval, const uint &smoothingRadius )
{
	const uint &width = billon.n_cols;
	const uint &height = billon.n_rows;
	const int &minVal = intensityInterval.min();

	uint i, j, k;

	clear();
	resize(intensityInterval.size()+1);

	for ( k=sliceInterval.min() ; k<=sliceInterval.max() ; ++k )
	{
		const Slice &slice = billon.slice(k);
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( intensityInterval.containsClosed(slice.at(j,i)) ) ++((*this)[slice.at(j,i)-minVal]);
			}
		}
	}

	meansSmoothing(smoothingRadius,false);
}

void IntensityDistributionHistogram::construct( const Billon &billon, const Interval<uint> &sliceInterval, const Interval<uint> &sectorInterval,
												const PieChart &pieChart, const iCoord2D &pithCoord, const uint &maxDistance, const Interval<int> &intensityInterval,
												const uint &smoothingRadius )
{
	const uint &width = billon.n_cols;
	const uint &height = billon.n_rows;
	const int &minVal = intensityInterval.min();

	uint i, j, k;

	clear();
	resize(intensityInterval.size()+1);

	for ( j=0 ; j<height ; ++j )
	{
		for ( i=0 ; i<width ; ++i )
		{
			if ( sectorInterval.containsClosed(pieChart.sectorIndexOfAngle(pithCoord.angle(iCoord2D(i,j)))) && pithCoord.euclideanDistance(iCoord2D(i,j)) < maxDistance )
			{
				for ( k=sliceInterval.min() ; k<=sliceInterval.max() ; ++k )
				{
					if ( intensityInterval.containsClosed(billon.slice(k).at(j,i)) ) ++((*this)[billon.slice(k).at(j,i)-minVal]);
				}
			}
		}
	}

	meansSmoothing(smoothingRadius,false);
}

int IntensityDistributionHistogram::computeIndexOfPartialSum( const qreal &percentage )
{
	int searchIndex;
	qreal partialSum, totalSum;
	totalSum = 0.;
	for ( int k=0 ; k<this->size() ; ++k )
	{
		totalSum += (*this)[k];
	}
	partialSum = 0.;
	searchIndex = 0;
	while ( partialSum/totalSum < percentage )
	{
		partialSum += (*this)[searchIndex++];
	}
	return searchIndex;
}

