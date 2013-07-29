#ifndef OFSEXPORT_H
#define OFSEXPORT_H

#include <QtGlobal>
#include "def/def_billon.h"
#include "def/def_coordinate.h"

template <typename T> class QVector;
template <typename T, typename U> struct QPair;
template <typename T> class Interval;

class QString;
class QTextStream;

namespace OfsExport
{
	void writeHeader( QTextStream &stream );
	void processOnPith( QTextStream &stream, const Billon &billon, const Interval<uint> &sliceInterval, const int &nbEdgesPerSlice,
						const int &radiusOfTubes, const bool &normalized );
	void processOnSector( QTextStream &stream, const Billon &billon, const int &nbEdgesPerSlice, const uint &radius, const Interval<uint> &sliceInterval,
						  const Interval<qreal> &angleInterval, const bool &normalized );
	void processOnAllSectorInAllIntervals( QTextStream &stream, const Billon &billon, const uint &nbEdgesPerSlice, const uint &radius,
										   const QVector< Interval<uint> > &sliceIntervals, const QVector< QVector< Interval<qreal> > > &angleIntervals, const bool &normalized );
	void processOnRestrictedMesh( QTextStream &stream, const Billon &billon, const Interval<uint> & sliceInterval, const QVector<rCoord2D> &vectVertex,
								  const uint & circleResolution, const bool &normalized, const bool &displayBegEndFaces = false );
}


#endif // OFSEXPORT_H
