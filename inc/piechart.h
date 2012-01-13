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
	QList<PiePart> sectors() const;
	int partOfAngle( double angle ) const;

	void setOrientation( double orientation );
	void setSectorsNumber( int nbSectors );

	void draw( QPainter &painter, int sectorIdx, Coord2D center ) const;

private:
	void updateSectors();

private:
	double _orientation;
	double _angle;

	QList<PiePart> _sectors;
};

#endif // PIECHART_H
