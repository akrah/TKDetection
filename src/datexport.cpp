#include "inc/datexport.h"

#include "inc/billon.h"
#include "inc/slicesinterval.h"

#include <QFile>
#include <iostream>
#include <fstream>


using namespace std;

namespace DatExport {
	void process( const Billon &billon, const SlicesInterval &interval, const QString &fileName ) {
		ofstream stream;
		stream.open(fileName.toStdString().c_str());

		const int depth = interval.max() - interval.min() + 1;
		const int width = billon.n_cols;
		const int height = billon.n_rows;
		const int minValue = billon.minValue();
		const int maxValue = billon.maxValue();
		const qreal fact = 255.0/(maxValue-minValue);

		stream << "1 " << width << " " << height << " " << depth << " 1 1 1 1";

		for ( int k=0 ; k<depth ; ++k ) {
			const arma::Mat<__billon_type__> &slice = billon.slice(k);
			for ( int j=0 ; j<height ; ++j ) {
				for ( int i=0 ; i<width ; ++i ) {
					stream << (unsigned char) ((slice.at(j,i)-minValue)*fact);
				}
			}
		}

		stream.close();
	}
}
