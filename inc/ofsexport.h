#ifndef OFSEXPORT_H
#define OFSEXPORT_H

#include <QString>

#include "billon_def.h"
class SlicesInterval;
class Marrow;

namespace OfsExport {
	void process( const Billon &billon, const Marrow &marrow, const SlicesInterval &interval, const QString &fileName = "output.ofs" );
}


#endif // OFSEXPORT_H
