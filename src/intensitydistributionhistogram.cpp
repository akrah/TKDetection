#include "inc/intensitydistributionhistogram.h"

#include "inc/billon.h"

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
