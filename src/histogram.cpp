#include "inc/histogram.h"

#include "inc/global.h"
#include "inc/interval.h"

Histogram::Histogram() : _marrowAroundDiameter(100), _smoothing(SmoothingType::MEANS), _maximumsNeighborhood(DEFAULT_MINIMUM_WIDTH_OF_NEIGHBORHOOD),
	_minimumIntervalWidth(DEFAULT_MINIMUM_WIDTH_OF_INTERVALS), _movementThresholdMin(MINIMUM_Z_MOTION), _movementThresholdMax(MAXIMUM_Z_MOTION),
	_derivativePercentage(DEFAULT_PERCENTAGE_FOR_MAXIMUM_CANDIDATE), _derivativeThreshold(0.), _useNextSlice(false)
{
}

Histogram::~Histogram()
{
}

/*******************************
 * Public getters
 *******************************/

int Histogram::count() const
{
	return _datas.size()+1;
}

qreal Histogram::value( const uint &index ) const
{
	qreal res = 0.;
	if ( static_cast<int>(index) < _datas.size() )
	{
		res = _datas[index];
	}
	return res;
}

int Histogram::nbMaximums() const
{
	return _maximums.size();
}

int Histogram::indexOfIemeMaximum( const int &maximumIndex ) const
{
	int sliceIndex = 0;
	if ( maximumIndex>-1 && maximumIndex<_maximums.size() )
	{
		sliceIndex = _maximums[maximumIndex];
	}
	return sliceIndex;
}

int Histogram::indexOfIemeInterval( const int &intervalIndex ) const
{
	int sliceIndex = 0;
	if ( intervalIndex>-1 && intervalIndex<_intervals.size() )
	{
		const Interval interval = _intervals[intervalIndex];
		sliceIndex = (interval.minValue()+interval.maxValue())/2;
	}
	return sliceIndex;
}

int Histogram::marrowAroundDiameter() const
{
	return _marrowAroundDiameter;
}

/*******************************
 * Public setters
 *******************************/

void Histogram::setMarrowAroundDiameter( const int &diameter )
{
	_marrowAroundDiameter = diameter;
}

void Histogram::setSmoothingType( const SmoothingType::SmoothingType &type )
{
	if ( type > SmoothingType::_SMOOTHING_TYPE_MIN_ && type < SmoothingType::_SMOOTHING_TYPE_MAX_ )
	{
		_smoothing = type;
	}
}

void Histogram::setMaximumsNeighborhood( const int &neighborhood )
{
	_maximumsNeighborhood = neighborhood;
}

void Histogram::setMinimumIntervalWidth( const int &width )
{
	_minimumIntervalWidth = width;
}

void Histogram::setMovementThresholdMin( const int &threshold )
{
	_movementThresholdMin = threshold;
}

void Histogram::setMovementThresholdMax( const int &threshold )
{
	_movementThresholdMax = threshold;
}

void Histogram::setDerivativePercentage( const qreal &percentage )
{
	_derivativePercentage = percentage;
}

void Histogram::useNextSliceInsteadOfCurrentSlice( const bool &enable )
{
	_useNextSlice = enable;
}
