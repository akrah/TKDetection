#ifndef PIECHART_H
#define PIECHART_H

#include <QList>
class PiePart;

class PieChart
{
public:
	PieChart( double orientation, int nbSectors );

	double orientation() const;
	double angle() const;
	QList<PiePart> sectors() const;

	int sectorOfAngle( double angle ) const;

	void setOrientation( double orientation );
	void setSectorsNumber( int nbSectors );


private:
	void updateSectors();

private:
	double _orientation;
	double _angle;

	QList<PiePart> _sectors;
};

#endif // PIECHART_H
