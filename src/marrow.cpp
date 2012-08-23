#include "inc/marrow.h"

#include <QPainter>

Marrow::Marrow() : QList<iCoord2D>(), _interval(0,0) {
}

Marrow::Marrow( const int &begin, const int &end ) : QList<iCoord2D>(), _interval(begin,end) {
}

/*******************************
 * Public getters
 *******************************/
const Interval &Marrow::interval() const {
	return _interval;
}


/*******************************
 * Public setters
 *******************************/
void Marrow::draw( QImage &image, const int &sliceIdx ) const {
	if ( _interval.containsClosed(sliceIdx) ) {
		const iCoord2D &coordToDraw = at(sliceIdx-_interval.minValue());

		QPainter painter(&image);
		QPainterPath ellipsePath;
		ellipsePath.addEllipse(coordToDraw.x-5,coordToDraw.y-5,10,10);
		QColor color(Qt::red);

		painter.setBrush(color);
		painter.setPen(color);
		painter.drawPath(ellipsePath);
	}
}
