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

	const QVector<ContourSlice> &contourSlices() const;
	const ContourSlice &contourSlice( const uint &sliceIndex ) const;

	void clear();
	bool isEmpty();

	void compute( Billon &resultBillon, const Billon &billon, const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius, const int &curvatureWidth );

private:
	QVector<ContourSlice> _contourSlices;
};

#endif // CONTOURBILLON_H
