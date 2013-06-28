#include "inc/piechart.h"

#include "inc/coordinate.h"
#include "inc/define.h"
#include "inc/interval.h"
#include "inc/piepart.h"

#include <QPainter>

#include <cmath>

PieChart::PieChart( const int &nbSectors )
{
	setSectorsNumber(nbSectors);
}


PieChart::PieChart( const PieChart &pieChart ) : _sectors(pieChart._sectors)
{
}

/*******************************
 * Public getters
 *******************************/

qreal PieChart::sectorAngle() const
{
	return TWO_PI/static_cast<qreal>(_sectors.size());
}

uint PieChart::nbSectors() const
{
	return _sectors.size();
}

const PiePart &PieChart::sector( const uint &index ) const
{
	return _sectors[index];
}

uint PieChart::sectorIndexOfAngle( qreal angle ) const
{
	while ( angle<0. ) angle += TWO_PI;
	return fmod(angle,TWO_PI)*nbSectors()/TWO_PI;
}

/*******************************
 * Public setters
 *******************************/

void PieChart::setSectorsNumber( const uint &nbSectors )
{
	const qreal sectorAngle = TWO_PI/static_cast<qreal>(nbSectors);
	_sectors.clear();
	for ( uint i=0 ; i<nbSectors ; ++i )
	{
		_sectors.append(PiePart( i*sectorAngle, sectorAngle ));
	}
}

void PieChart::draw( QImage &image, const uiCoord2D &center, const uint &sectorIdx, const TKD::ViewType &viewType ) const
{
	Q_ASSERT_X( sectorIdx<nbSectors() , "draw", "sector index greater than sector number" );

	// Liste qui va contenir les angles des deux côté du secteur à dessiner
	// Permet de factoriser le code de calcul des coordonnées juste en dessous
	QList<qreal> twoSides;
	twoSides.append( _sectors.at(sectorIdx).rightAngle() );
	twoSides.append( _sectors.at(sectorIdx).leftAngle() );

	// Dessin des deux côtés du secteur
	const int width = image.width();
	const int height = image.height();
	iCoord2D end;
	qreal angle, a, b;

	QPainter painter(&image);
	painter.setPen(Qt::red);
	QList<qreal>::const_iterator side;
	if ( viewType == TKD::Z_VIEW )
	{
		for ( side = twoSides.constBegin() ; side < twoSides.constEnd() ; ++side )
		{
			// Calcul des coordonnées du segment à tracer
			angle = *side;
			end = center;
			if ( qFuzzyCompare(angle,PI_ON_TWO) ) end.y = height;
			else if ( qFuzzyCompare(angle,THREE_PI_ON_TWO) ) end.y = 0;
			else
			{
				a = qTan(angle);
				b = center.y - (a*center.x);
				if ( (angle < PI_ON_TWO) || (angle > THREE_PI_ON_TWO) ) end = iCoord2D(width,a*width+b);
				else end = iCoord2D(0,b);
			}
			// Tracé du segment
			painter.drawLine(center.x,center.y,end.x,end.y);
		}
	}
	else if ( viewType == TKD::CARTESIAN_VIEW )
	{
		for ( side = twoSides.constBegin() ; side < twoSides.constEnd() ; ++side )
		{
			angle = (*side)*width/TWO_PI;
			// Tracé du segment
			painter.drawLine(angle,0,angle,height);
		}
	}
}

void PieChart::draw( QImage &image, const uiCoord2D &center, const QVector< Interval<uint> > &intervals, const TKD::ViewType &viewType ) const
{
	if ( !intervals.isEmpty() )
	{
		// Liste qui va contenir les angles des deux côté du secteur à dessiner
		// Permet de factoriser le code de calcul des coordonnées juste en dessous
		QVector<qreal> twoSides;
		QVector< Interval<uint> >::ConstIterator interval;
		for ( interval = intervals.constBegin() ; interval < intervals.constEnd() ; ++interval )
		{
			twoSides.append( sector((*interval).min()).leftAngle() );
			twoSides.append( sector((*interval).max()).rightAngle() );
		}

		// Dessin des deux côtés du secteur
		const int &width = image.width();
		const int &height = image.height();

		QVector<QColor> colors(6);
		colors[0] = Qt::blue;
		colors[1] = Qt::yellow;
		colors[2] = Qt::green;
		colors[3] = Qt::magenta;
		colors[4] = Qt::cyan;
		colors[5] = Qt::white;

		const int nbColorsToUse = qMax( intervals.size()>colors.size() ? ((intervals.size()+1)/2)%colors.size() : colors.size() , 1 );

		QColor currentColor;
		iCoord2D end;
		qreal angle;
		int colorIndex = 0;

		QPainter painter(&image);
		QVector<qreal>::ConstIterator side;
		if ( viewType == TKD::Z_VIEW )
		{
			for ( side = twoSides.constBegin() ; side != twoSides.constEnd() ; ++side )
			{
				// Calcul des coordonnées du segment à tracer
				currentColor = colors[(colorIndex++/2)%nbColorsToUse];
				painter.setPen(currentColor);
				painter.setBrush(currentColor);
				angle = *side;
				end = center;
				if ( qFuzzyCompare(angle,PI_ON_TWO) ) end.y = height;
				else if ( qFuzzyCompare(angle,THREE_PI_ON_TWO) ) end.y = 0;
				else
				{
					const qreal a = qTan(angle);
					const qreal b = center.y - (a*center.x);
					if ( (angle < PI_ON_TWO) || (angle > THREE_PI_ON_TWO) ) end = iCoord2D(width,a*width+b);
					else end = iCoord2D(0,b);
				}
				// Tracé du segment
				painter.drawLine(center.x,center.y,end.x,end.y);
			}
		}
		else if ( viewType == TKD::CARTESIAN_VIEW )
		{
			for ( side = twoSides.constBegin() ; side != twoSides.constEnd() ; ++side )
			{
				currentColor = colors[(colorIndex++/2)%nbColorsToUse];
				painter.setPen(currentColor);
				painter.setBrush(currentColor);
				angle = (*side)*width/TWO_PI;
				// Tracé du segment
				painter.drawLine(angle,0,angle,height);
			}
		}
	}
}
