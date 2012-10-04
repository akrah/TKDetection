#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "interval.h"
#include <numeric>
#include <QVector>
#include <QTextStream>

/*######################################################
  # DECLARATION
  ######################################################*/

template <typename T>
class Histogram : public QVector<T>
{
public:
	Histogram();
	virtual ~Histogram();

	const QVector<int> & maximums() const;
	const QVector< Interval<int> > & intervals() const;

	T min() const;
	T max() const;
	int nbMaximums() const;
	int maximumIndex( int i ) const;
	int nbIntervals() const;
	const Interval<int> &interval( int i ) const;
	int intervalIndex( int i ) const;
	T thresholdOfMaximums( const int &percentage ) const;
	T firstdDerivated( int i, bool loop ) const;

	void computeMaximumsAndIntervals( const int & smoothingRadius,
									  const int & minimumHeightPercentageOfMaximum, const int & neighborhoodOfMaximums,
									  const int & derivativesPercentage, const int &minimumWidthOfIntervals, const bool & loop );

private:
	void meansSmoothing( const int &smoothingRadius, const bool &loop );
	void computeMaximums( const int &minimumHeightPercentageOfMaximum, const int &neighborhoodOfMaximums, const bool &loop );
	void computeIntervals( const int &derivativesPercentage, const int &minimumWidthOfIntervals, const bool &loop );

protected:
	QVector<int> _maximums;
	QVector< Interval<int> > _intervals;
};

template <typename T>
QTextStream & operator <<( QTextStream & stream, const Histogram<T> & histogram )
{
	for ( int i=0 ; i<histogram.size() ; ++i )
	{
		stream << i << " " <<  histogram[i] << endl;
	}
	return stream;
}


/*######################################################
  # IMPLEMENTATION
  ######################################################*/

/**********************************
 * Public constructors/destructors
 **********************************/

template <typename T> Histogram<T>::Histogram() {}
template <typename T> Histogram<T>::~Histogram() {}


/**********************************
 * Public getters
 **********************************/

template <typename T>
inline const QVector<int> & Histogram<T>::maximums() const
{
	return _maximums;
}

template <typename T>
inline const QVector< Interval<int> > & Histogram<T>::intervals() const
{
	return _intervals;
}

template <typename T>
T Histogram<T>::min() const
{
	typename QVector<T>::const_iterator begin = this->begin();
	typename QVector<T>::const_iterator end = this->end();

	T min = begin != end ? (*begin++) : T();
	while ( begin != end ) min = qMin(min,*begin++);

	return min;
}

template <typename T>
T Histogram<T>::max() const
{
	typename QVector<T>::const_iterator begin = this->begin();
	typename QVector<T>::const_iterator end = this->end();

	T max = begin != end ? (*begin++) : T();
	while ( begin != end ) max = qMax(max,*begin++);

	return max;
}

template <typename T>
inline int Histogram<T>::nbMaximums() const
{
	return _maximums.size();
}

template <typename T>
int Histogram<T>::maximumIndex( int i ) const
{
	Q_ASSERT_X( i>=0 && i<nbMaximums(), "Histogram::maximumIndex", "index en dehors des bornes" );
	return _maximums[i];
}

template <typename T>
inline int Histogram<T>::nbIntervals() const
{
	return _intervals.size();
}

template <typename T>
inline const Interval<int> &Histogram<T>::interval( int i ) const
{
	Q_ASSERT_X( i>=0 && i<this->nbIntervals(), "Histogram::interval", "index en dehors des bornes" );
	return _intervals[i];
}

template <typename T>
int Histogram<T>::intervalIndex( int i ) const
{
	Q_ASSERT_X( i>=0 && i<this->nbIntervals(), "Histogram::intervalIndex", "index en dehors des bornes" );
	return (_intervals[i].min()+_intervals[i].max())/2;
}

template <typename T>
T Histogram<T>::thresholdOfMaximums( const int & percentage ) const
{
	const T min = this->min();
	const T max = this->max();
	return (max-min)*(percentage/100.)+min;
}

template <typename T>
T Histogram<T>::firstdDerivated( int i, bool loop ) const
{
	Q_ASSERT_X( i>=0 && i<this->size(), "Histogram::firstDerivated", "index en dehors des bornes de l'histogramme" );
	return i>1 ? (*this)[i] - (*this)[i-2] : loop && this->size()>1 ? (*this)[i] - (*this)[this->size()-2+i] : T();
}

/**********************************
 * Public setters
 **********************************/
template <typename T>
void Histogram<T>::computeMaximumsAndIntervals( const int & smoothingRadius,
												const int & minimumHeightPercentageOfMaximum, const int & neighborhoodOfMaximums,
												const int & derivativesPercentage, const int &minimumWidthOfIntervals, const bool & loop )
{
	meansSmoothing( smoothingRadius, loop );
	computeMaximums( minimumHeightPercentageOfMaximum, neighborhoodOfMaximums, loop );
	computeIntervals( derivativesPercentage, minimumWidthOfIntervals, loop );
}

/**********************************
 * Private setters
 **********************************/
template <typename T>
void Histogram<T>::meansSmoothing( const int & smoothingRadius, const bool & loop )
{
	if ( this->size() > 0 )
	{
		const int maskWidth = 2*smoothingRadius+1;
		int i;

		QVector<T> copy;
		copy.reserve( this->size() + 2*smoothingRadius );
		if ( loop )
		{
			for ( i=this->size()-smoothingRadius ; i<this->size() ; ++i ) copy << this->at(i);
			copy << (*this);
			for ( i=0 ; i<smoothingRadius ; ++i ) copy << this->at(i);
		}
		else
		{
			for ( i=this->size()-smoothingRadius ; i<this->size() ; ++i ) copy << this->at(0);
			copy << (*this);
			for ( i=this->size()-smoothingRadius ; i<this->size() ; ++i ) copy << this->at(this->size()-1);
		}

		typename QVector<T>::const_iterator copyIterBegin = copy.begin();
		typename QVector<T>::const_iterator copyIterEnd = copyIterBegin+(2*smoothingRadius+1);

		typename QVector<T>::iterator histIter = this->begin();
		const typename QVector<T>::const_iterator histEnd = this->end();

		while ( histIter != histEnd )
		{
			*histIter++ = std::accumulate( copyIterBegin++, copyIterEnd++, T() )/static_cast<T>(maskWidth);
		}
	}
}

template <typename T>
void Histogram<T>::computeMaximums( const int & minimumHeightPercentageOfMaximum, const int & neighborhoodOfMaximums, const bool & loop )
{
	_maximums.clear();
	if ( this->size() > 0 )
	{
		QVector<T> copy;
		copy.reserve(this->size()+2*neighborhoodOfMaximums);
		if ( loop )
		{
			for ( int i=this->size()-neighborhoodOfMaximums ; i<this->size() ; ++i )
			{
				copy << this->at(i);
			}
			copy << (*this);
			for ( int i=0 ; i<neighborhoodOfMaximums ; ++i )
			{
				copy << this->at(i);
			}
		}
		else
		{
			for ( int i=this->size()-neighborhoodOfMaximums ; i<this->size() ; ++i )
			{
				copy << this->at(0);
			}
			copy << (*this);
			for ( int i=0 ; i<neighborhoodOfMaximums ; ++i )
			{
				copy << this->at(this->size()-1);
			}
		}

		const int end = this->size()+neighborhoodOfMaximums;
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
void Histogram<T>::computeIntervals( const int & derivativesPercentage, const int & minimumWidthOfIntervals, const bool & loop )
{
	_intervals.clear();
	if ( !_maximums.isEmpty() )
	{
		int cursorMax, cursorMin, derivativeThreshold;
		bool fusionLast, fusionFirst;
		cursorMax = -1;
		for ( int i=0 ; i<nbMaximums() ; ++i )
		{
			cursorMin = _maximums[i];
			derivativeThreshold = this->at(cursorMin)*derivativesPercentage/100.;
			while ( this->at(cursorMin) > derivativeThreshold )
			{
				cursorMin--;
				if ( cursorMin < 0 ) cursorMin = this->size()-1;
			}
			while ( firstdDerivated(cursorMin,loop) > 0. )
			{
				cursorMin--;
				if ( cursorMin < 0 ) cursorMin = this->size()-1;
			}

			cursorMax = _maximums[i]+1;
			if ( cursorMax == this->size() ) cursorMax = 0;

			while ( this->at(cursorMax) > derivativeThreshold )
			{
				cursorMax++;
				if ( cursorMax == this->size() ) cursorMax = 0;
			}
			while ( firstdDerivated(cursorMax,loop) < 0. )
			{
				cursorMax++;
				if ( cursorMax == this->size() ) cursorMax = 0;
			}
			cursorMax--;
			if ( cursorMax<0 ) cursorMax = this->size()-1;

			if ( cursorMax>cursorMin && (cursorMax-cursorMin) >= minimumWidthOfIntervals )
			{
				if ( _intervals.isEmpty() || _intervals.last().max() <= cursorMin )
				{
					_intervals.append(Interval<int>(cursorMin,cursorMax));
				}
				else if ( _intervals.last().isValid() )
				{
					_intervals.last().setMin( qMin(_intervals.last().min(), cursorMin) );
					_intervals.last().setMax( qMax(_intervals.last().max(), cursorMax) );
					if ( _intervals.size() > 1 )
					{
						Interval<int> &previousOfLast = _intervals[_intervals.size()-2];
						if ( previousOfLast.max() > _intervals.last().min() )
						{
							previousOfLast.setMin( qMin(previousOfLast.min(), _intervals.last().min()) );
							previousOfLast.setMax( qMax(previousOfLast.max(), _intervals.last().max()) );
							_intervals.pop_back();
						}
					}
					cursorMin = _intervals.last().min();
					cursorMax = _intervals.last().max();
				}
				else
				{
					_intervals.last().setMax( qMax(_intervals.last().max(), cursorMax) );
					cursorMax = _intervals.last().max();
				}
			}
			else if ( cursorMax<cursorMin && (this->size()-cursorMin+cursorMax) >= minimumWidthOfIntervals )
			{
				if ( loop )
				{
					if ( _intervals.isEmpty() || (_intervals.last().isValid() && _intervals.last().max() <= cursorMin && _intervals.first().isValid() && _intervals.first().min() >= cursorMax) )
					{
						_intervals.append(Interval<int>(cursorMin,cursorMax));
					}
					else
					{
						fusionLast = false;
						if ( _intervals.last().isValid() && _intervals.last().max() > cursorMin )
						{
							_intervals.last().setMin( qMin(_intervals.last().min(), cursorMin) );
							_intervals.last().setMax(cursorMax);
							fusionLast = true;
						}
						if ( !_intervals.last().isValid() )
						{
							_intervals.last().setMin( qMin(_intervals.last().min(), cursorMin) );
							_intervals.last().setMax( qMax(_intervals.last().max(), cursorMax) );
							fusionLast = true;
						}
						if ( !fusionLast ) _intervals.append( Interval<int>(cursorMin, cursorMax) );
						else
						{
							cursorMin = _intervals.last().min();
							cursorMax = _intervals.last().max();
						}
						fusionFirst = false;
						if ( _intervals.first().isValid() && _intervals.first().min() < _intervals.last().max() )
						{
							_intervals.first().setMin( _intervals.last().min() );
							_intervals.first().setMax( qMax(_intervals.first().max(), _intervals.last().max()) );
							fusionFirst = true;
						}
						if ( !_intervals.first().isValid() )
						{
							_intervals.first().setMin( qMin(_intervals.first().min(), _intervals.last().min()) );
							_intervals.first().setMax( qMax(_intervals.first().max(), _intervals.last().max()) );
							fusionFirst = true;
						}
						if (fusionFirst)
						{
							_intervals.pop_back();
							cursorMin = _intervals.first().min();
							cursorMax = _intervals.first().max();
						}
					}
				}
				else if ( _intervals.isEmpty() )
				{
					_intervals.append(Interval<int>(0,cursorMax));
					cursorMin = 0;
				}
				else
				{
					_intervals.append(Interval<int>(cursorMin,this->size()-1));
					cursorMax = this->size()-1;
				}
			}
		}
		if ( !_intervals.first().isValid() && _intervals.last().isValid() && _intervals.first().min() < _intervals.last().max() )
		{
			_intervals.first().setMin( qMin(_intervals.first().min(),_intervals.last().min()) );
			_intervals.pop_back();
		}
	}
}

#endif // HISTOGRAM_H
