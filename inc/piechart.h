#ifndef PIECHART_H
#define PIECHART_H

#include <QList>
#include "inc/marrow_def.h"

class PiePart;
class QPainter;

class PieChart
{
public:
	PieChart( const qreal &orientation, const int &nbSectors );

	qreal orientation() const;
	qreal angle() const;
	int nbSectors() const;
	int partOfAngle( const qreal &angle ) const;

	void setOrientation( const qreal &orientation );
	void setSectorsNumber( const int &nbSectors );

	void draw( QPainter &painter, const int &sectorIdx, const iCoord2D &center ) const;

private:
	void updateSectors();

private:
	qreal _orientation;
	qreal _angle;

	QList<PiePart> _sectors;
};

#endif // PIECHART_H
