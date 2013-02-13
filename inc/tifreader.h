#ifndef TIFREADER_H
#define TIFREADER_H

#include "def/def_billon.h"

class QString;

namespace TifReader
{
	Billon* read( const QString &filename );
}

#endif // TIFREADER_H
