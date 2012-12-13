#include "inc/zmotiondistributionhistogram.h"

#include "inc/billon.h"

ZMotionDistributionHistogram::ZMotionDistributionHistogram() : Histogram<qreal>()
{
}


ZMotionDistributionHistogram::~ZMotionDistributionHistogram()
{
}

void ZMotionDistributionHistogram::construct( const Billon &billon, const Interval<int> &intensityInterval, const Interval<uint> &zMotionInterval, const uint &smoothingRadius )
{
	const uint &width = billon.n_cols;
	const uint &height = billon.n_rows;
	const uint &depth = billon.n_slices;
	const int &minVal = zMotionInterval.min();

	uint i, j, k;
	__billon_type__ zMotion;

	clear();
	resize(zMotionInterval.size()+1);

	for ( k=1 ; k<depth ; ++k )
	{
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				zMotion = billon.zMotion(j,i,k);
				if ( intensityInterval.containsClosed(billon.at(j,i,k))
					 && intensityInterval.containsClosed(billon.at(j,i,k-1))
					 && zMotionInterval.containsClosed(zMotion)
				   )
					++((*this)[zMotion-minVal]);
			}
		}
	}

	meansSmoothing(smoothingRadius,false);
}
