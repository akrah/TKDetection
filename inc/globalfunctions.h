#ifndef GLOBALFUNCTIONS_H
#define GLOBALFUNCTIONS_H

#include "def/def_billon.h"
template <typename T> class Interval;

namespace TKD
{
	__billon_type__ restrictedValue( __billon_type__ value , const Interval<int> &intensityInterval );
}

#endif // GLOBALFUNCTIONS_H
