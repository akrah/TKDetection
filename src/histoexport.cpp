#include "inc/histoexport.h"

#include "inc/interval.h"

#include <QFile>
#include <QTextStream>

namespace HistoExport {
	void process( const SliceHistogram &sliceHisto, const Interval &interval, const QString &fileName) {
			QFile file(fileName);
			if ( file.open(QIODevice::WriteOnly) ) {
				QTextStream stream(&file);
				for ( int i=interval.minValue() ; i<interval.maxValue() ; i++ ) {
					stream << i << " " <<  sliceHisto.value(i) << endl;
				}
				file.close();
			}
	}
}
