#ifndef DATEXPORT_H
#define DATEXPORT_H

#include <QString>

#include "def/def_billon.h"

template<typename T> class Interval;
class QTextStream;

namespace DatExport
{
	void process( QTextStream &stream, const Billon &billon, const Interval<int> &slicesInterval, const Interval<int> &intensityIntervale, const int &resolution = 1, const qreal &contrastFactor = 1. );
}

#endif // DATEXPORT_H
