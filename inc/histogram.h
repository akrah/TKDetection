#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "interval.h"
#include "inc/globalfunctions.h"

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
	const QVector< Interval<uint> > &intervals() const;

	T min() const;
	T max() const;
	uint nbMaximums() const;
	uint maximumIndex( uint i ) const;
	uint nbIntervals() const;
	const Interval<uint> &interval( uint i ) const;
	uint intervalIndex( uint i ) const;
	T thresholdOfMaximums( const int &percentage ) const;
	T firstdDerivated( int i, bool loop ) const;

	void clear();
	void computeMaximumsAndIntervals(const uint &smoothingRadius, const int & minimumHeightPercentageOfMaximum,
									  const int & derivativesPercentage, const int &minimumWidthOfIntervals, const bool & loop );

protected:
	virtual void computeMaximums( const int &minimumHeightPercentageOfMaximum, const int &neighborhoodOfMaximums, const bool &loop );
	virtual void computeIntervals(const int &derivativesPercentage, const uint &minimumWidthOfIntervals, const bool &loop );

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
const QVector<uint> & Histogram<T>::maximums() const
{
	return _maximums;
}

template <typename T>
const QVector< Interval<uint> > & Histogram<T>::intervals() const
{
	return _intervals;
}

template <typename T>
T Histogram<T>::min() const
{
	typename QVector<T>::const_iterator begin = this->begin();
	const typename QVector<T>::const_iterator end = this->end();

	T min = begin != end ? (*begin++) : T();
	while ( begin != end ) min = qMin(min,*begin++);

	return min;
}

template <typename T>
T Histogram<T>::max() const
{
	typename QVector<T>::const_iterator begin = this->begin();
	const typename QVector<T>::const_iterator end = this->end();

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
uint Histogram<T>::maximumIndex( uint i ) const
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
	//return i>0 ? (*this)[i] - (*this)[i-1] : loop ? (*this)[0] - (*this)[this->size()-1] : T();
	return i && i<this->size()-1 ? (*this)[i+1] - (*this)[i-1] :
								   loop ? ( i ? (*this)[0] - (*this)[this->size()-2] : (*this)[1] - (*this)[this->size()-1] ) :
										  T() ;
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
	TKD::meanSmoothing<T>( this->begin(), this->end(), smoothingRadius, loop );
	/******************************************************/
	/* Soustraction de la valeur min à toutes les valeurs */
	typename QVector<T>::iterator begin = this->begin();
	typename QVector<T>::const_iterator end = this->end();
	T min = this->min();
	while ( begin != end ) (*begin++) -= min;
	/******************************************************/
	computeMaximums( minimumHeightPercentageOfMaximum, minimumWidthOfIntervals/2, loop );
	computeIntervals( derivativesPercentage, minimumWidthOfIntervals, loop );
}

/**********************************
 * Private setters
 **********************************/
template <typename T>
void Histogram<T>::computeMaximums( const int & minimumHeightPercentageOfMaximum, const int & neighborhoodOfMaximums, const bool & loop )
{
	_maximums.clear();
	int size = this->size();
	if ( size > 0 )
	{
		QVector<T> copy;
		copy.reserve(this->size()+2*neighborhoodOfMaximums);
		if ( loop )
		{
			for ( int i=this->size()-neighborhoodOfMaximums ; i<this->size() ; ++i )
			{
				copy << (*this)[i];
			}
			copy << (*this);
			for ( int i=0 ; i<neighborhoodOfMaximums ; ++i )
			{
				copy << (*this)[i];
			}
		}
		else
		{
			for ( int i=this->size()-neighborhoodOfMaximums ; i<this->size() ; ++i )
			{
				copy << (*this)[0];
			}
			copy << (*this);
			for ( int i=0 ; i<neighborhoodOfMaximums ; ++i )
			{
				copy << (*this)[size-1];
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
				cursor = qMin(2,neighborhoodOfMaximums);
				isMax = false;
				do
				{
					isMax = ( (value >= copy[i-cursor]) && (value >= copy[i+cursor]) );
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

	uint cursorMax, cursorMin, cursorStart;
	T derivativeThreshold;
	Interval<uint> cursor;
	const uint histoSize = this->size();
	const uint histoSizeMinusOne = histoSize - 1;

	for ( int i=0 ; i<static_cast<int>(nbMaximums()) ; ++i )
	{
		// Detection des bornes de l'intervalle courant
		cursorMin = cursorMax =_maximums[i];

		// Détection de la borne inf de l'intervalle
		cursorStart = cursorMin+1;
		if ( cursorStart == histoSize )
		{
			if ( loop ) cursorStart = 0;
			else cursorStart = cursorMin;
		}
		derivativeThreshold = (*this)[cursorMin]*derivativesPercentage/100.;
		while ( cursorMin != cursorStart && (*this)[cursorMin] > derivativeThreshold )
		{
			if ( cursorMin ) cursorMin--;
			else if ( loop ) cursorMin = histoSizeMinusOne;
			else break;
		}
		while ( cursorMin != cursorStart && firstdDerivated(cursorMin,loop) > 0.5 )
		{
			if ( cursorMin ) cursorMin--;
			else if ( loop ) cursorMin = histoSizeMinusOne;
			else break;
		}

		// Détection de la borne sup de l'intervalle
		cursorStart = cursorMax;
		if ( cursorStart ) cursorStart--;
		else if ( loop ) cursorStart = histoSizeMinusOne;

		while ( cursorMax != cursorStart && (*this)[cursorMax] > derivativeThreshold )
		{
			cursorMax++;
			if ( cursorMax == histoSize )
			{
				if ( loop ) cursorMax = 0;
				else
				{
					cursorMax--;
					break;
				}
			}
		}
		while ( cursorMax != cursorStart && firstdDerivated(cursorMax,loop) < -0.5 )
		{
			cursorMax++;
			if ( cursorMax == histoSize )
			{
				if ( loop ) cursorMax = 0;
				else
				{
					cursorMax--;
					break;
				}
			}
		}
		if ( cursorMax ) cursorMax--;
		else if (loop) cursorMax = histoSizeMinusOne;

		cursor.setBounds(cursorMin,cursorMax);

		// Ajout et fusion de l'intervalle courant
		if ( _intervals.isEmpty() )
		{
			if ( (cursor.isValid() ? cursor.width() : histoSize-(cursor.min()-cursor.max())) > minimumWidthOfIntervals ) _intervals.append(cursor);
		}
		else
		{
			const Interval<uint> &last = _intervals.last();
			if ( last.max() >= cursorMax && !(!cursor.isValid() && last.isValid()) )
			{
				cursor = last;
				cursorMin = last.min();
				cursorMax = last.max();
				if ( (*this)[_maximums[i]]>(*this)[_maximums[i-1]] ) _maximums[i-1] = _maximums[i];
				_maximums.remove(i--);
			}
			else
			{
				if ( !(cursor.isValid() || last.isValid()) || last.max() >= cursor.min() )
				{
					cursorMin = last.max();
					cursor.setMin(cursorMin);
				}
				if ( (cursor.isValid() ? cursor.width() : histoSize-(cursor.min()-cursor.max())) > minimumWidthOfIntervals ) _intervals.append(cursor);
				else
				{
					if ( (*this)[_maximums[i]]>(*this)[_maximums[i-1]] ) _maximums[i-1] = _maximums[i];
					_maximums.remove(i--);
				}
			}
		}
	}
	if ( _intervals.size() > 1 && _intervals.last() == _intervals.first() ) _intervals.pop_back();
}

#endif // HISTOGRAM_H
