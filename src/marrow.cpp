#include "inc/marrow.h"

#include <QPainter>

Marrow::Marrow() : QList<Coord2D>(), _begin(0), _end(0), _isCenteredMarrow(false) {
}

Marrow::Marrow( const int &begin, const int &end ) : QList<Coord2D>(), _begin(begin), _end(end), _isCenteredMarrow(false) {
}

/*******************************
 * Public getters
 *******************************/

int Marrow::beginSlice() const {
	return _begin;
}

int Marrow::endSlice() const {
	return _end;
}


/*******************************
 * Public setters
 *******************************/
void Marrow::centerMarrow( const bool &enable ) {
	_isCenteredMarrow = enable;
}

void Marrow::draw( QPainter &painter, const int &sliceIdx ) const {
	if ( (sliceIdx>=_begin) && (sliceIdx<=_end) ) {
		const Coord2D &coordToDraw = _isCenteredMarrow?Coord2D(painter.window().width()/2,painter.window().height()/2):at(sliceIdx-_begin);

		QPainterPath ellipsePath;
		ellipsePath.addEllipse(coordToDraw.x-5,coordToDraw.y-5,10,10);
		QColor color(100,200,100);

		painter.setBrush(color);
		painter.setPen(color);
		painter.drawPath(ellipsePath);
	}
}
