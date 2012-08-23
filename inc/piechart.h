#ifndef PIECHART_H
#define PIECHART_H

#include <QVector>
#include "inc/global.h"

class PiePart;
class QImage;

class PieChart
{
public:
	PieChart( const qreal &orientation, const int &nbSectors );
	PieChart( const PieChart &pieChart );

	qreal orientation() const;
	qreal angle() const;
	int nbSectors() const;
	const PiePart &sector( const int &index ) const;
	int partOfAngle( const qreal &angle ) const;

	void setOrientation( const qreal &orientation );
	void setSectorsNumber( const int &nbSectors );

	void draw( QImage &painter, const int &sectorIdx, const iCoord2D &center ) const;

private:
	void updateSectors( const int &nbSectors );

private:
	qreal _orientation;
	qreal _angle; // En radians

	QVector<PiePart> _sectors;
};

#endif // PIECHART_H
