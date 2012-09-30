#ifndef V3DEXPORT_H
#define V3DEXPORT_H

#include "billon_def.h"
class QString;
class Marrow;
template<typename T> class Interval;

namespace V3DExport {
	void process( const Billon &billon, const Marrow *marrow, const QString &fileName, const Interval<int> &interval, const int &threshold );
}

#endif // V3DEXPORT_H
