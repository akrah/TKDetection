#include "inc/contour.h"

#include "inc/billon.h"
#include "inc/slicealgorithm.h"
#include "inc/define.h"

#include <QPainter>
#include <QLinkedList>

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
			for ( int i=0 ; i<nbPoints-1 ; ++i )
			{
				smoothingValueX = smoothingValueX - initialContour[i].x + initialContour[i+smoothingDiameter].x;
				smoothingValueY = smoothingValueY - initialContour[i].y + initialContour[i+smoothingDiameter].y;
				currentCoord.x = smoothingValueX / qSmoothingDiameter;
				currentCoord.y = smoothingValueY / qSmoothingDiameter;
				if ( this->last() != currentCoord ) this->append(currentCoord);
			}
		}
	}
}

Contour Contour::convexHull()
{
	Contour convexHull;
	QVector<iCoord2D>::ConstIterator iterPoints = this->constBegin();
	convexHull.append(*iterPoints++);
	convexHull.append(*iterPoints++);
	while ( iterPoints != this->end() )
	{
		const iCoord2D &point = *iterPoints++;
		while ( convexHull.size() > 1 &&  convexHull.at(convexHull.size()-2).vectorProduct(convexHull.last(),point) <= 0 )
		{
			convexHull.pop_back();
		}
		convexHull.push_back(point);
	}
	return convexHull;
}

void Contour::draw( QPainter &painter, const int &cursorPosition, const iCoord2D &sliceCenter, const TKD::ViewType &viewType ) const
{
	const int nbPoints = this->size();
	if ( nbPoints > 0 )
	{
		const qreal angularFactor = painter.window().width()/TWO_PI;
		int i, j, k, x, y;

		painter.save();
		painter.setPen(Qt::blue);
		if ( viewType == TKD::Z_VIEW )
		{
			for ( int i=0 ; i<nbPoints ; ++i )
			{
				painter.drawPoint((*this)[i].x,(*this)[i].y);
			}

		}
		else if ( viewType == TKD::CARTESIAN_VIEW )
		{
			for ( k=0 ; k<nbPoints ; ++k )
			{
				i = (*this)[k].x - sliceCenter.x;
				j = (*this)[k].y - sliceCenter.y;
				y = qSqrt(qPow(i,2) + qPow(j,2));
				x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
				painter.drawPoint(x,y);
			}
		}

		// Dessin du curseur
		if ( cursorPosition >= 0 )
		{
			painter.setPen(Qt::cyan);
			if ( viewType == TKD::Z_VIEW )
			{
				painter.drawEllipse((*this)[cursorPosition].x-1,(*this)[cursorPosition].y-1,2,2);
			}
			else if ( viewType == TKD::CARTESIAN_VIEW )
			{
				i = sliceCenter.x - (*this)[cursorPosition].x;
				j = sliceCenter.y - (*this)[cursorPosition].y;
				y = qSqrt(qPow(i,2) + qPow(j,2));
				x = 2. * qAtan( j / (i + y ) );
				painter.drawEllipse(x-1,y-1,2,2);
			}
		}
		painter.restore();
	}

}
