#ifndef SECTORHISTOGRAM_H
#define SECTORHISTOGRAM_H

#include "histogram.h"

#include "def/def_billon.h"

class Pith;
class PieChart;

class SectorHistogram : public Histogram<qreal>
{
public:
	SectorHistogram();
	virtual ~SectorHistogram();

	void construct( const Billon &billon, const PieChart &pieChart, const Interval<uint> &sliceInterval,
				  const Interval<int> &intensity, const Interval<int> &motionInterval, const int &radiusAroundPith );

protected:
	void computeIntervals( const int &derivativesPercentage, const uint &minimumWidthOfIntervals, const bool &loop );
};

#endif // SECTORHISTOGRAM_H
