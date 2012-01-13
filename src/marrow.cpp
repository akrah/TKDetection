#include "inc/marrow.h"

Marrow::Marrow() : QList<Coord2D>(), _begin(0), _end(0) {
}

Marrow::Marrow( int begin, int end ) : QList<Coord2D>(), _begin(begin), _end(end) {
}

int Marrow::beginSlice() const {
	return _begin;
}

int Marrow::endSlice() const {
	return _end;
}
