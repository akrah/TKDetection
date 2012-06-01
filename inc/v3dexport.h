#ifndef V3DEXPORT_H
#define V3DEXPORT_H

#include "billon_def.h"
class QString;
class Marrow;
class Interval;

namespace V3DExport {
	void process( const Billon &billon, const Marrow *marrow, const QString &fileName, const Interval &interval, const int &threshold );
}

#endif // V3DEXPORT_H
