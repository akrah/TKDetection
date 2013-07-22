#ifndef CONTOURBILLON_H
#define CONTOURBILLON_H

#include "def/def_billon.h"
#include "inc/billon.h"

template <typename T> class QVector;
class ContourSlice;
class CurvatureHistogram;
class NearestPointsHistogram;

class ContourBillon
{
public:
	ContourBillon();
	~ContourBillon();

	const QVector<ContourSlice> &contourSlices() const;
	const ContourSlice &contourSlice( const uint &sliceIndex ) const;

	void clear();
	bool isEmpty();

	void compute( Billon &resultBillon, const Billon &billon, const int &intensityThreshold, const int &smoothingRadius,
				  const int &curvatureWidth, const qreal &curvatureThreshold, const QVector< Interval<uint> > &sliceIntervals,
				  const Interval<qreal> &angularInterval , const uint &minimumDistanceFromContourOrigin );

private:
	QVector<ContourSlice> _contourSlices;
};

#endif // CONTOURBILLON_H
