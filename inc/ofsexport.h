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
						const int &radiusOfTubes, const bool &normalized = false );
	void processOnSector( QTextStream &stream, const Billon &billon, const qreal &minAngle, const qreal &maxAngle,
						  const int &nbEdgesPerSlice, const bool &normalized = false );
	void processOnAllSectorInAllIntervals( QTextStream &stream, const Billon &billon, const QVector< QPair< Interval<uint>, QPair<qreal,qreal> > > &intervals,
										   const int &nbEdgesPerSlice,  const bool &normalized = false );
	void processOnRestrictedMesh( QTextStream &stream, const Billon &billon, const Interval<uint> & sliceInterval, const QVector<rCoord2D> &vectVertex,
								  const uint & circleResolution, const bool &normalized, const bool &displayBegEndFaces = false );
}


#endif // OFSEXPORT_H
