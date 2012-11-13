#ifndef V3DEXPORT_H
#define V3DEXPORT_H

#include <QtGlobal>
#include "def/def_billon.h"

class QString;
class QFile;
class Pith;
template<typename T> class Interval;

class QXmlStreamWriter;

namespace V3DExport
{
	void process( QFile &file, const Billon &billon, const Interval<uint> &sliceInterval, const int &threshold );

	void init( QFile &file, QXmlStreamWriter &stream );
	void close( QXmlStreamWriter &stream );

	void appendTags(QXmlStreamWriter &stream, const Billon &billon );
	void startComponents( QXmlStreamWriter &stream );
	void appendComponent( QXmlStreamWriter &stream, const Billon &billon, const Interval<uint> &sliceInterval, const int &index, const int &threshold );
	void endComponents( QXmlStreamWriter &stream );
	void appendPith( QXmlStreamWriter &stream, const Billon &billon );
}

#endif // V3DEXPORT_H
