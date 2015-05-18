#ifndef ZMOTIONACCUMULATOR_H
#define ZMOTIONACCUMULATOR_H

#include "def/def_billon.h"
#include "inc/interval.h"

#include <QtGlobal>

class ZMotionAccumulator
{
	public:
		ZMotionAccumulator();

		void execute( const Billon &billon, Slice &slice, const Interval<uint> &validSlices );

		const Interval<int> &intensityOInterval() const;
		const uint &zMotionMin() const;
		const uint &radiusAroundPith() const;
		const uint &nbAngularSectors() const;
		const qreal &maxFindIntensity() const;

		void setIntensityInterval( const Interval<int> interval );
		void setZMotionMin( const uint & min );
		void setRadiusAroundPith( const uint radius );
		void setNbAngularSectors( const uint &nbAngularSectors );


	private:
		Interval<int> _intensityInterval;
		uint _zMotionMin;
		uint _radiusAroundPith;
		uint _nbAngularSectors;

		qreal _maxFindIntensity;
};

#endif // ZMOTIONACCUMULATOR_H
