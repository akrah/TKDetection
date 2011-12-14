#include "billon.h"

Billon::Billon() : icube(0,0,0), _minValue(0), _maxValue(0) {
}

Billon::Billon(int width, int height, int depth) : icube(height,width,depth), _minValue(0), _maxValue(0) {
}

int Billon::minValue() const {
	return _minValue;
}

int Billon::maxValue() const {
	return _maxValue;
}

void Billon::setMinValue(const int max) {
	_minValue = max;
}

void Billon::setMaxValue(const int min) {
	_maxValue = min;
}
