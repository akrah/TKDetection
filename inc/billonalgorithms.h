#ifndef BILLONALGORITHMS_H
#define BILLONALGORITHMS_H

#include "def/def_billon.h"
#include "def/def_coordinate.h"

template <typename T> class Interval;
template <typename T> class QVector;

namespace BillonAlgorithms
{
	qreal restrictedAreaMeansRadius(const Billon &billon, const uint &nbDirections, const int &intensityThreshold, const uint &minimumRadius, const uint &nbSlicesToIgnore );
	QVector<rCoord2D> restrictedAreaVertex( const Billon &billon, const Interval<uint> &sliceInterval, const uint &nbPolygonVertex, const int &intensityThreshold );
}

#endif // BILLONALGORITHMS_H
