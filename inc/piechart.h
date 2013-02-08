#ifndef PIECHART_H
#define PIECHART_H

#include "def/def_coordinate.h"
#include "define.h"

#include <QVector>

class PiePart;
class QImage;
template <typename T> class Interval;

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

	void draw( QImage &image, const uiCoord2D &center, const uint &sectorIdx, const TKD::ViewType &viewType ) const;
	void draw( QImage &image, const uiCoord2D &center, const QVector<Interval<uint> > &intervals, const TKD::ViewType &viewType ) const;

private:
	void updateSectors( const int &nbSectors );

private:
	qreal _sectorAngle; // En radians
	QVector<PiePart> _sectors;
};

#endif // PIECHART_H
