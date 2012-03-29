#ifndef OFSEXPORT_H
#define OFSEXPORT_H

#include <QString>

#include "billon_def.h"
class SlicesInterval;
class Marrow;

namespace OfsExport {
  void process( const Billon &billon, const Marrow &marrow, const SlicesInterval &interval, const QString &fileName, const int &nbEdgesPerSlice = 4, const int &radiusOfTubes = 10 );
}


#endif // OFSEXPORT_H
