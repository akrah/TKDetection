#ifndef CONTOURDISTANCESHISTOGRAM_H
#define CONTOURDISTANCESHISTOGRAM_H

#include "def/def_coordinate.h"
#include "histogram.h"

class Contour;

class ContourDistancesHistogram : public Histogram<qreal>
{
public:
	ContourDistancesHistogram();
	~ContourDistancesHistogram();

	void construct( const Contour &contour, const iCoord2D &pithCoord );
};

#endif // CONTOURDISTANCESHISTOGRAM_H
