#ifndef CONCAVITYPOINTSERIEHISTOGRAM_H
#define CONCAVITYPOINTSERIEHISTOGRAM_H

#include "def/def_coordinate.h"

#include <QVector>

class ContourBillon;

class ConcavityPointSerieCurve
{
public:
	ConcavityPointSerieCurve();
	~ConcavityPointSerieCurve();

	const QVector<rCoord2D> &maxConcavityPointsSerie() const;
	const QVector<rCoord2D> &minConcavityPointsSerie() const;

	int nbMaxConcavityPoints() const;
	int nbMinConcavityPoints() const;

	void clear();

	void construct( const ContourBillon &contourBillon );

private:
	QVector<rCoord2D> _maxConcavityPointsSerie;
	QVector<rCoord2D> _minConcavityPointsSerie;
};

#endif // CONCAVITYPOINTSERIEHISTOGRAM_H
