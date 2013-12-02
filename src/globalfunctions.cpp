#include "inc/globalfunctions.h"

#include "inc/interval.h"

#include <QVector>

namespace TKD
{
	__billon_type__ restrictedValue( __billon_type__ value , const Interval<int> &intensityInterval )
	{
		return intensityInterval.containsOpen(value) ? value : intensityInterval.min();
	}
}
