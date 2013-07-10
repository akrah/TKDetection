#ifndef PIECHART_H
#define PIECHART_H

#include "def/def_coordinate.h"
#include "define.h"
#include "inc/piepart.h"

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
	QVector<PiePart> _sectors;
};

class PieChartSingleton : public PieChart
{
private:
	PieChartSingleton() : PieChart(360) {}
	~PieChartSingleton () { kill(); }

public:

	// Fonctions de création et destruction du singleton
	static PieChartSingleton *getInstance()
	{
		if ( !_singleton ) _singleton =  new PieChartSingleton();
		return _singleton;
	}

	static void kill()
	{
		if (_singleton)
		{
			delete _singleton;
			_singleton = 0;
		}
	}

private:
	static PieChartSingleton *_singleton;
};

#endif // PIECHART_H
