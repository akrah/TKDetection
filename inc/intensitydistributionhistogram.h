#ifndef INTENSITYDISTRIBUTIONHISTOGRAM_H
#define INTENSITYDISTRIBUTIONHISTOGRAM_H

#include "def/def_billon.h"
#include "def/def_coordinate.h"
#include "inc/histogram.h"

class PieChart;

class IntensityDistributionHistogram : public Histogram<qreal>
{
public:
	IntensityDistributionHistogram();
	virtual ~IntensityDistributionHistogram();

	void construct( const Billon &billon, const Interval<uint> &sliceInterval, const Interval<int> &intensityInterval, const uint &smoothingRadius );
	void construct( const Billon &billon, const Interval<uint> &sliceInterval, const Interval<uint> &sectorInterval, const PieChart &pieChart, const iCoord2D &pithCoord,
					const Interval<int> &intensityInterval, const uint &smoothingRadius );
};

#endif // INTENSITYDISTRIBUTIONHISTOGRAM_H
