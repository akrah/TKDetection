#ifndef V3DEXPORT_H
#define V3DEXPORT_H

#include "def/def_billon.h"

class QString;
class QFile;
class Pith;
template<typename T> class Interval;

namespace V3DExport
{
	void process( QFile &file, const Billon &billon, const int &threshold );
}

#endif // V3DEXPORT_H
