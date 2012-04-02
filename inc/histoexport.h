#ifndef HISTOEXPORT_H
#define HISTOEXPORT_H

#include <QString>
#include "slicehistogram.h"
#include "billon_def.h"
class SlicesInterval;
class Marrow;

namespace HistoExport {
  void process( const Billon &billon, const SliceHistogram &sliceHisto, const SlicesInterval &interval, const QString &fileName);
}


#endif // OFSEXPORT_H
