#include "inc/histogram.h"

#include "inc/global.h"
#include "inc/interval.h"

Histogram::Histogram() : _useNextSlice(false)
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

/*******************************
 * Public setters
 *******************************/

void Histogram::useNextSliceInsteadOfCurrentSlice( const bool &enable )
{
	_useNextSlice = enable;
}
