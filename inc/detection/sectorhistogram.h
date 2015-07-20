#ifndef SECTORHISTOGRAM_H
#define SECTORHISTOGRAM_H

#include "def/def_billon.h"
#include "inc/histogram.h"
#include "inc/piechart.h"

class SectorHistogram : public Histogram<qreal>
{
public:
	SectorHistogram();
	SectorHistogram( const PieChart &pieChart );
	virtual ~SectorHistogram();

	void construct( const Billon &billon, const Interval<uint> &sliceInterval,
					const Interval<int> &intensity, const uint &zMotionMin, const int &radiusAroundPith );

	const PieChart &pieChart() const;
	void setPieChart( const PieChart &pieChart );
	void setSectorNumber( const uint &nbSectors );

private:
	PieChart _pieChart;
};

#endif // SECTORHISTOGRAM_H
