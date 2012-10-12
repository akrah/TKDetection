#ifndef PGM3DEXPORT_H
#define PGM3DEXPORT_H

#include "def/def_billon.h"

#include <QtGlobal>

class QString;

namespace Pgm3dExport
{
	void process( const Billon &billon, const QString &fileName, const qreal &contrastFactor = 1., const int &componentNumber = 0 );
}

#endif // PGM3DEXPORT_H
