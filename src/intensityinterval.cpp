#include "inc/intensityinterval.h"

IntensityInterval::IntensityInterval() : Interval() {
}

IntensityInterval::IntensityInterval( const int &min, const int &max ) : Interval(min,max) {
}

bool IntensityInterval::isValid() const {
	const bool ok = (_min <= _max);
	return ok;
}
