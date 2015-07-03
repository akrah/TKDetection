#ifndef PIECHART_H
#define PIECHART_H

#include "def/def_coordinate.h"
#include "define.h"
#include "inc/piepart.h"

#include <QVector>

class PiePart;
class QImage;
class QColor;
template <typename T> class Interval;

class PieChart
{
public:
	PieChart( const int &nbSectors );
	PieChart( const PieChart &pieChart );

	qreal angleStep() const;
	uint nbSectors() const;
	const PiePart &sector( const uint &index ) const;
	uint sectorIndexOfAngle( qreal angle ) const;

	void setNumberOfAngularSectors( const uint &nbSectors );

	void draw( QImage &image, const uiCoord2D &center, const uint &sectorIdx, const TKD::ProjectionType &viewType ) const;
	void draw( QImage &image, const uiCoord2D &center, const QVector<Interval<uint> > &angleIntervals, const TKD::ProjectionType &viewType, const QVector<QColor> &colors = TKD::KnotAreaColors ) const;

private:
	QVector<PiePart> _sectors;
};

#endif // PIECHART_H
