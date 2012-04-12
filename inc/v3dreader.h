#ifndef V3DREADER_H
#define V3DREADER_H

#include "billon_def.h"

class QString;

namespace V3DReader {

	Billon* read( const QString &repository );
}

#endif // V3DREADER_H
