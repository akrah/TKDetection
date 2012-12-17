#ifndef ZMOTIONDISTRIBUTIONHISTOGRAM_H
#define ZMOTIONDISTRIBUTIONHISTOGRAM_H

#include "def/def_billon.h"
#include "inc/histogram.h"

class ZMotionDistributionHistogram : public Histogram<qreal>
{
public:
	ZMotionDistributionHistogram();
	virtual ~ZMotionDistributionHistogram();


	void construct(const Billon &billon, const Interval<int> &intensityInterval,
				   const Interval<uint> &zMotionInterval, const uint &smoothingRadius, const int &radiusAroundPith );
};

#endif // ZMOTIONDISTRIBUTIONHISTOGRAM_H
