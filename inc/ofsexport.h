#ifndef OFSEXPORT_H
#define OFSEXPORT_H

#include <QString>
#include <QPair>

#include "billon_def.h"
template<typename T> class Interval;
class Marrow;
class PiePart;

namespace OfsExport {
	void process( const Billon &billon, const Marrow &marrow, const Interval<int> &interval,
			  const QString &fileName, const int &nbEdgesPerSlice = 4,
			  const int &radiusOfTubes = 10,  bool normalized=true);
	void processOnSector( const Billon &billon, const Marrow &marrow, const Interval<int> &interval,
				  const QString &fileName, const qreal &rightAngle,
				  const qreal &leftAngle, const int &nbEdgesPerSlice = 4,
				  bool normalized=true);
	void processOnAllSectorInAllIntervals( const Billon &billon, const Marrow &marrow,
						   const QVector< QPair< Interval<int>, QPair<qreal,qreal> > > &intervals,
						   const QString &fileName, const int &nbEdgesPerSlice = 4,  bool normalized=true );
	void processRestrictedMesh( const Billon &billon, const Marrow &marrow, const Interval<uint> &interval, const QString &fileName, const uint resolutionCercle = 100, const int seuilContour = -900,  const bool normalized = true, const bool displayBegEndFaces = true ) ;
}


#endif // OFSEXPORT_H
