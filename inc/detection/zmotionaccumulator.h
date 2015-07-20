#ifndef ZMOTIONACCUMULATOR_H
#define ZMOTIONACCUMULATOR_H

#include "def/def_billon.h"
#include "inc/interval.h"
#include "inc/piechart.h"

#include <QtGlobal>

class ZMotionAccumulator
{
public:
	ZMotionAccumulator();

	void execute( const Billon &billon, Slice &accumulationSlice , const Interval<uint> &validSlices );

	PieChart &pieChart();
	const PieChart &pieChart() const;
	const Interval<int> &intensityOInterval() const;
	const uint &zMotionMin() const;
	const uint &radiusAroundPith() const;
	const qreal &maxFindIntensity() const;

	void setIntensityInterval( const Interval<int> interval );
	void setZMotionMin( const uint & min );
	void setRadiusAroundPith( const uint radius );


private:
	PieChart _pieChart;
	Interval<int> _intensityInterval;
	uint _zMotionMin;
	uint _radiusAroundPith;

	qreal _maxFindIntensity;
};

#endif // ZMOTIONACCUMULATOR_H
