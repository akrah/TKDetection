#include "inc/piechart.h"

#include "inc/global.h"
#include "inc/piepart.h"

#include <QPainter>

PieChart::PieChart( const int &nbSectors ) : _sectorAngle(TWO_PI/static_cast<qreal>(nbSectors))
{
	updateSectors(nbSectors);
}


PieChart::PieChart( const PieChart &pieChart ) : _sectorAngle(pieChart._sectorAngle), _sectors(pieChart._sectors)
{
}

/*******************************
 * Public getters
 *******************************/

qreal PieChart::sectorAngle() const
{
	return _sectorAngle;
}

uint PieChart::nbSectors() const
{
	return qRound(TWO_PI/_sectorAngle);
}

const PiePart &PieChart::sector( const int &index ) const
{
	return _sectors[index];
}

uint PieChart::sectorIndexOfAngle( qreal angle ) const
{
	while ( angle<0 ) angle += TWO_PI;
	return fmod(angle,TWO_PI)*nbSectors()/TWO_PI;
}

/*******************************
 * Public setters
 *******************************/

void PieChart::setSectorsNumber( const int &nbSectors )
{
	_sectorAngle = TWO_PI/static_cast<qreal>(nbSectors);
	updateSectors(nbSectors);
}

void PieChart::draw( QImage &image, const int &sectorIdx, const iCoord2D &center ) const
{
	const int width = image.width();
	const int height = image.height();

	// Liste qui va contenir les angles des deux côté du secteur à dessiner
	// Permet de factoriser le code de calcul des coordonnées juste en dessous
	QList<qreal> twoSides;
	twoSides.append( _sectors.at(sectorIdx).rightAngle() );
	twoSides.append( _sectors.at(sectorIdx).leftAngle() );

	QPainter painter(&image);
	painter.setPen(Qt::red);

	// Dessin des deux côtés du secteur
	qreal angle;
	iCoord2D c1, c2;
	while ( !twoSides.isEmpty() )
	{
		// Calcul des coordonnées du segment à tracer
		angle = twoSides.takeLast();
		c1 = c2 = center;
		if ( qFuzzyCompare(angle,PI_ON_TWO) ) c2.y = height;
		else if ( qFuzzyCompare(angle,THREE_PI_ON_TWO) ) c1.y = 0;
		else {
			const qreal a = tan(angle);
			const qreal b = center.y - (a*center.x);
			if ( angle < PI_ON_TWO || angle > THREE_PI_ON_TWO ) c2 = iCoord2D(width,a*width+b);
			else c1 = iCoord2D(0,b);
		}

		// Tracé du segment droit
		painter.drawLine(c1.x,c1.y,c2.x,c2.y);
	}
}

/*******************************
 * Private setters
 *******************************/

void PieChart::updateSectors( const int &nbSectors )
{
	_sectors.clear();
	qreal currentOrientation = 0.;
	for ( int i=0 ; i<nbSectors ; ++i )
	{
		_sectors.append(PiePart( currentOrientation, _sectorAngle ));
		currentOrientation += _sectorAngle;
	}
}
