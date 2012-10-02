#ifndef PIECHART_H
#define PIECHART_H

#include <QVector>
#include "inc/global.h"

class PiePart;
class QImage;

class PieChart
{
public:
	PieChart( const int &nbSectors );
	PieChart( const PieChart &pieChart );

	qreal sectorAngle() const;
	uint nbSectors() const;
	const PiePart &sector( const uint &index ) const;
	uint sectorIndexOfAngle( qreal sectorAngle ) const;

	void setSectorsNumber( const uint &nbSectors );

	void draw( QImage &painter, const uint &sectorIdx, const iCoord2D &center ) const;

private:
	void updateSectors( const int &nbSectors );

private:
	qreal _sectorAngle; // En radians
	QVector<PiePart> _sectors;
};

#endif // PIECHART_H
