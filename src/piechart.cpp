#include "inc/piechart.h"

#include "inc/coordinate.h"
#include "inc/define.h"
#include "inc/interval.h"
#include "inc/piepart.h"

#include <QPainter>

#include <cmath>

PieChartSingleton *PieChartSingleton::_singleton = 0;

PieChart::PieChart( const int &nbSectors )
{
	setNumberOfAngularSectors(nbSectors);
}


PieChart::PieChart( const PieChart &pieChart ) : _sectors(pieChart._sectors)
{
}

/*******************************
 * Public getters
 *******************************/

qreal PieChart::angleStep() const
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

void PieChart::setNumberOfAngularSectors( const uint &nbSectors )
{
	const qreal sectorAngle = TWO_PI/static_cast<qreal>(nbSectors);
	_sectors.clear();
	for ( uint i=0 ; i<nbSectors ; ++i )
	{
		_sectors.append(PiePart( i*sectorAngle, sectorAngle ));
	}
}

void PieChart::draw( QImage &image, const uiCoord2D &center, const uint &sectorIdx, const TKD::ProjectionType &viewType ) const
{
	Q_ASSERT_X( sectorIdx<nbSectors() , "draw", "sector index greater than sector number" );

	// Liste qui va contenir les angles des deux côté du secteur à dessiner
	// Permet de factoriser le code de calcul des coordonnées juste en dessous
	QList<qreal> twoSides;
	twoSides.append( _sectors[sectorIdx].minAngle() );
	twoSides.append( _sectors[sectorIdx].maxAngle() );

	// Dessin des deux côtés du secteur
	const int width = image.width();
	const int height = image.height();
	iCoord2D end;
	qreal angle, a, b;

	QPainter painter(&image);
	painter.setPen(Qt::red);
	QList<qreal>::const_iterator side;
	if ( viewType == TKD::Z_PROJECTION )
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
	else if ( viewType == TKD::POLAR_PROJECTION )
	{
		for ( side = twoSides.constBegin() ; side < twoSides.constEnd() ; ++side )
		{
			angle = (*side)*width/TWO_PI;
			// Tracé du segment
			painter.drawLine(angle,0,angle,height);
		}
	}
}

void PieChart::draw( QImage &image, const uiCoord2D &center, const QVector< Interval<uint> > &angleIntervals, const TKD::ProjectionType &viewType ) const
{
	if ( !angleIntervals.isEmpty() )
	{
		// Liste qui va contenir les angles des deux côté du secteur à dessiner
		// Permet de factoriser le code de calcul des coordonnées juste en dessous
		QVector<qreal> twoSides;
		QVector< Interval<uint> >::ConstIterator interval;
		for ( interval = angleIntervals.constBegin() ; interval < angleIntervals.constEnd() ; ++interval )
		{
			twoSides.append( sector((*interval).min()).maxAngle() );
			twoSides.append( sector((*interval).max()).minAngle() );
		}

		// Dessin des deux côtés du secteur
		const int &width = image.width();
		const int &height = image.height();

		QVector<QColor> colors;
		colors << Qt::blue << Qt::yellow << Qt::green << Qt::magenta << Qt::cyan << Qt::white;

		const int nbColorsToUse = qMax( angleIntervals.size()>colors.size() ? ((angleIntervals.size()+1)/2)%colors.size() : colors.size() , 1 );

		QColor currentColor;
		iCoord2D end;
		qreal angle;
		int colorIndex = 0;

		QPainter painter(&image);
		QVector<qreal>::ConstIterator side;
		if ( viewType == TKD::Z_PROJECTION )
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
		else if ( viewType == TKD::POLAR_PROJECTION )
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
