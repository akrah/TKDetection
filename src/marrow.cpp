#include "inc/marrow.h"

#include <QPainter>
#include <iostream>

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

void Marrow::draw( QPainter &painter, int sliceIdx ) const {
	if ( (sliceIdx>=_begin) && (sliceIdx<=_end) ) {
		const Coord2D &coordToDraw = at(sliceIdx-_begin);
		std::cout << "Affichage de la moelle en " << coordToDraw << std::endl;

		QPainterPath ellipsePath;
		ellipsePath.addEllipse(coordToDraw.x-5,coordToDraw.y-5,10,10);
		QColor color(100,200,100);

		painter.setBrush(color);
		painter.setPen(color);
		painter.drawPath(ellipsePath);
	}
}
