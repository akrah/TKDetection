#ifndef CONTOURCURVEBILLON_H
#define CONTOURCURVEBILLON_H

#include "def/def_billon.h"
#include "inc/billon.h"

template <typename T> class QVector;
class ContourCurveSlice;

class ContourCurveBillon
{
public:
	ContourCurveBillon( const Billon &billon );

private:
	void intialize(const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius );

private:
	const Billon &_initialBillon;
	Billon _resultBillon;

	QVector<ContourCurveSlice> _contourCurves;
};

#endif // CONTOURCURVEBILLON_H
