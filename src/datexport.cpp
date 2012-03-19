#include "inc/datexport.h"

#include "inc/billon.h"
#include "inc/slicesinterval.h"

#include <QFile>
#include <QDataStream>
#include <QTextStream>

namespace DatExport {
	void process( const Billon &billon, const SlicesInterval &interval, const QString &fileName, const int &resolution ) {
		QFile file(fileName);
		if ( file.open(QIODevice::WriteOnly) ) {
			const int firstSlice = interval.min();
			const int lastSlice = interval.max();
			const int width = billon.n_cols;
			const int height = billon.n_rows;
			const int minValue = billon.minValue();
			const int shift = resolution-1;
			const qreal fact = 255.0/(billon.maxValue()-minValue);

			QTextStream tStream(&file);
			tStream << "1 " << width/resolution << " " << height/resolution << " " << interval.count() << " 1 1 1 1";
			tStream.flush();

			QDataStream dStream(&file);
			for ( int k=firstSlice ; k<=lastSlice ; ++k ) {
				const arma::Slice &slice = billon.slice(k);
				for ( int j=0 ; j<height-shift ; j+=resolution ) {
					for ( int i=0 ; i<width-shift ; i+=resolution ) {
						dStream << (qint8) ((arma::mean(arma::mean(slice.submat(j,i,j+shift,i+shift)))-minValue)*fact);
					}
				}
			}

			file.close();
		}
	}
}
