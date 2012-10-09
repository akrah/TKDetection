#ifndef SLICEHISTOGRAM_H
#define SLICEHISTOGRAM_H

#include "inc/histogram.h"
#include "def/def_billon.h"

class Pith;

/*######################################################
  # DECLARATION
  ######################################################*/

class SliceHistogram : public Histogram<qreal>
{
public:
	SliceHistogram();
	virtual ~SliceHistogram();

	void construct( const Billon &billon, const Interval<int> &intensity, const Interval<int> &motionInterval, const int &borderPercentageToCut, const int &radiusAroundPith );
};

#endif // SLICEHISTOGRAM_H
