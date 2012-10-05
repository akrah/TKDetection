#ifndef SECTORHISTOGRAM_H
#define SECTORHISTOGRAM_H

#include "histogram.h"

#include "def/def_billon.h"

class Marrow;
class PieChart;

class SectorHistogram : public Histogram<qreal>
{
public:
	SectorHistogram();
	virtual ~SectorHistogram();

	void construct( const Billon &billon, const Marrow &marrow, const PieChart &pieChart,
				  const Interval<int> &slicesInterval, const Interval<int> &intensity, const Interval<int> &motionInterval,
				  const int &radiusAroundPith );
};

#endif // SECTORHISTOGRAM_H
