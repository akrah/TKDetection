#ifndef ELLIPTICALACCUMULATIONHISTOGRAM_H
#define ELLIPTICALACCUMULATIONHISTOGRAM_H

#include "def/def_billon.h"
#include "def/def_coordinate.h"

#include "inc/histogram.h"

class EllipticalAccumulationHistogram : public Histogram<qreal>
{
public:
	EllipticalAccumulationHistogram();
	virtual ~EllipticalAccumulationHistogram();

	uint detectedMaximum() const;
	uint detectedRadius() const;

	void construct(const Slice &slice, const uiCoord2D &origin, const qreal &ellipticityRate,
					const uint &smoothingRadius, const qreal &widthCoeff );

private:
	void findFirstMaximumAndNextMinimum();
};

#endif // ELLIPTICALACCUMULATIONHISTOGRAM_H
