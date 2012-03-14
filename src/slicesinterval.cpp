#include "inc/slicesinterval.h"

SlicesInterval::SlicesInterval() : QObject(), _min(0), _max(0) {
}

SlicesInterval::SlicesInterval( int min, int max ) : QObject(), _min(min), _max(max) {
}

int SlicesInterval::min() const {
	return _min;
}

int SlicesInterval::max() const {
	return _max;
}

void SlicesInterval::setMin( const int &min ) {
	_min = min;
}

void SlicesInterval::setMax( const int &max ) {
	_max = max;
}

bool SlicesInterval::isValid() const {
	bool ok = (_min > -1) && (_min <= _max);
	return ok;
}
