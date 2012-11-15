#include "inc/contour.h"

#include "inc/billon.h"
#include "inc/slicealgorithm.h"
#include "inc/define.h"

#include <QPainter>

Contour::Contour() : QVector<iCoord2D>(0)
{
}

Contour::Contour( const QVector<iCoord2D> &vector ) : QVector<iCoord2D>(vector)
{
}

Contour::Contour( const Contour &contour ) : QVector<iCoord2D>(contour)
{
}

Contour::~Contour()
{
}

void Contour::compute( const Slice &slice, const iCoord2D &sliceCenter, const int &intensityThreshold, iCoord2D startPoint )
{
	this->clear();
	if ( startPoint == iCoord2D(-1,-1) )
	{
		startPoint = SliceAlgorithm::findNearestPointOfThePith( slice, sliceCenter, intensityThreshold );
	}
	if ( startPoint != iCoord2D(-1,-1) )
	{
		iCoord2D currentPos(startPoint);
		QVector<iCoord2D> mask(8);
		qreal startAngle;
		int interdit, j;

		// Using Moore-Neighbor Tracing
		startAngle = startPoint.angle(sliceCenter);
		interdit = qRound(startAngle>=0 ? startAngle*4./PI : (startAngle+TWO_PI)*8./TWO_PI);
		do
		{
			this->append(currentPos);
			mask[0].x = mask[1].x = mask[7].x = currentPos.x+1;
			mask[2].x = mask[6].x = currentPos.x;
			mask[3].x = mask[4].x = mask[5].x = currentPos.x-1;
			mask[1].y = mask[2].y = mask[3].y = currentPos.y+1;
			mask[0].y = mask[4].y = currentPos.y;
			mask[5].y = mask[6].y = mask[7].y = currentPos.y-1;
			j = (interdit+1)%8;
			while ( j < interdit+8 && slice.at(mask[j%8].y,mask[j%8].x) <= intensityThreshold ) ++j;
			currentPos = mask[j%8];
			interdit = (j+4)%8;
		}
		while ( currentPos != startPoint || this->size() < 10 );
	}
}

void Contour::smooth( int smoothingRadius )
{
	const int nbPoints = this->size();
	if ( nbPoints > 0 && smoothingRadius > 0 )
	{
		smoothingRadius = qMin(smoothingRadius,nbPoints);
		const int smoothingDiameter = 2*smoothingRadius+1;
		const qreal qSmoothingDiameter = smoothingDiameter;
		if ( nbPoints > smoothingDiameter )
		{
			QVector<iCoord2D> initialContour;
			initialContour.reserve(nbPoints+2*smoothingRadius);
			initialContour << this->mid(nbPoints-smoothingRadius) <<  *this << this->mid(0,smoothingRadius);

			int i, smoothingValueX, smoothingValueY;
			smoothingValueX = smoothingValueY = 0;

			for ( i=0 ; i<smoothingDiameter ; ++i )
			{
				smoothingValueX += initialContour[i].x;
				smoothingValueY += initialContour[i].y;
			}

			this->clear();
			this->append(iCoord2D( smoothingValueX/qSmoothingDiameter, smoothingValueY/qSmoothingDiameter ));
			iCoord2D currentCoord;
			for ( int i=1 ; i<nbPoints ; ++i )
			{
				smoothingValueX = smoothingValueX - initialContour[i-1].x + initialContour[i+smoothingDiameter-1].x;
				smoothingValueY = smoothingValueY - initialContour[i-1].y + initialContour[i+smoothingDiameter-1].y;
				currentCoord.x = smoothingValueX / qSmoothingDiameter;
				currentCoord.y = smoothingValueY / qSmoothingDiameter;
				if ( this->last() != currentCoord ) this->append(currentCoord);
			}
		}
	}
}

void Contour::draw( QPainter &painter, const int &cursorPosition ) const
{

	const int nbPoints = this->size();
	if ( nbPoints > 0 )
	{
		painter.save();
		painter.setPen(Qt::blue);
		for ( int i=0 ; i<nbPoints ; ++i )
		{
			painter.drawPoint((*this)[i].x,(*this)[i].y);
		}

		// Dessin du curseur
		if ( cursorPosition >= 0 )
		{
			painter.setPen(Qt::cyan);
			painter.drawEllipse((*this)[cursorPosition].x-1,(*this)[cursorPosition].y-1,2,2);
		}
		painter.restore();
	}

}
