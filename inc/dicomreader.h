#ifndef DICOMREADER_H
#define DICOMREADER_H

#include "billon_def.h"

class QString;

namespace DicomReader {

	Billon* read( const QString &repository );
}

#endif // DICOMREADER_H
