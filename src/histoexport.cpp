#include "inc/histoexport.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/slicesinterval.h"
#include "inc/global.h"

#include <QFile>
#include <QTextStream>
#include <QVector>
#include <qmath.h>

#include <QDebug>

namespace HistoExport {

	namespace {
		// Calcul les coordonnées des sommets du maillage de la moelle
                void computeAllEdges( const Billon &billon, const SliceHistogram &sliceHisto, const SlicesInterval &interval, QTextStream &stream );
	}

        void process( const Billon &billon, const SliceHistogram &sliceHisto, const SlicesInterval &interval, const QString &fileName) {

                        QFile file(fileName);
                        if ( file.open(QIODevice::WriteOnly) ) {
                                QTextStream stream(&file);
                                for(int i=interval.min(); i<interval.max(); i++){
                                    stream << i << " " <<  sliceHisto.value(i) << endl;
                                }
                                file.close();
                        }
	}




}
