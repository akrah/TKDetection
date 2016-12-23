#include "inc/globalfunctions.h"

#include "inc/interval.h"

#include <QVector>
#include <qmath.h>

namespace TKD
{
	__billon_type__ restrictedValue( __billon_type__ value , const Interval<int> &intensityInterval )
	{
		return intensityInterval.containsOpen(value) ? value : value > intensityInterval.max() ? intensityInterval.max() : intensityInterval.min();
	}

	QVector<int> circleLines( const int &radius )
	{
		QVector<int> linesLength(2*radius+1);

		const qreal squareRadius = qPow(radius,2);
		const int midIndex = radius;

		linesLength[midIndex] = radius;
		for ( int lineIndex=1 ; lineIndex<=radius ; ++lineIndex )
		{
			linesLength[midIndex+lineIndex] = linesLength[midIndex-lineIndex] = qSqrt(squareRadius-qPow(lineIndex,2));
		}
		return linesLength;
	}
}
