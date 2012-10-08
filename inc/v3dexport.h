#ifndef V3DEXPORT_H
#define V3DEXPORT_H

#include "def/def_billon.h"

class QString;
class Pith;
template<typename T> class Interval;

namespace V3DExport
{
	void process( const Billon &billon, const Pith *pith, const QString &fileName, const Interval<int> &interval, const int &threshold );
}

#endif // V3DEXPORT_H
