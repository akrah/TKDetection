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
	Histogram( const QVector<T> &vector );
	Histogram( const Histogram<T> &histogram );
	virtual ~Histogram();

	const QVector<uint> &maximums() const;
	const QVector<Interval<uint> > &intervals() const;

	T min() const;
	T max() const;
	uint nbMaximums() const;
	int maximumIndex( uint i ) const;
	uint nbIntervals() const;
	const Interval<uint> &interval( uint i ) const;
	int intervalIndex( uint i ) const;
	T thresholdOfMaximums( const int &percentage ) const;
	T firstdDerivated( int i, bool loop ) const;

	void clear();
	void computeMaximumsAndIntervals(const uint &smoothingRadius, const int & minimumHeightPercentageOfMaximum,
									  const int & derivativesPercentage, const int &minimumWidthOfIntervals, const bool & loop );

protected:
	virtual void meansSmoothing( const uint &smoothingRadius, const bool &loop );
	virtual void computeMaximums( const int &minimumHeightPercentageOfMaximum, const int &neighborhoodOfMaximums, const bool &loop );
	virtual void computeIntervals( const int &derivativesPercentage, const uint &minimumWidthOfIntervals, const bool &loop );

protected:
	QVector<uint> _maximums;
	QVector< Interval<uint> > _intervals;
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

template <typename T> Histogram<T>::Histogram() : QVector<T>() {}
template <typename T> Histogram<T>::Histogram( const QVector<T> &vector ) : QVector<T>(vector) {}
template <typename T> Histogram<T>::Histogram( const Histogram<T> &histogram ) : QVector<T>(histogram), _maximums(histogram._maximums), _intervals(histogram._intervals) {}
template <typename T> Histogram<T>::~Histogram() {}


/**********************************
 * Public getters
 **********************************/

template <typename T>
inline const QVector<uint> & Histogram<T>::maximums() const
{
	return _maximums;
}

template <typename T>
inline const QVector< Interval<uint> > & Histogram<T>::intervals() const
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
inline uint Histogram<T>::nbMaximums() const
{
	return _maximums.size();
}

template <typename T>
int Histogram<T>::maximumIndex( uint i ) const
{
	Q_ASSERT_X( i<nbMaximums(), "Histogram::maximumIndex", "index en dehors des bornes" );
	return _maximums[i];
}

template <typename T>
inline uint Histogram<T>::nbIntervals() const
{
	return _intervals.size();
}

template <typename T>
inline const Interval<uint> &Histogram<T>::interval( uint i ) const
{
	Q_ASSERT_X( i<this->nbIntervals(), "Histogram::interval", "index en dehors des bornes" );
	return _intervals[i];
}

template <typename T>
int Histogram<T>::intervalIndex( uint i ) const
{
	Q_ASSERT_X( i<this->nbIntervals(), "Histogram::intervalIndex", "index en dehors des bornes" );
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
	return i>0 ? (*this)[i] - (*this)[i-1] : loop ? (*this)[0] - (*this)[this->size()-1] : T();
}

/**********************************
 * Public setters
 **********************************/
template <typename T>
void Histogram<T>::clear()
{
	QVector<T>::clear();
	_maximums.clear();
	_intervals.clear();
}

template <typename T>
void Histogram<T>::computeMaximumsAndIntervals( const uint & smoothingRadius, const int & minimumHeightPercentageOfMaximum,
												const int & derivativesPercentage, const int &minimumWidthOfIntervals, const bool & loop )
{
	meansSmoothing( smoothingRadius, loop );
	computeMaximums( minimumHeightPercentageOfMaximum, minimumWidthOfIntervals/2, loop );
	computeIntervals( derivativesPercentage, minimumWidthOfIntervals, loop );
}

/**********************************
 * Private setters
 **********************************/
template <typename T>
void Histogram<T>::meansSmoothing( const uint & smoothingRadius, const bool & loop )
{
	const uint histoSize = this->size();
	if ( histoSize > 0 )
	{
		const T maskWidth = 2*smoothingRadius+1;
		uint i;

		QVector<T> copy;
		copy.reserve( histoSize + 2*smoothingRadius );
		if ( loop )
		{
			for ( i=histoSize-smoothingRadius ; i<histoSize ; ++i ) copy << this->at(i);
			copy << (*this);
			for ( i=0 ; i<smoothingRadius ; ++i ) copy << this->at(i);
		}
		else
		{
			for ( i=0 ; i<smoothingRadius ; ++i ) copy << this->at(0);
			copy << (*this);
			for ( i=0 ; i<smoothingRadius ; ++i ) copy << this->at(histoSize-1);
		}

		typename QVector<T>::ConstIterator copyIterBegin = copy.constBegin();
		typename QVector<T>::ConstIterator copyIterEnd = copy.constBegin() + static_cast<int>(maskWidth);

		typename QVector<T>::Iterator histIter = this->begin();
		const typename QVector<T>::ConstIterator histEnd = this->constEnd();

		T currentValue = std::accumulate( copyIterBegin, copyIterEnd, T() );
		*histIter++ = currentValue/maskWidth;
		while ( histIter != histEnd )
		{
			currentValue += (*copyIterEnd++ - *copyIterBegin++);
			*histIter++ = currentValue/maskWidth;
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
void Histogram<T>::computeIntervals( const int & derivativesPercentage, const uint & minimumWidthOfIntervals, const bool & loop )
{
	_intervals.clear();
	if ( _maximums.isEmpty() ) return;

	uint cursorMax, cursorMin, cursorStart, derivativeThreshold;
	Interval<uint> cursor;
	const uint histoSize = this->size();
	const uint histoSizeMinusOne = histoSize - 1;

	for ( uint i=0 ; i<nbMaximums() ; ++i )
	{
		// Detection des bornes de l'intervalle courant
		cursorMin = _maximums[i];
		cursorStart = cursorMin+1;
		if ( cursorStart == histoSize ) cursorStart = 0;
		derivativeThreshold = this->at(cursorMin)*derivativesPercentage/100.;
		while ( cursorMin != cursorStart && this->at(cursorMin) > derivativeThreshold )
		{
			if ( cursorMin ) cursorMin--;
			else cursorMin = histoSize-1;
		}
		while ( firstdDerivated(cursorMin,loop) > 0. )
		{
			if ( cursorMin ) cursorMin--;
			else cursorMin = histoSizeMinusOne;
		}

		cursorMax = cursorStart;
		if ( cursorStart ) cursorStart--;
		else cursorStart = histoSize-1;
		while ( cursorMax != cursorStart && this->at(cursorMax) > derivativeThreshold )
		{
			cursorMax++;
			if ( cursorMax == histoSize ) cursorMax = 0;
		}
		while ( firstdDerivated(cursorMax,loop) < 0. )
		{
			cursorMax++;
			if ( cursorMax == histoSize ) cursorMax = 0;
		}
		if ( cursorMax ) cursorMax--;
		else cursorMax = histoSizeMinusOne;

		cursor.setBounds(cursorMin,cursorMax);

		// Ajout et fusion de l'intervalle courant
		if ( cursor.isValid() )
		{
			if ( cursor.width() >= minimumWidthOfIntervals )
			{
				if ( _intervals.isEmpty() || cursor.min() >= _intervals.last().max() )
				{
					_intervals.append(cursor);
				}
				else
				{
					cursorMin = _intervals.last().max();
					if ( cursorMax > cursorMin )
					{
						cursor.setMin(cursorMin);
						if ( cursor.width() > minimumWidthOfIntervals ) _intervals.append(cursor);
					}
					cursorMin = _intervals.last().min();
				}
			}
		}
		else
		{
			if ( loop )
			{
				if ( histoSize-(cursor.min()-cursor.max()) >= minimumWidthOfIntervals )
				{
					if ( _intervals.isEmpty()  )
					{
						_intervals.append(cursor);
					}
					else
					{
						const uint &lastMax = _intervals.last().max();
						const uint &firstMin = _intervals.first().min();
						if ( cursorMin < lastMax )
						{
							cursorMin = lastMax;
							cursor.setMin(lastMax);
						}
						if ( cursorMax > firstMin )
						{
							cursorMax = firstMin;
							cursor.setMax(firstMin);
						}
						if ( histoSize-(cursor.min()-cursor.max()) >= minimumWidthOfIntervals ) _intervals.append(cursor);
					}
				}
			}
			else
			{
				if ( _intervals.isEmpty()  )
				{
					cursor.setMin(0);
					cursorMin = 0;
					if ( cursor.width() > minimumWidthOfIntervals ) _intervals.append(cursor);
				}
				else
				{
					cursorMax = histoSizeMinusOne;
					cursor.setMax(cursorMax);
					if ( _intervals.last().max() > cursorMin )
					{
						cursorMin = _intervals.last().max();
						cursor.setMin(cursorMin);
					}
					if ( cursor.width() > minimumWidthOfIntervals )	_intervals.append(cursor);
				}
			}
		}
	}
}

#endif // HISTOGRAM_H
