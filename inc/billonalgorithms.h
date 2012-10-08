#ifndef BILLONALGORITHMS_H
#define BILLONALGORITHMS_H

#include "def/def_billon.h"
#include "def/def_coordinate.h"
#include "inc/coordinate.h"

template <typename T> class Interval;
template <typename T> class QVector;
class Pith;

namespace BillonAlgorithms
{
	iCoord2D findNearestPointOfThePith( const Slice &slice, const iCoord2D & sliceCenter, const int &intensityThreshold );
	QVector<iCoord2D> extractContour( const Slice &slice, const iCoord2D & sliceCenter, int intensityThreshold, iCoord2D startPoint = iCoord2D(-1,-1) );
	qreal getRestrictedAreaMeansRadius( const Billon &billon, const Pith &pith, const uint &nbPolygonPoints, int intensityThreshold );
	QVector<rCoord2D> getRestrictedAreaVertex( const Billon &billon, const Pith &pith, const Interval<uint> &sliceInterval, const uint &nbPolygonPoints, const int &intenstyThreshold );
}

#endif // BILLONALGORITHMS_H
