#ifndef CONTOURCURVEBILLON_H
#define CONTOURCURVEBILLON_H

#include "def/def_billon.h"
#include "inc/billon.h"

template <typename T> class QVector;
class ContourCurveSlice;

class ContourCurveBillon
{
public:
	ContourCurveBillon();
	~ContourCurveBillon();

	const Billon &knotBillon() const;
	const QVector<ContourCurveSlice> &contours() const;
	const ContourCurveSlice &contour( const uint &sliceIndex ) const;

	void clear();
	bool isEmpty();

	void compute( const Billon &billon, const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius );

private:
	Billon _resultBillon;

	QVector<ContourCurveSlice> _contourCurves;
};

#endif // CONTOURCURVEBILLON_H
