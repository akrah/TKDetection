#include "inc/pith.h"

#include "inc/coordinate.h"

#include <QPainter>

Pith::Pith() : QList<iCoord2D>(), _interval(0,0)
{
}

Pith::Pith( const int &begin, const int &end ) : QList<iCoord2D>(), _interval(begin,end)
{
}

Pith::Pith( const Pith &pith ) : QList<iCoord2D>(pith)
{
	_interval = pith._interval;
}

/*******************************
 * Public getters
 *******************************/
const Interval<int> &Pith::interval() const
{
	return _interval;
}


/*******************************
 * Public setters
 *******************************/
void Pith::draw( QImage &image, const int &sliceIdx ) const
{
	if ( _interval.containsClosed(sliceIdx) )
	{
		const iCoord2D &coordToDraw = at(sliceIdx-_interval.min());

		QPainter painter(&image);
		QPainterPath ellipsePath;
		ellipsePath.addEllipse(coordToDraw.x-5,coordToDraw.y-5,10,10);
		QColor color(Qt::red);

		painter.setBrush(color);
		painter.setPen(color);
		painter.drawPath(ellipsePath);
	}
}
