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

	void construct( const Billon &billon, const int &treeRadius );
	void computeMaximumsAndIntervals( const uint &smoothingRadius, const int & minimumHeightPercentageOfMaximum,
									  const int & derivativesPercentage, const int &minimumWidthOfIntervals, const bool & loop );
};

#endif // NEARESTPOINTSHISTOGRAM_H
