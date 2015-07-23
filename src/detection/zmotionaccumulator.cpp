#include "inc/detection/zmotionaccumulator.h"

#include "inc/billon.h"
#include "inc/piechart.h"
#include "inc/detection/sectorhistogram.h"

ZMotionAccumulator::ZMotionAccumulator() : _pieChart(PieChart(500)), _intensityInterval(Interval<int>(MINIMUM_INTENSITY,MAXIMUM_INTENSITY)),
	_zMotionMin(MINIMUM_Z_MOTION), _radiusAroundPith(100), _maxFindIntensity(0.)
{
}

void ZMotionAccumulator::execute( const Billon &billon, Slice &accumulationSlice, const Interval<uint> &validSlices )
{
	_maxFindIntensity = 0;
	if ( billon.hasPith() )
	{
		const uint &firstSlice = validSlices.min();
		const uint &lastSlice = validSlices.max();
		accumulationSlice.set_size(_pieChart.nbSectors(),validSlices.size());

		SectorHistogram hist(_pieChart);

		for ( uint z=firstSlice ; z<lastSlice ; ++z )
		{
			hist.construct( billon, Interval<uint>(z,z), _intensityInterval, _zMotionMin, _radiusAroundPith );
			QVector<qreal>::ConstIterator histIter = hist.constBegin();
			const QVector<qreal>::ConstIterator histEnd = hist.constEnd();
			Slice::col_iterator accSliceIter = accumulationSlice.begin_col(z-firstSlice);
			while ( histIter != histEnd )
			{
				*accSliceIter = (__billon_type__)(*histIter);
				_maxFindIntensity = qMax(_maxFindIntensity,*histIter);
				++histIter;
				++accSliceIter;
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

void ZMotionAccumulator::setPieChart( const PieChart &pieChart )
{
	_pieChart = pieChart;
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
