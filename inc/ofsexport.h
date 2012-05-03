#ifndef OFSEXPORT_H
#define OFSEXPORT_H

#include <QString>

#include "billon_def.h"
class SlicesInterval;
class Marrow;

namespace OfsExport {
	void process( const Billon &billon, const Marrow &marrow, const SlicesInterval &interval, const QString &fileName, const int &nbEdgesPerSlice = 4, const int &radiusOfTubes = 10 );
        void processRestrictedMesh( Billon &billon, const Marrow &marrow, const QString &fileName, const int &resolutionCercle = 100, const int &seuilContour = -900 ) ;

}


#endif // OFSEXPORT_H
