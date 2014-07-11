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
		//const int diameter = 2*radius;
		const int xPos = qRound((*this)[sliceIdx].x);
		const int yPos = qRound((*this)[sliceIdx].y);
		QPainter painter(&image);
		painter.setBrush(Qt::red);
		painter.setPen(Qt::red);
		painter.drawLine(xPos-radius,yPos,xPos+radius,yPos);
		painter.drawLine(xPos,yPos-radius,xPos,yPos+radius);
		//painter.drawEllipse(xPos-radius,yPos-radius,diameter,diameter);
	}
}
