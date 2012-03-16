#include "inc/interval.h"

Interval::Interval() : QObject(), _min(0), _max(0) {
}

Interval::Interval( const int &min, const int &max ) : QObject(), _min(min), _max(max) {
}

int Interval::min() const {
	return _min;
}

int Interval::max() const {
	return _max;
}

int Interval::size() const {
	return _max - _min;
}

int Interval::count() const {
	return size() + 1;
}

void Interval::setMin( const int &min ) {
	_min = min;
}

void Interval::setMax( const int &max ) {
	_max = max;
}

void Interval::setBounds( const int &min, const int &max ) {
	_min = min;
	_max = max;
}
