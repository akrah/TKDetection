#ifndef OFSEXPORT_H
#define OFSEXPORT_H

#include <QtGlobal>
#include "def/def_billon.h"

template <typename T> class QVector;
template <typename T, typename U> class QPair;
template <typename T> class Interval;

class QString;
class Pith;
class PiePart;

namespace OfsExport
{
	void process(const Billon &billon, const Interval<int> &sliceInterval, const QString &fileName, const int &nbEdgesPerSlice, const int &radiusOfTubes = 4, bool normalized = true );
	void processOnSector( const Billon &billon, const Interval<uint> &interval, const QString &fileName, const qreal &rightAngle, const qreal &leftAngle, const int &nbEdgesPerSlice, bool normalized = true );
	void processOnAllSectorInAllIntervals( const Billon &billon, const QVector< QPair< Interval<uint>, QPair<qreal,qreal> > > &intervals, const QString &fileName, const int &nbEdgesPerSlice = 4,  bool normalized = true );
	void processOnRestrictedMesh( const Billon &billon, const Interval<uint> &sliceInterval, const QString &fileName, const uint &resolutionCercle, const int &seuilContour = 100, const bool &normalized = -900,  const bool &displayBegEndFaces = true );
}


#endif // OFSEXPORT_H
