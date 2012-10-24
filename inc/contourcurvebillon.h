#ifndef CONTOURCURVEBILLON_H
#define CONTOURCURVEBILLON_H

#include "def/def_billon.h"
#include "inc/billon.h"

template <typename T> class QList;
class ContourCurveSlice;

class ContourCurveBillon
{
public:
	ContourCurveBillon( const Billon &billon );
	~ContourCurveBillon();

	const Billon &knotBillon() const;
	const QList<ContourCurveSlice> &contours() const;
	const ContourCurveSlice &contour( const uint &sliceIndex ) const;

	void compute(const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius );

private:
	const Billon &_initialBillon;
	Billon _resultBillon;

	QList<ContourCurveSlice> _contourCurves;
};

#endif // CONTOURCURVEBILLON_H
