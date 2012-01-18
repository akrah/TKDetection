#ifndef PIECHART_H
#define PIECHART_H

#include <QList>
#include "inc/marrow_def.h"

class PiePart;
class QPainter;

class PieChart
{
public:
	PieChart( double orientation, int nbSectors );

	double orientation() const;
	double angle() const;
	int nbSectors() const;
	int partOfAngle( const double &angle ) const;

	void setOrientation( const double &orientation );
	void setSectorsNumber( const int &nbSectors );

	void draw( QPainter &painter, const int &sectorIdx, const Coord2D &center ) const;

private:
	void updateSectors();

private:
	double _orientation;
	double _angle;

	QList<PiePart> _sectors;
};

#endif // PIECHART_H
