#ifndef GLOBALFUNCTIONS_H
#define GLOBALFUNCTIONS_H

#include "def/def_billon.h"
#include <numeric>
#include <QtGlobal>
#include <QVector>

template <typename T> class Interval;

namespace TKD
{
	__billon_type__ restrictedValue( __billon_type__ value , const Interval<int> &intensityInterval );

	template<typename T >
	void meanSmoothingOld( const typename QVector<T>::iterator &begin, const typename QVector<T>::iterator &end, const uint &smoothingRadius, const bool &loop = false )
	{
		if ( !smoothingRadius || begin == end )
			return;

		const qreal smoothingDiameter = 2*smoothingRadius+1;

		uint i;
		typename QVector<T>::iterator dataIter, dataEnd;

		QVector<T> copy;
		if ( loop )
		{
			dataIter = end-smoothingRadius-1;
			dataEnd = end;
			while ( dataIter != dataEnd ) copy << *dataIter++;
			dataIter = begin;
			while ( dataIter != dataEnd ) copy << *dataIter++;
			dataIter = begin;
			dataEnd = begin+smoothingRadius+1;
			while ( dataIter != dataEnd ) copy << *dataIter++;
		}
		else
		{
			dataIter = begin;
			dataEnd = end;
			for ( i=0 ; i<smoothingRadius ; ++i ) copy << *dataIter++;
			dataIter = begin;
			while ( dataIter != dataEnd ) copy << *dataIter++;
			dataIter-=smoothingRadius;
			for ( i=0 ; i<smoothingRadius ; ++i ) copy << *dataIter++;
		}

		typename QVector<T>::ConstIterator copyIterBegin = copy.constBegin();
		typename QVector<T>::ConstIterator copyIterEnd = copy.constBegin() + static_cast<int>(smoothingDiameter);

		dataIter = begin;
		dataEnd = end;

		T currentValue = std::accumulate( copyIterBegin, copyIterEnd, T() );
		*dataIter++ = currentValue/smoothingDiameter;
		while ( dataIter != dataEnd )
		{
			currentValue += (*copyIterEnd++ - *copyIterBegin++);
			*dataIter++ = currentValue/smoothingDiameter;
		}
	}

	template<typename T >
	void meanSmoothingLoop( const typename QVector<T>::iterator &begin, const typename QVector<T>::iterator &end, const uint &smoothingRadius )
	{
		const qreal smoothingDiameter = 2*smoothingRadius+1;

		if ( (end-begin) <= smoothingDiameter )
			return;

		QVector<T> copy;
		copy.reserve(end-begin+2*smoothingRadius);
		typename QVector<T>::iterator dataIter, dataEnd;

		dataIter = end-smoothingRadius-1;
		dataEnd = end;
		while ( dataIter != dataEnd ) copy << *dataIter++;
		dataIter = begin;
		while ( dataIter != dataEnd ) copy << *dataIter++;
		dataIter = begin;
		dataEnd = begin+smoothingRadius+1;
		while ( dataIter != dataEnd ) copy << *dataIter++;

		typename QVector<T>::ConstIterator copyIterBegin = copy.constBegin();
		typename QVector<T>::ConstIterator copyIterEnd = copy.constBegin() + static_cast<int>(smoothingDiameter);

		dataIter = begin;
		dataEnd = end;

		T currentValue = std::accumulate( copyIterBegin, copyIterEnd, T() );
		*dataIter++ = currentValue/smoothingDiameter;
		while ( dataIter != dataEnd )
		{
			currentValue += (*copyIterEnd++ - *copyIterBegin++);
			*dataIter++ = currentValue/smoothingDiameter;
		}
	}

	template<typename T >
	void meanSmoothingNoLoop( const typename QVector<T>::iterator &begin, const typename QVector<T>::iterator &end, const uint &smoothingRadius )
	{
		const qreal smoothingDiameter = 2*smoothingRadius+1;

		if ( (end-begin) <= smoothingDiameter )
			return;

		QVector<T> copy;
		copy.reserve(end-begin);

		typename QVector<T>::iterator dataIter, dataEnd;
		uint i;
		T currentValue;

		dataIter = begin;
		dataEnd = end;
		while ( dataIter != dataEnd ) copy << *dataIter++;

		typename QVector<T>::ConstIterator copyIterBegin = copy.constBegin();
		typename QVector<T>::ConstIterator copyIterEnd = copyIterBegin + static_cast<int>(smoothingRadius);

		dataIter = begin;
		currentValue = std::accumulate( copyIterBegin, copyIterEnd, T() );
		for ( i=smoothingRadius+1 ; i<=smoothingDiameter ; ++i )
		{
			currentValue += *copyIterEnd++;
			*dataIter++ = currentValue/static_cast<qreal>(i);
		}

		dataEnd = end-smoothingRadius;
		while ( dataIter != dataEnd )
		{
			currentValue += (*copyIterEnd++ - *copyIterBegin++);
			*dataIter++ = currentValue/smoothingDiameter;
		}

		for ( i=smoothingDiameter-1 ; i>smoothingRadius ; --i )
		{
			currentValue -= *copyIterBegin++;
			*dataIter++ = currentValue/static_cast<qreal>(i);
		}
	}

	template<typename T >
	void meanSmoothing( const typename QVector<T>::iterator &begin, const typename QVector<T>::iterator &end, const uint &smoothingRadius, const bool &loop = false )
	{
		if (loop) meanSmoothingLoop<T>( begin, end, smoothingRadius );
		else meanSmoothingNoLoop<T>( begin, end, smoothingRadius );
	}
}

#endif // GLOBALFUNCTIONS_H
