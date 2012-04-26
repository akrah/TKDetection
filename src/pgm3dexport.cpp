#include "inc/pgm3dexport.h"

#include "inc/billon.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

namespace Pgm3dExport {

	void process( const Billon &billon, const QString &fileName ) {
		QFile file(fileName);

		if( !file.open(QIODevice::WriteOnly) ) {
			qDebug() << QObject::tr("ERREUR : Impossible de créer le ficher PGM %1.").arg(fileName);
			return;
		}

		const uint width = billon.n_cols;
		const uint height = billon.n_rows;
		const uint depth = billon.n_slices;
		const int minValue = billon.minValue();

		QTextStream stream(&file);

		stream << "P3D" << endl;
		stream << width << " " << height << " " << depth << endl;
		stream << billon.maxValue()-minValue << endl;

		uint i, j, k;
		for ( k=0 ; k<depth ; ++k ) {
			const arma::Slice &slice = billon.slice(k);
			for ( j=0 ; j<height ; ++j ) {
				for ( i=0 ; i<width ; ++i ) {
					stream << (char)(slice.at(j,i)-minValue);
				}
			}
		}

		file.close();
	}

}
