#include "inc/piechart.h"

#include "inc/global.h"
#include "inc/piepart.h"

#include <QPainter>

PieChart::PieChart( const qreal &orientation, const int &nbSectors ) : _orientation(orientation), _angle(TWO_PI/static_cast<qreal>(nbSectors)) {
	updateSectors();
}

/*******************************
 * Public getters
 *******************************/

qreal PieChart::orientation() const {
	return _orientation;
}

qreal PieChart::angle() const {
	return _angle;
}

int PieChart::nbSectors() const {
	return (TWO_PI/_angle);
}

int PieChart::partOfAngle( const qreal &angle ) const {
	int sectorId;
	bool ok = false;
	const int nbSectors = _sectors.size();
	for ( sectorId = 0 ; !ok && (sectorId < nbSectors) ; sectorId += !ok ) {
		ok = _sectors.at(sectorId).contains(angle);
	}
	return (sectorId<nbSectors)?sectorId:0;
}

/*******************************
 * Public setters
 *******************************/

void PieChart::setOrientation( const qreal &orientation ) {
	_orientation = orientation;
	updateSectors();
}

void PieChart::setSectorsNumber( const int &nbSectors ) {
	_angle = TWO_PI/static_cast<qreal>(nbSectors);
	updateSectors();
}

void PieChart::draw( QImage &image, const int &sectorIdx, const iCoord2D &center ) const {
	const int width = image.width();
	const int height = image.height();
	const int centerX = center.x;
	const int centerY = center.y;

	// Liste qui va contenir les angles des deux côté du secteur à dessiner
	// Permet de factoriser le code de calcul des coordonnées juste en dessous
	QList<qreal> twoSides;
	twoSides.append( TWO_PI-_sectors.at(sectorIdx).rightAngle() );
	twoSides.append( TWO_PI-_sectors.at(sectorIdx).leftAngle() );

	QPainter painter(&image);
	painter.setPen(QColor(0,255,0));

	// Dessin des deux côtés du secteur
	qreal angle, x1,y1,x2,y2;
	while ( !twoSides.isEmpty() ) {
		// Calcul des coordonnées du segment à tracer
		angle = twoSides.takeLast();
		x1 = x2 = centerX;
		y1 = y2 = centerY;
		if ( IS_EQUAL(angle,PI_ON_TWO) ) y2 = height;
		else if ( IS_EQUAL(angle,THREE_PI_ON_TWO) ) y1 = 0;
		else {
			const qreal a = tan(angle);
			const qreal b = centerY - (a*centerX);
			if ( angle < PI_ON_TWO || angle > THREE_PI_ON_TWO ) {
				x2 = width;
				y2 = a*width+b;
			}
			else {
				x1 = 0;
				y1 = b;
			}
		}

		// Tracé du segment droit
		painter.drawLine(x1,y1,x2,y2);
	}
}

/*******************************
 * Private setters
 *******************************/

void PieChart::updateSectors() {
	_sectors.clear();
	const int nbSectors = TWO_PI/_angle;
	qreal currentOrientation = _orientation;
	for ( int i=0 ; i<nbSectors ; ++i ) {
		_sectors.append(PiePart( fmod( currentOrientation, TWO_PI ), _angle ));
		currentOrientation += _angle;
	}
}
