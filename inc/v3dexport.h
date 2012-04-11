#ifndef V3DEXPORT_H
#define V3DEXPORT_H

#include "billon_def.h"
class QString;
class Marrow;
class SlicesInterval;

namespace V3DExport {
	void process( const Billon &billon, const Marrow *marrow, const QString &fileName, const SlicesInterval &interval, const int &threshold );
}

#endif // V3DEXPORT_H
