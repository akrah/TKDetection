#ifndef DICOMREADER_H
#define DICOMREADER_H

class Billon;
class QString;

namespace DicomReader {

	Billon* read( const QString &repository );
}

#endif // DICOMREADER_H
