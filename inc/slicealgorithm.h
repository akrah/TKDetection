#ifndef SLICEALGORITHM_H
#define SLICEALGORITHM_H

#include "def/def_billon.h"
#include "def/def_coordinate.h"
#include "inc/coordinate.h"

#include <QtGlobal>

template <typename T> class QVector;
class QImage;
class QDataStream;
class QTextStream;

namespace SliceAlgorithm
{
	iCoord2D findNearestPointOfThePith( const Slice &slice, const iCoord2D & sliceCenter, const int &intensityThreshold );

	void draw( const Slice &slice, QImage &image, const int &intensityThreshold );
	void writeInPgm3D( const Slice &slice, QDataStream &stream );
	void writeInSDP( const Slice &slice, QTextStream &stream, const uint &sliceNum, const int &intensityThreshold );
}

#endif // SLICEALGORITHM_H
