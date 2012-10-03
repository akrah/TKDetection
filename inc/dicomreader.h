#ifndef DICOMREADER_H
#define DICOMREADER_H

#include "def/def_billon.h"

class QString;

namespace DicomReader {

	Billon* read( const QString &repository );
}

#endif // DICOMREADER_H
