#ifndef KNOTAREAHISTOGRAM_H
#define KNOTAREAHISTOGRAM_H

#include "def/def_billon.h"
#include "histogram.h"

class Pith;

class KnotAreaHistogram : public Histogram<qreal>
{
public:
	KnotAreaHistogram();
	virtual ~KnotAreaHistogram();

	void construct(const Billon &billon);
	void computeMaximumsAndIntervals( const uint &comparisonShift, const qreal &comparisonValue );
};

#endif // KNOTAREAHISTOGRAM_H
