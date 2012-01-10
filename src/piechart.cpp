#include "inc/piechart.h"

#include "inc/piepart.h"

namespace {
	inline double ANGLE( double xo, double yo, double x2, double y2 ) {
		const double x_diff = x2-xo;
		double arcos = std::acos(x_diff / sqrt(pow(x_diff,2)+pow(y2-yo,2)));
		if ( yo > y2 ) arcos = -arcos+TWO_PI;
		return arcos;
	}
}

PieChart::PieChart( double orientation, int nbSectors ) : _orientation(orientation), _angle(TWO_PI/static_cast<double>(nbSectors)) {
	updateSectors();
}

double PieChart::orientation() const {
	return _orientation;
}

double PieChart::angle() const {
	return _angle;
}

QList<PiePart> PieChart::sectors() const {
	return _sectors;
}

int PieChart::sectorOfAngle( double angle ) const {
	int sectorId;
	bool ok = false;
	const int nbSectors = _sectors.size();
	for ( sectorId = 0 ; !ok && (sectorId < nbSectors) ; sectorId += !ok ) {
		ok = _sectors.at(sectorId).contains(angle);
	}
	return (sectorId<nbSectors)?sectorId:0;
}

void PieChart::setOrientation( double orientation ) {
	_orientation = orientation;
	updateSectors();
}

void PieChart::setSectorsNumber( int nbSectors ) {
	_angle = TWO_PI/static_cast<double>(nbSectors);
	updateSectors();
}

void PieChart::updateSectors() {
	_sectors.clear();
	const int nbSectors = TWO_PI/_angle;
	double currentOrientation = _orientation;
	for ( int i=0 ; i<nbSectors ; ++i ) {
		_sectors.append(PiePart( fmod( currentOrientation, TWO_PI ), _angle ));
		currentOrientation += _angle;
	}
}
