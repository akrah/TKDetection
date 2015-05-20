#include "inc/zmotionaccumulator.h"

#include "inc/billon.h"
#include "inc/piechart.h"
#include "inc/sectorhistogram.h"

ZMotionAccumulator::ZMotionAccumulator() : _intensityInterval(Interval<int>(MINIMUM_INTENSITY,MAXIMUM_INTENSITY)),
	_zMotionMin(MINIMUM_Z_MOTION), _radiusAroundPith(100), _nbAngularSectors(RESTRICTED_AREA_DEFAULT_RESOLUTION), _maxFindIntensity(0.)
{
}

void ZMotionAccumulator::execute( const Billon &billon, Slice &slice, const Interval<uint> &validSlices )
{
	if ( billon.hasPith() )
	{
		slice.set_size(_nbAngularSectors,validSlices.size());

		SectorHistogram sect;
		uint oldNbAngularSectors = PieChartSingleton::getInstance()->nbSectors();
		PieChartSingleton::getInstance()->setNumberOfAngularSectors(_nbAngularSectors);

		_maxFindIntensity = 0;

		for ( uint z=validSlices.min() ; z<validSlices.max() ; ++z )
		{
			sect.construct( billon, Interval<uint>(z,z), _intensityInterval, _zMotionMin, _radiusAroundPith );
			QVector<qreal>::ConstIterator sectIter = sect.constBegin();
			QVector<qreal>::ConstIterator sectIterEnd = sect.constEnd();
			Slice::col_iterator sliceIter = slice.begin_col(z-validSlices.min());
			while ( sectIter != sectIterEnd )
			{
				*sliceIter = (__billon_type__)(*sectIter);
				_maxFindIntensity = qMax(_maxFindIntensity,*sectIter);
				sectIter++;
				sliceIter++;
			}
		}

		PieChartSingleton::getInstance()->setNumberOfAngularSectors(oldNbAngularSectors);
	}
}

const Interval<int> &ZMotionAccumulator::intensityOInterval() const
{
	return _intensityInterval;
}

const uint &ZMotionAccumulator::zMotionMin() const
{
	return _zMotionMin;
}

const uint &ZMotionAccumulator::radiusAroundPith() const
{
	return _radiusAroundPith;
}

const uint &ZMotionAccumulator::nbAngularSectors() const
{
	return _nbAngularSectors;
}

const qreal &ZMotionAccumulator::maxFindIntensity() const
{
	return _maxFindIntensity;
}

void ZMotionAccumulator::setIntensityInterval( const Interval<int> interval )
{
	_intensityInterval = interval;
}

void ZMotionAccumulator::setZMotionMin( const uint & min )
{
	_zMotionMin = min;
}

void ZMotionAccumulator::setRadiusAroundPith( const uint radius )
{
	_radiusAroundPith = radius;
}

void ZMotionAccumulator::setNbAngularSectors( const uint &nbAngularSectors )
{
	_nbAngularSectors = nbAngularSectors;
}
