#ifndef DATEXPORT_H
#define DATEXPORT_H

#include <QString>

#include "billon_def.h"
class SlicesInterval;

namespace DatExport {
	void process( const Billon &billon, const SlicesInterval &interval, const QString &fileName = "output.dat" );
}

#endif // DATEXPORT_H
