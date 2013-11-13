#include "inc/pith.h"

#include "inc/coordinate.h"

#include <QPainter>

Pith::Pith(const int size) : QVector<rCoord2D>(size)
{
}

Pith::Pith( const Pith &pith ) : QVector<rCoord2D>(pith)
{
}

Pith::Pith( const QVector<rCoord2D> &coordinates ) : QVector<rCoord2D>(coordinates)
{
}

/*******************************
 * Public getters
 *******************************/


/*******************************
 * Public setters
 *******************************/
void Pith::draw( QImage &image, const int &sliceIdx, const int &radius ) const
{
	if ( sliceIdx < size() )
	{
		const int diameter = 2*radius;
		QPainter painter(&image);
		painter.setBrush(Qt::red);
		painter.setPen(Qt::red);
		painter.drawEllipse((*this)[sliceIdx].x-radius,(*this)[sliceIdx].y-radius,diameter,diameter);
	}
}
