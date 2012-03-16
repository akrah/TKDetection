#include "inc/slicesinterval.h"

SlicesInterval::SlicesInterval() : Interval() {
}

SlicesInterval::SlicesInterval( const int &min, const int &max ) : Interval(min,max) {
}

bool SlicesInterval::isValid() const {
	const bool ok = (_min > -1) && (_min <= _max);
	return ok;
}
