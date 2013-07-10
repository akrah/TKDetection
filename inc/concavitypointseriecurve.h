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

	const QVector<rCoord2D> &leftConcavityPointsSerie() const;
	const QVector<rCoord2D> &rightConcavityPointsSerie() const;

	int nbLeftConcavityPoints() const;
	int nbRightConcavityPoints() const;

	void clear();

	void construct( const ContourBillon &contourBillon );

private:
	QVector<rCoord2D> _leftConcavityPointsSerie;
	QVector<rCoord2D> _rightConcavityPointsSerie;
};

#endif // CONCAVITYPOINTSERIEHISTOGRAM_H
