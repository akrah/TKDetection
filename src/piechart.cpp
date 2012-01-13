#include "inc/piechart.h"

#include "inc/pie_def.h"
#include "inc/piepart.h"

PieChart::PieChart( double orientation, int nbSectors ) : _orientation(orientation), _angle(TWO_PI/static_cast<double>(nbSectors)) {
	updateSectors();
}

/*******************************
 * Public getters
 *******************************/

double PieChart::orientation() const {
	return _orientation;
}

double PieChart::angle() const {
	return _angle;
}

QList<PiePart> PieChart::sectors() const {
	return _sectors;
}

int PieChart::partOfAngle( double angle ) const {
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

void PieChart::setOrientation( double orientation ) {
	_orientation = orientation;
	updateSectors();
}

void PieChart::setSectorsNumber( int nbSectors ) {
	_angle = TWO_PI/static_cast<double>(nbSectors);
	updateSectors();
}

/*******************************
 * Private setters
 *******************************/

void PieChart::updateSectors() {
	_sectors.clear();
	const int nbSectors = TWO_PI/_angle;
	double currentOrientation = _orientation;
	for ( int i=0 ; i<nbSectors ; ++i ) {
		_sectors.append(PiePart( fmod( currentOrientation, TWO_PI ), _angle ));
		currentOrientation += _angle;
	}
}
