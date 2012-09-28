#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include"intervalscomputerdefaultparameters.h"
#include <QVector>
#include "interval.h"
#include <numeric>

template <typename T>
class Histogram
{
public:
	Histogram();
	virtual ~Histogram();

	int size() const;
	T value( int i ) const;
	int nbMaximums() const;
	int maximumIndex( int i ) const;
	int nbIntervals() const;
	int intervalIndex( int i ) const;
	T maxValue() const;
	T minValue() const;
	T thresholdOfMaximums( const int percentage ) const;
	T firstdDerivated( int i, bool loop ) const;

	void meansSmoothing( int smoothingRadius, bool loop );
	void maximumsComputing( int minimumHeightPercentageOfMaximum, int neighborhoodOfMaximums, bool loop );
	void intervalsComputing( int derivativesPercentage, int minimumWidthOfIntervals, bool loop );
	void computeAll( int smoothingRadius, int minimumHeightPercentageOfMaximum, int neighborhoodOfMaximums, int derivativesPercentage, int minimumWidthOfIntervals, bool loop );

protected:
	QVector<T> _datas;
	QVector<int> _maximums;
	QVector<Interval> _intervals;
};

template <typename T> Histogram<T>::Histogram() {}
template <typename T> Histogram<T>::~Histogram() {}


/*******************************
 * Public getters
 *******************************/

template <typename T>
int Histogram<T>::size() const
{
	return _datas.size();
}

template <typename T>
T Histogram<T>::value( int i ) const
{
	if (i < 0 || i >= size()) return T();
	return _datas[i];
}

template <typename T>
int Histogram<T>::nbMaximums() const
{
	return _maximums.size();
}

template <typename T>
int Histogram<T>::maximumIndex( int i ) const
{
	if ( i<0 || i>=_maximums.size() ) return -1;
	return _maximums[i];
}

template <typename T>
int Histogram<T>::nbIntervals() const
{
	return _intervals.size();
}

template <typename T>
int Histogram<T>::intervalIndex( int i ) const
{
	if ( i<0 || i>=_maximums.size() ) return -1;
	return (_intervals[i].minValue()+_intervals[i].maxValue())/2;
}

template <typename T>
T Histogram<T>::minValue() const
{
	typename QVector<T>::const_iterator begin = _datas.begin();
	typename QVector<T>::const_iterator end = _datas.end();

	T res = T();
	while ( begin != end ) res = qMin(res,*begin++);
	return res;
}

template <typename T>
T Histogram<T>::maxValue() const
{
	typename QVector<T>::const_iterator begin = _datas.begin();
	typename QVector<T>::const_iterator end = _datas.end();

	T res = T();
	while ( begin != end ) res = qMax(res,*begin++);
	return res;
}

template <typename T>
T Histogram<T>::thresholdOfMaximums( const int percentage ) const
{
	const T min = minValue();
	const T max = maxValue();
	return (max-min)*(percentage/100.)+min;
}

template <typename T>
T Histogram<T>::firstdDerivated( int i, bool loop ) const
{
	return i>0 ? _datas[i] - _datas[i-1] : loop ? _datas[0] - _datas.last() : _datas[1] - _datas[0];
}

/*******************************
 * Public setters
 *******************************/

template <typename T>
void Histogram<T>::meansSmoothing( int smoothingRadius, bool loop )
{
	const int sizeHist = size();
	const int maskWidth = 2*smoothingRadius+1;
	int i;

	QVector<T> copy;
	copy.reserve( sizeHist + 2*smoothingRadius );
	if ( loop ) {
		for ( i=sizeHist-smoothingRadius ; i<sizeHist ; ++i ) copy << _datas[i];
		copy << _datas;
		for ( i=0 ; i<smoothingRadius ; ++i ) copy << _datas[i];
	}
	else
	{
		for ( i=sizeHist-smoothingRadius ; i<sizeHist ; ++i ) copy << _datas[0];
		copy << _datas;
		for ( i=sizeHist-smoothingRadius ; i<sizeHist ; ++i ) copy << _datas[sizeHist-1];
	}

	typename QVector<T>::const_iterator copyIterBegin = copy.begin();
	typename QVector<T>::const_iterator copyIterEnd = copyIterBegin+(2*smoothingRadius+1);

	typename QVector<T>::iterator histIter = _datas.begin();
	const typename QVector<T>::const_iterator histEnd = _datas.end();

	while ( histIter != histEnd )
	{
		*histIter++ = std::accumulate( copyIterBegin++, copyIterEnd++, T() )/static_cast<T>(maskWidth);
	}
}

template <typename T>
void Histogram<T>::maximumsComputing( int minimumHeightPercentageOfMaximum, int neighborhoodOfMaximums, bool loop )
{
	const int sizeHist = size();
	_maximums.clear();
	if ( sizeHist > 0 )
	{
		QVector<T> copy;
		copy.reserve(sizeHist+2*neighborhoodOfMaximums);
		if ( loop )
		{
			for ( int i=sizeHist-neighborhoodOfMaximums ; i<sizeHist ; ++i )
			{
				copy << _datas[i];
			}
			copy << _datas;
			for ( int i=0 ; i<neighborhoodOfMaximums ; ++i )
			{
				copy << _datas[i];
			}
		}
		else
		{
			for ( int i=sizeHist-neighborhoodOfMaximums ; i<sizeHist ; ++i )
			{
				copy << _datas[0];
			}
			copy << _datas;
			for ( int i=0 ; i<neighborhoodOfMaximums ; ++i )
			{
				copy << _datas[sizeHist-1];
			}
		}

		const int end = sizeHist+neighborhoodOfMaximums;
		const T thresholdOfMaximums = this->thresholdOfMaximums(minimumHeightPercentageOfMaximum);

		T value;
		int cursor;
		bool isMax;

		for ( int i=neighborhoodOfMaximums ; i<end ; ++i )
		{
			value = copy[i];
			if ( value > thresholdOfMaximums )
			{
				cursor = 1;
				do
				{
					isMax = ( (value > copy[i-cursor]) && (value > copy[i+cursor]) );
					cursor++;
				}
				while ( isMax && cursor<=neighborhoodOfMaximums );
				if ( isMax )
				{
					_maximums.append(i-neighborhoodOfMaximums);
					i+=(neighborhoodOfMaximums-1);
				}
			}
		}
	}
}

template <typename T>
void Histogram<T>::intervalsComputing( int derivativesPercentage, int minimumWidthOfIntervals, bool loop )
{
	_intervals.clear();
	if ( !_maximums.isEmpty() )
	{
		const int sizeHist = size();
		int cursorMax, cursorMin, derivativeThreshold;
		bool fusionLast, fusionFirst;
		cursorMax = -1;
		for ( int i=0 ; i<nbMaximums() ; ++i )
		{
			cursorMin = _maximums[i];
			derivativeThreshold = _datas[cursorMin]*derivativesPercentage/100.;
			while ( _datas[cursorMin] > derivativeThreshold )
			{
				cursorMin--;
				if ( cursorMin < 0 ) cursorMin = sizeHist-1;
			}
			while ( firstdDerivated(cursorMin,loop) > 0. )
			{
				cursorMin--;
				if ( cursorMin < 0 ) cursorMin = sizeHist-1;
			}

			cursorMax = _maximums[i]+1;
			if ( cursorMax == sizeHist ) cursorMax = 0;

			while ( _datas[cursorMax] > derivativeThreshold )
			{
				cursorMax++;
				if ( cursorMax == sizeHist ) cursorMax = 0;
			}
			while ( firstdDerivated(cursorMax,loop) < 0. )
			{
				cursorMax++;
				if ( cursorMax == sizeHist ) cursorMax = 0;
			}
			cursorMax--;
			if ( cursorMax<0 ) cursorMax = sizeHist-1;

			if ( cursorMax>cursorMin && (cursorMax-cursorMin) >= minimumWidthOfIntervals )
			{
				if ( _intervals.isEmpty() || _intervals.last().maxValue() <= cursorMin )
				{
					_intervals.append(Interval(cursorMin,cursorMax));
				}
				else if ( _intervals.last().isValid() )
				{
					_intervals.last().setMin( qMin(_intervals.last().minValue(), cursorMin) );
					_intervals.last().setMax( qMax(_intervals.last().maxValue(), cursorMax) );
					if ( _intervals.size() > 1 )
					{
						Interval &previousOfLast = _intervals[_intervals.size()-2];
						if ( previousOfLast.maxValue() > _intervals.last().minValue() )
						{
							previousOfLast.setMin( qMin(previousOfLast.minValue(), _intervals.last().minValue()) );
							previousOfLast.setMax( qMax(previousOfLast.maxValue(), _intervals.last().maxValue()) );
							_intervals.pop_back();
						}
					}
					cursorMin = _intervals.last().minValue();
					cursorMax = _intervals.last().maxValue();
				}
				else
				{
					_intervals.last().setMax( qMax(_intervals.last().maxValue(), cursorMax) );
					cursorMax = _intervals.last().maxValue();
				}
			}
			else if ( cursorMax<cursorMin && (sizeHist-cursorMin+cursorMax) >= minimumWidthOfIntervals )
			{
				if ( loop )
				{
					if ( _intervals.isEmpty() || (_intervals.last().isValid() && _intervals.last().maxValue() <= cursorMin && _intervals.first().isValid() && _intervals.first().minValue() >= cursorMax) )
					{
						_intervals.append(Interval(cursorMin,cursorMax));
					}
					else
					{
						fusionLast = false;
						if ( _intervals.last().isValid() && _intervals.last().maxValue() > cursorMin )
						{
							_intervals.last().setMin( qMin(_intervals.last().minValue(), cursorMin) );
							_intervals.last().setMax(cursorMax);
							fusionLast = true;
						}
						if ( !_intervals.last().isValid() )
						{
							_intervals.last().setMin( qMin(_intervals.last().minValue(), cursorMin) );
							_intervals.last().setMax( qMax(_intervals.last().maxValue(), cursorMax) );
							fusionLast = true;
						}
						if ( !fusionLast ) _intervals.append( Interval(cursorMin, cursorMax) );
						else
						{
							cursorMin = _intervals.last().minValue();
							cursorMax = _intervals.last().maxValue();
						}
						fusionFirst = false;
						if ( _intervals.first().isValid() && _intervals.first().minValue() < _intervals.last().maxValue() )
						{
							_intervals.first().setMin( _intervals.last().minValue() );
							_intervals.first().setMax( qMax(_intervals.first().maxValue(), _intervals.last().maxValue()) );
							fusionFirst = true;
						}
						if ( !_intervals.first().isValid() )
						{
							_intervals.first().setMin( qMin(_intervals.first().minValue(), _intervals.last().minValue()) );
							_intervals.first().setMax( qMax(_intervals.first().maxValue(), _intervals.last().maxValue()) );
							fusionFirst = true;
						}
						if (fusionFirst)
						{
							_intervals.pop_back();
							cursorMin = _intervals.first().minValue();
							cursorMax = _intervals.first().maxValue();
						}
					}
				}
				else if ( _intervals.isEmpty() )
				{
					_intervals.append(Interval(0,cursorMax));
					cursorMin = 0;
				}
				else
				{
					_intervals.append(Interval(cursorMin,sizeHist-1));
					cursorMax = sizeHist-1;
				}
			}
		}
		if ( !_intervals.first().isValid() && _intervals.last().isValid() && _intervals.first().minValue() < _intervals.last().maxValue() )
		{
			_intervals.first().setMin( qMin(_intervals.first().minValue(),_intervals.last().minValue()) );
			_intervals.pop_back();
		}
	}
}


template <typename T>
void Histogram<T>::computeAll( int smoothingRadius, int minimumHeightPercentageOfMaximum, int neighborhoodOfMaximums, int derivativesPercentage, int minimumWidthOfIntervals, bool loop )
{
	meansSmoothing( smoothingRadius, loop );
	maximumsComputing( minimumHeightPercentageOfMaximum, neighborhoodOfMaximums, loop );
	intervalsComputing( derivativesPercentage, minimumWidthOfIntervals, loop );
}

#endif // HISTOGRAM_H
