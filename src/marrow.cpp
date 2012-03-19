#include "inc/marrow.h"

#include <QPainter>
#include "inc/slicesinterval.h"

Marrow::Marrow() : QList<iCoord2D>(), _interval(0,0) {
}

Marrow::Marrow( const int &begin, const int &end ) : QList<iCoord2D>(), _interval(begin,end) {
}

/*******************************
 * Public getters
 *******************************/
const SlicesInterval &Marrow::interval() const {
	return _interval;
}


/*******************************
 * Public setters
 *******************************/
void Marrow::draw( QPainter &painter, const int &sliceIdx ) const {
	if ( _interval.containsClosed(sliceIdx) ) {
		const iCoord2D &coordToDraw = at(sliceIdx-_interval.min());

		QPainterPath ellipsePath;
		ellipsePath.addEllipse(coordToDraw.x-5,coordToDraw.y-5,10,10);
		QColor color(100,200,100);

		painter.setBrush(color);
		painter.setPen(color);
		painter.drawPath(ellipsePath);
	}
}
