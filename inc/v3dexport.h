#ifndef V3DEXPORT_H
#define V3DEXPORT_H

#include <QtGlobal>
#include "def/def_billon.h"

class QString;
class QFile;
class Pith;
template<typename T> class Interval;
class TangentialGenerator;

class QXmlStreamWriter;

namespace V3DExport
{
	void init( QFile &file, QXmlStreamWriter &stream );
	void close( QXmlStreamWriter &stream );

	void appendTags( QXmlStreamWriter &stream, const Billon &billon );
	void startComponents( QXmlStreamWriter &stream );
	void appendSegmentedKnotArea( QXmlStreamWriter &stream, const uint &knotAreaIndex, const Billon &tangentialBillon, const TangentialGenerator &tangentialGenerator );
	void endComponents( QXmlStreamWriter &stream );
	void appendBillonPith( QXmlStreamWriter &stream, const Billon &billon , const uint &firstSlicePos );
	void appendTangentialPith( QXmlStreamWriter &stream, const Pith &pith, const TangentialGenerator &tangentialGenerator );
}

#endif // V3DEXPORT_H
