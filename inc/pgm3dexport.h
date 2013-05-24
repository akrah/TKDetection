#ifndef PGM3DEXPORT_H
#define PGM3DEXPORT_H

#include "def/def_billon.h"

#include <QtGlobal>
#include <QString>


template<typename T> class Interval;
class QTextStream;
class QString;

namespace Pgm3dExport
{
  void process( const Billon &billon, const QString &fileName, const qreal &contrastFactor = 1., const int &componentNumber = 0 );
  void processImage( QTextStream &stream, const Billon &billon, const Interval<int> &slicesInterval,
               const Interval<int> &intensityInterval, const int &resolution, const qreal &contrastFactor );
  void processImageCartesian( QTextStream &stream, const Billon &billon, const Interval<int> &slicesInterval,
			      const Interval<int> &intensityInterval, const int &resolution, const int &angularResolution,const qreal &contrastFactor );
}

#endif // PGM3DEXPORT_H
