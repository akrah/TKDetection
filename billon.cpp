#include "billon.h"

Billon::Billon() : icube(0,0,0), _minValue(0), _maxValue(0) {
}

Billon::Billon(int width, int height, int depth) : icube(height,width,depth), _minValue(0), _maxValue(0) {
}
