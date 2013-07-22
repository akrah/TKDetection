#ifndef SECTORHISTOGRAM_H
#define SECTORHISTOGRAM_H

#include "histogram.h"

#include "def/def_billon.h"

class Pith;

class SectorHistogram : public Histogram<qreal>
{
public:
	SectorHistogram();
	virtual ~SectorHistogram();

	void construct(const Billon &billon, const Interval<uint> &sliceInterval,
				   const Interval<int> &intensity, const uint &zMotionMin, const int &radiusAroundPith );

	void computeMaximumsAndIntervals( const uint &smoothingRadius, const int & minimumHeightPercentageOfMaximum, const int & derivativesPercentage,
									  const int &minimumWidthOfIntervals,  const uint &intervalGap, const bool & loop );
};

#endif // SECTORHISTOGRAM_H
