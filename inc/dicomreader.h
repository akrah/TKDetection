#ifndef DICOMREADER_H
#define DICOMREADER_H

#include "def/def_billon.h"

class QString;

namespace DicomReader
{
	Billon* read( const QString &repository, const bool &sliceOrderInversed = false );
}

#endif // DICOMREADER_H
