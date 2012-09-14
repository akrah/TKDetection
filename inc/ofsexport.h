#ifndef OFSEXPORT_H
#define OFSEXPORT_H

#include <QString>
#include <QPair>

#include "billon_def.h"
class Interval;
class Marrow;
class PiePart;

namespace OfsExport {
	void process( const Billon &billon, const Marrow &marrow, const Interval &interval,
		      const QString &fileName, const int &nbEdgesPerSlice = 4, 
		      const int &radiusOfTubes = 10,  bool normalized=true);
	void processOnSector( const Billon &billon, const Marrow &marrow, const Interval &interval,
			      const QString &fileName, const qreal &rightAngle,
			      const qreal &leftAngle, const int &nbEdgesPerSlice = 4,
			      bool normalized=true);
	void processOnAllSectorInAllIntervals( const Billon &billon, const Marrow &marrow, 
					       const QVector< QPair< Interval, QPair<qreal,qreal> > > &intervals,
					       const QString &fileName, const int &nbEdgesPerSlice = 4,  bool normalized=true );
	void processRestrictedMesh( const Billon &billon, const Marrow &marrow, const Interval &interval, 
				    const QString &fileName, const int &resolutionCercle = 100,
				    const int &seuilContour = -900,  bool normalized=true ) ;
}


#endif // OFSEXPORT_H
