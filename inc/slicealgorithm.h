#ifndef SLICEALGORITHM_H
#define SLICEALGORITHM_H

#include "def/def_billon.h"

#include <QtGlobal>

class QImage;
class QDataStream;
class QTextStream;

namespace SliceAlgorithm
{
	void draw( const Slice &slice, QImage &image, const int &intensityThreshold );
	void writeInPgm3D( const Slice &slice, QDataStream &stream );
	void writeInSDP( const Slice &slice, QTextStream &stream, const uint &sliceNum, const int &intensityThreshold );
}

#endif // SLICEALGORITHM_H
