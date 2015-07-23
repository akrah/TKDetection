#ifndef KNOTELLIPSERADIIHISTOGRAM_H
#define KNOTELLIPSERADIIHISTOGRAM_H

#include "def/def_billon.h"
#include "def/def_coordinate.h"

#include "inc/histogram.h"
#include "inc/segmentation/ellipticalaccumulationhistogram.h"

class PithProfile;

class EllipseRadiiHistogram : public Histogram<qreal>
{
public:
	EllipseRadiiHistogram();
	virtual ~EllipseRadiiHistogram();

	const EllipticalAccumulationHistogram & ellipticalHistogram( const int & sliceIndex ) const;

	void construct(const Billon &tangentialBillon, const PithProfile &knotPithProfile,
					const qreal & lowessBandWidth, const uint &smoothingRadius, const qreal &iqrCoeff,
					const uint &percentageOfFirstValidSlicesToExtrapolate, const uint &percentageOfLastValidSlicesToExtrapolate );

private:
	void extrapolation( const Interval<uint> &validSlices , const uint & percentageOfFirstValidSlicesToExtrapolate,
						const uint & percentageOfLastValidSlicesToExtrapolate);
	void outlierInterpolation( const QVector<qreal> &residus, const qreal &iqrCoeff );

private:
	QVector<EllipticalAccumulationHistogram> _ellipticalHistograms;
};

#endif // KNOTELLIPSERADIIHISTOGRAM_H
