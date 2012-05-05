#ifndef DATEXPORT_H
#define DATEXPORT_H

#include <QString>

#include "billon_def.h"
class SlicesInterval;
class IntensityInterval;

namespace DatExport {
	void process( const Billon &billon, const SlicesInterval &slicesInterval, const IntensityInterval &intensityIntervale, const QString &fileName, const int &resolution = 1, const qreal &contrastFactor = 1. );
}

#endif // DATEXPORT_H
