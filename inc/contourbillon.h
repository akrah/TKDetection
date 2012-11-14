#ifndef CONTOURBILLON_H
#define CONTOURBILLON_H

#include "def/def_billon.h"
#include "inc/billon.h"

template <typename T> class QVector;
class ContourSlice;
class CurvatureHistogram;

class ContourBillon
{
public:
	ContourBillon();
	~ContourBillon();

	const Billon &knotBillon() const;
	const QVector<ContourSlice> &contours() const;
	const ContourSlice &contour( const uint &sliceIndex ) const;

	void clear();
	bool isEmpty();

	void compute( const Billon &billon, const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius );

private:
	Billon _resultBillon;

	QVector<ContourSlice> _contourCurves;
};

#endif // CONTOURBILLON_H
