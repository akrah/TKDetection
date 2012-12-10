#ifndef NEARESTPOINTSHISTOGRAM_H
#define NEARESTPOINTSHISTOGRAM_H

#include "def/def_billon.h"
#include "histogram.h"

class Pith;

class NearestPointsHistogram : public Histogram<qreal>
{
public:
	NearestPointsHistogram();
	virtual ~NearestPointsHistogram();

	void construct(const Billon &billon);
	void computeMaximumsAndIntervals( const uint &comparisonShift, const qreal &comparisonValue );
};

#endif // NEARESTPOINTSHISTOGRAM_H
