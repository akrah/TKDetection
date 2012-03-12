#include "inc/billon.h"

Billon::Billon() : icube(0,0,0), _minValue(0), _maxValue(0) {
}

Billon::Billon( const int &width, const int &height, const int &depth ) : icube(height,width,depth), _minValue(0), _maxValue(0) {
}
