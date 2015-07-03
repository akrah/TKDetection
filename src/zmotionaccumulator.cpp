#include "inc/zmotionaccumulator.h"

#include "inc/billon.h"
#include "inc/piechart.h"
#include "inc/sectorhistogram.h"

ZMotionAccumulator::ZMotionAccumulator() : _pieChart(PieChart(500)), _intensityInterval(Interval<int>(MINIMUM_INTENSITY,MAXIMUM_INTENSITY)),
	_zMotionMin(MINIMUM_Z_MOTION), _radiusAroundPith(100), _maxFindIntensity(0.)
{
}

void ZMotionAccumulator::execute( const Billon &billon, Slice &accumulationSlice, const Interval<uint> &validSlices )
{
	if ( billon.hasPith() )
	{
		accumulationSlice.set_size(_pieChart.nbSectors(),validSlices.size());

		SectorHistogram sect;
		sect.pieChart().setNumberOfAngularSectors(_pieChart.nbSectors());

		_maxFindIntensity = 0;

		for ( uint z=validSlices.min() ; z<validSlices.max() ; ++z )
		{
			sect.construct( billon, Interval<uint>(z,z), _intensityInterval, _zMotionMin, _radiusAroundPith );
			QVector<qreal>::ConstIterator sectIter = sect.constBegin();
			QVector<qreal>::ConstIterator sectIterEnd = sect.constEnd();
			Slice::col_iterator accumulationSliceIter = accumulationSlice.begin_col(z-validSlices.min());
			while ( sectIter != sectIterEnd )
			{
				*accumulationSliceIter = (__billon_type__)(*sectIter);
				_maxFindIntensity = qMax(_maxFindIntensity,*sectIter);
				sectIter++;
				accumulationSliceIter++;
			}
		}
	}
}

PieChart &ZMotionAccumulator::pieChart()
{
	return _pieChart;
}

const PieChart &ZMotionAccumulator::pieChart() const
{
	return _pieChart;
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

