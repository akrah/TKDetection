#include "inc/histoexport.h"

#include "inc/slicesinterval.h"

#include <QFile>
#include <QTextStream>

namespace HistoExport {
	void process( const SliceHistogram &sliceHisto, const SlicesInterval &interval, const QString &fileName) {
			QFile file(fileName);
			if ( file.open(QIODevice::WriteOnly) ) {
				QTextStream stream(&file);
				for ( int i=interval.min() ; i<interval.max() ; i++ ) {
					stream << i << " " <<  sliceHisto.value(i) << endl;
				}
				file.close();
			}
	}
}
