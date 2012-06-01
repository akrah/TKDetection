#ifndef HISTOEXPORT_H
#define HISTOEXPORT_H

#include <QString>
#include "slicehistogram.h"
#include "billon_def.h"
class Interval;
class Marrow;

namespace HistoExport {
  void process( const SliceHistogram &sliceHisto, const Interval &interval, const QString &fileName);
}


#endif // OFSEXPORT_H
