#ifndef INTENSITYDISTRIBUTIONHISTOGRAM_H
#define INTENSITYDISTRIBUTIONHISTOGRAM_H

#include "def/def_billon.h"
#include "inc/histogram.h"

class IntensityDistributionHistogram : public Histogram<qreal>
{
public:
	IntensityDistributionHistogram();
	virtual ~IntensityDistributionHistogram();

	void construct( const Billon &billon, const Interval<uint> &sliceInterval, const Interval<int> &intensityInterval, const uint &smoothingRadius );
};

#endif // INTENSITYDISTRIBUTIONHISTOGRAM_H
