#ifndef GLOBALFUNCTIONS_H
#define GLOBALFUNCTIONS_H

#include "def/def_billon.h"

#include <QtGlobal>
#include <QVector>

template <typename T> class Interval;

namespace TKD
{
	__billon_type__ restrictedValue( __billon_type__ value , const Interval<int> &intensityInterval );

	template<typename T >
	void meanSmoothing( const typename QVector<T>::iterator &begin, const typename QVector<T>::iterator &end, const uint &smoothingRadius, const bool &loop = false )
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
			for ( i=0 ; i<smoothingRadius ; ++i ) copy << *dataIter;
			while ( dataIter != dataEnd ) copy << *dataIter++;
			dataIter--;
			for ( i=0 ; i<smoothingRadius ; ++i ) copy << *dataIter;
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
}

#endif // GLOBALFUNCTIONS_H
