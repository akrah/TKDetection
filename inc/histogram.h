#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "interval.h"
#include "inc/globalfunctions.h"

#include <QVector>
#include <QTextStream>
#include <QDebug>

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
	T thresholdOfMaximums() const;
	T firstdDerivated( int i, bool loop ) const;

	const uint &smoothingRadius() const;
	const qreal &minimumHeightPercentageOfMaximum() const;
	const qreal &derivativesPercentage() const;
	const uint &minimumIntervalWidth() const;

	void setSmoothingRadius( const uint &radius );
	void setMinimumHeightPercentageOfMaximum( const uint &percentage );
	void setDerivativesPercentage( const uint &percentage );
	void setMinimumIntervalWidth( const uint &width );

	void clear();
	void computeMaximumsAndIntervals( const bool & loop );

protected:
	virtual void computeMaximums( const bool &loop );
	virtual void computeIntervals( const bool &loop );

protected:
	QVector<uint> _maximums;
	QVector< Interval<uint> > _intervals;

	uint _smoothingRadius;
	qreal _minimumHeightPercentageOfMaximum;
	qreal _derivativesPercentage;
	uint _minimumIntervalWidth;
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
T Histogram<T>::thresholdOfMaximums() const
{
	const T min = this->min();
	const T max = this->max();
	return (max-min)*_minimumHeightPercentageOfMaximum+min;
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

template <typename T>
const uint &Histogram<T>::smoothingRadius() const
{
	return _smoothingRadius;
}

template <typename T>
const qreal &Histogram<T>::minimumHeightPercentageOfMaximum() const
{
	return _minimumHeightPercentageOfMaximum;
}

template <typename T>
const qreal &Histogram<T>::derivativesPercentage() const
{
	return _derivativesPercentage;
}

template <typename T>
const uint &Histogram<T>::minimumIntervalWidth() const
{
	return _minimumIntervalWidth;
}


template <typename T>
void Histogram<T>::setSmoothingRadius( const uint &radius )
{
	_smoothingRadius = radius;
}

template <typename T>
void Histogram<T>::setMinimumHeightPercentageOfMaximum( const uint &percentage )
{
	_minimumHeightPercentageOfMaximum = percentage/100.;
}

template <typename T>
void Histogram<T>::setDerivativesPercentage( const uint &percentage )
{
	Q_ASSERT_X( percentage <= 100 , "Histogram<T>::setDerivativesPercentage", "percentage doit être compris entre 0 et 100." );
	_derivativesPercentage = percentage/100.;
}

template <typename T>
void Histogram<T>::setMinimumIntervalWidth( const uint &width )
{
	_minimumIntervalWidth = width;
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
void Histogram<T>::computeMaximumsAndIntervals( const bool & loop )
{
	TKD::meanSmoothing<T>( this->begin(), this->end(), _smoothingRadius, loop );
	/******************************************************/
	/* Soustraction de la valeur min à toutes les valeurs */
	typename QVector<T>::iterator begin = this->begin();
	typename QVector<T>::const_iterator end = this->end();
	T min = this->min();
	while ( begin != end ) (*begin++) -= min;
	/******************************************************/
	computeMaximums( loop );
	computeIntervals( loop );
}

/**********************************
 * Private setters
 **********************************/
template <typename T>
void Histogram<T>::computeMaximums( const bool & loop )
{
	_maximums.clear();
	int size = this->size();
	if ( size > 0 )
	{
		const uint neighborhoodOfMaximums = _minimumIntervalWidth/2;
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
		const T thresholdOfMaximums = this->thresholdOfMaximums();

		T value;
		int cursor;
		bool isMax;

		for ( int i=neighborhoodOfMaximums ; i<end ; ++i )
		{
			value = copy[i];
			if ( value > thresholdOfMaximums )
			{
				cursor = qMin((uint)2,neighborhoodOfMaximums);
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

//template <typename T>
//void Histogram<T>::computeIntervals( const bool & loop )
//{
//	_intervals.clear();
//	if ( _maximums.isEmpty() ) return;

//	uint cursorMax, cursorMin, cursorEnd;
//	T derivativeThreshold;
//	Interval<uint> cursor;
//	const uint histoSize = this->size();
//	const uint histoSizeMinusOne = histoSize - 1;

//	for ( int i=0 ; i<static_cast<int>(nbMaximums()) ; ++i )
//	{
//		// Detection des bornes de l'intervalle courant
//		cursorMin = cursorMax =_maximums[i];
//		derivativeThreshold = (*this)[cursorMin]*_derivativesPercentage;

//		// Détection de la borne inf de l'intervalle
//		cursorEnd = cursorMin+1;
//		if ( cursorEnd == histoSize )
//		{
//			if ( loop ) cursorEnd = 0;
//			else cursorEnd = cursorMin;
//		}
//		// Tant que cursorMin n'est pas revenu au point de départ et que les valeurs sont supérieures au seuil, on recule cursorMin
//		while ( cursorMin != cursorEnd && (*this)[cursorMin] > derivativeThreshold )
//		{
//			if ( cursorMin ) cursorMin--;
//			else if ( loop ) cursorMin = histoSizeMinusOne; // Si on arrive a 0, on boucle seuelement si loop est vrai
//			else break;
//		}
//		// On continue à rechercher la borne min tant qu'on est pas revenu au point de départ et que la dérivée est supérieure à 0.5
//		while ( cursorMin != cursorEnd && firstdDerivated(cursorMin,loop) > 0.5 )
//		{
//			if ( cursorMin ) cursorMin--;
//			else if ( loop ) cursorMin = histoSizeMinusOne;
//			else break;
//		}

//		// Détection de la borne sup de l'intervalle
//		cursorEnd = cursorMax;
//		if ( cursorEnd ) cursorEnd--;
//		else if ( loop ) cursorEnd = histoSizeMinusOne;
//		// Tant que cursorMax n'est pas revenu au point de départ et que les valeurs sont supérieures au seuil, on avance cursorMax
//		while ( cursorMax != cursorEnd && (*this)[cursorMax] > derivativeThreshold )
//		{
//			cursorMax++;
//			if ( cursorMax == histoSize )
//			{
//				if ( loop ) cursorMax = 0; // Si cursorMax atteint la fin de l'histogramme, on boucle seulement si loop est vrai
//				else
//				{
//					cursorMax--;
//					break;
//				}
//			}
//		}
//		// On continue à rechercher la borne min tant qu'on est pas revenu au point de départ et que la dérivée est inférieure à -0.5
//		while ( cursorMax != cursorEnd && firstdDerivated(cursorMax,loop) < -0.5 )
//		{
//			cursorMax++;
//			if ( cursorMax == histoSize )
//			{
//				if ( loop ) cursorMax = 0; // Si cursorMax atteint la fin de l'histogramme, on boucle seulement si loop est vrai
//				else
//				{
//					cursorMax--;
//					break;
//				}
//			}
//		}
//		if ( cursorMax ) cursorMax--;
//		else if (loop) cursorMax = histoSizeMinusOne;

//		cursor.setBounds(cursorMin,cursorMax);

//		// Ajout et fusion de l'intervalle courant
//		if ( _intervals.isEmpty() )
//		{
//			if ( (cursor.isValid() ? cursor.width() : histoSize-(cursor.min()-cursor.max())) > _minimumIntervalWidth ) _intervals.append(cursor);
////			qDebug() << "** Interval " << i << " : [" << cursor.min() << ", " << cursor.max() << "]";
//		}
//		else
//		{
//			const Interval<uint> &last = _intervals.last();
//			if ( last.max() >= cursorMax && !(!cursor.isValid() && last.isValid()) )
//			{
////				qDebug() << "** Interval " << i << " : [" << cursor.min() << ", " << cursor.max() << "] => [" << last.min() << ", " << last.max() << "]";
//				cursor = last;
//				cursorMin = last.min();
//				cursorMax = last.max();
//				if ( (*this)[_maximums[i]]>(*this)[_maximums[i-1]] ) _maximums[i-1] = _maximums[i];
//				_maximums.remove(i--);
//			}
//			else
//			{
////				qDebug() << "** Interval " << i << " : [" << cursor.min() << ", " << cursor.max() << "] from [" << last.min() << ", " << last.max() << "]";
//				if ( !(cursor.isValid() || last.isValid()) || last.max() > cursor.min() )
//				{
//					cursorMin = last.max();
//					cursor.setMin(cursorMin);
////					qDebug() << "** => New " << i << " : [" << cursor.min() << ", " << cursor.max() << "]";
//				}
//				if ( (cursor.isValid() ? cursor.width() : histoSize-(cursor.min()-cursor.max())) > _minimumIntervalWidth )
//				{
////					qDebug() << "** => Ok";
//					_intervals.append(cursor);
//				}
//				else
//				{
////					qDebug() << "** => Garde l'ancien";
//					if ( (*this)[_maximums[i]]>(*this)[_maximums[i-1]] ) _maximums[i-1] = _maximums[i];
//					_maximums.remove(i--);
//				}
//			}
//		}
//	}
//	if ( _intervals.size() > 1 && _intervals.last() == _intervals.first() ) _intervals.pop_back();
//}


template <typename T>
void Histogram<T>::computeIntervals( const bool & loop )
{
	_intervals.clear();
	if ( _maximums.isEmpty() ) return;

	uint cursorMax, cursorMin, cursorEnd;
	T derivativeThreshold;
	Interval<uint> cursor;
	const uint histoSize = this->size();
	const uint histoSizeMinusOne = histoSize - 1;

	for ( uint i=0 ; i<nbMaximums() ; ++i )
	{
		// Detection des bornes de l'intervalle courant
		cursorMin = cursorMax =_maximums[i];
		derivativeThreshold = (*this)[cursorMin]*_derivativesPercentage;

		// Détection de la borne inf de l'intervalle
		cursorEnd = cursorMin+1;
		if ( cursorEnd == histoSize )
		{
			if ( loop ) cursorEnd = 0;
			else cursorEnd = cursorMin;
		}
		// Tant que cursorMin n'est pas revenu au point de départ et que les valeurs sont supérieures au seuil, on recule cursorMin
		while ( cursorMin != cursorEnd && (*this)[cursorMin] > derivativeThreshold )
		{
			if ( cursorMin ) cursorMin--;
			else if ( loop ) cursorMin = histoSizeMinusOne; // Si on arrive a 0, on boucle seuelement si loop est vrai
			else break;
		}
		// On continue à rechercher la borne min tant qu'on est pas revenu au point de départ et que la dérivée est supérieure à 0.5
		while ( cursorMin != cursorEnd && firstdDerivated(cursorMin,loop) > 0.5 )
		{
			if ( cursorMin ) cursorMin--;
			else if ( loop ) cursorMin = histoSizeMinusOne;
			else break;
		}

		// Détection de la borne sup de l'intervalle
		cursorEnd = cursorMax;
		if ( cursorEnd ) cursorEnd--;
		else if ( loop ) cursorEnd = histoSizeMinusOne;
		// Tant que cursorMax n'est pas revenu au point de départ et que les valeurs sont supérieures au seuil, on avance cursorMax
		while ( cursorMax != cursorEnd && (*this)[cursorMax] > derivativeThreshold )
		{
			cursorMax++;
			if ( cursorMax == histoSize )
			{
				if ( loop ) cursorMax = 0; // Si cursorMax atteint la fin de l'histogramme, on boucle seulement si loop est vrai
				else
				{
					cursorMax--;
					break;
				}
			}
		}
		// On continue à rechercher la borne min tant qu'on est pas revenu au point de départ et que la dérivée est inférieure à -0.5
		while ( cursorMax != cursorEnd && firstdDerivated(cursorMax,loop) < -0.5 )
		{
			cursorMax++;
			if ( cursorMax == histoSize )
			{
				if ( loop ) cursorMax = 0; // Si cursorMax atteint la fin de l'histogramme, on boucle seulement si loop est vrai
				else
				{
					cursorMax--;
					break;
				}
			}
		}
		if ( cursorMax ) cursorMax--;
		else if (loop) cursorMax = histoSizeMinusOne;

		if ( cursorMin<cursorMax )
		{
			cursor.setBounds(cursorMin,cursorMax);
		}
		else
		{
			cursor.setBounds(cursorMin,cursorMax+histoSize);
		}

		// Ajout et fusion de l'intervalle courant
		if ( _intervals.isEmpty() )
		{
			if ( cursor.width() > _minimumIntervalWidth ) _intervals.append(cursor);
//			qDebug() << "** Interval " << i << " : [" << cursor.min() << ", " << cursor.max() << "]";
		}
		else
		{
			const Interval<uint> &last = _intervals.last();
			// Si l'intervalle courant contient le précédent
			if ( cursor.containsClosed( last ) )
			{
//				qDebug() << "** Interval " << i << " : [" << cursor.min() << ", " << cursor.max() << "] => [" << last.min() << ", " << last.max() << "]";
				_intervals.last().setBounds(cursorMin,cursorMax);
				_maximums.remove(--i);
			}
			// Si l'intervalle précédent contient l'intervalle courant
			else if ( last.containsClosed( cursor ) )
			{
				_maximums.remove(i--);
			}
			else
			{
				// Si l'intervalle courant et le précédent s'intersectent
				if ( last.intersect(cursor) )
				{
					// Si le maximum précédent est plus grand que le maximum courant
					if ( _maximums[i-1] > _maximums[i] )
					{
						cursor.setMin(last.max());
					}
					// Si le maximum courant est supérieur au maximum précédent
					else
					{
						_intervals.last().setMax(cursorMin);
						if ( _intervals.last().width() <= _minimumIntervalWidth )
						{
							_intervals.pop_back();
							_maximums.remove(--i);
						}
					}
				}
				// Dans tous les cas on ajoute l'intervalle courant s'il a la taille minimum requise
				if ( cursor.width() > _minimumIntervalWidth )
				{
//					qDebug() << "** => Ok";
					_intervals.append(cursor);
				}
				// Sinon on supprime le maximum correspondant
				else
				{
					_maximums.remove(i--);
				}
			}
		}
	}
	if ( _intervals.size() > 1 && _intervals.last() == _intervals.first() )
	{
		_intervals.pop_back();
		_maximums.pop_back();
	}
	// Si l'histogramme est circulaire et que le premier intervalle contient 0
	if ( loop && this->size() > 1 && _intervals.first().max() > this->size() && _intervals.last().max() > this->size() )
	{
		const Interval<uint> &first = _intervals.first();
		const Interval<uint> &last = _intervals.last();
		// Si le premier intervalle contient le dernier
		if ( first.containsClosed( last ) )
		{
//				qDebug() << "** Interval " << i << " : [" << cursor.min() << ", " << cursor.max() << "] => [" << last.min() << ", " << last.max() << "]";
			_intervals.pop_back();
			_maximums.pop_back();
		}
		// Si le dernier intervalle contient le premier
		else if ( last.containsClosed( first ) )
		{
			_intervals.pop_front();
			_maximums.pop_front();
		}
		else
		{
			// Si le premier intervalle et le dernier s'intersectent
			if ( last.intersect(first) )
			{
				// Si le premier maximum est plus grand que le dernier
				if ( _maximums.first() > _maximums.last() )
				{
					_intervals.last().setMin(first.max());
					if ( _intervals.last().width() <= _minimumIntervalWidth )
					{
						_intervals.pop_back();
						_maximums.pop_back();
					}
				}
				// Si le dernier maximum est plus grand que le premier
				else
				{
					_intervals.first().setMax(last.min());
					if ( _intervals.first().width() <= _minimumIntervalWidth )
					{
						_intervals.pop_front();
						_maximums.pop_front();
					}
				}
			}
		}
	}
}

#endif // HISTOGRAM_H
