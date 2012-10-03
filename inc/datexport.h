#ifndef DATEXPORT_H
#define DATEXPORT_H

#include <QString>

#include "def/def_billon.h"

template<typename T> class Interval;

namespace DatExport
{
	void process( const Billon &billon, const Interval<int> &slicesInterval, const Interval<int> &intensityIntervale, const QString &fileName, const int &resolution = 1, const qreal &contrastFactor = 1. );
}

#endif // DATEXPORT_H
