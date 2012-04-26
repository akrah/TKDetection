#ifndef PGM3DEXPORT_H
#define PGM3DEXPORT_H

#include "billon_def.h"

class QString;

namespace Pgm3dExport {
	void process( const Billon &billon, const QString &fileName );
}

#endif // PGM3DEXPORT_H
