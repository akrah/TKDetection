#include "inc/ofsexport.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/slicesinterval.h"
#include "inc/global.h"

#include <QFile>
#include <QTextStream>
#include <qmath.h>

namespace OfsExport {

	namespace {
		qreal toOfsSystem( const int &pos, const qreal resolution, const int max );
		void computeAllEdges( const Billon &billon, const Marrow &marrow, const SlicesInterval &interval, const int &nbEdges, const int &radius, QTextStream &stream );
		void computeEgesLinks( const int &nbEdges, const int &nbSlices, QTextStream &stream );
	}

	void process( const Billon &billon, const Marrow &marrow, const SlicesInterval &interval, const QString &fileName ) {
		QFile file(fileName);
		if ( file.open(QIODevice::WriteOnly) ) {
			const int nbEdges = 4;
			const int radius = 10;

			QTextStream stream(&file);
			stream << "OFS MHD" << endl;
			computeAllEdges( billon, marrow, interval, nbEdges, radius, stream );
			computeEgesLinks( nbEdges, interval.count(), stream );

			file.close();
		}
	}

	namespace {
		inline qreal toOfsSystem( const int &pos, const int &max, const int &shift ) {
			return pos/max - shift;
		}

		void computeAllEdges( const Billon &billon, const Marrow &marrow, const SlicesInterval &interval, const int &nbEdges, const int &radius, QTextStream &stream ) {
			const int width = billon.n_cols;
			const int height = billon.n_rows;
			const int firstMarrow = interval.min() - marrow.beginSlice();
			const int lastMarrow = qMin(firstMarrow + interval.size(),marrow.size());
			qreal depth = -0.5;
			const qreal depthShift = 1./(qreal)interval.size();
			int i,k;

			QList<rCoord2D> offsets;
			const qreal angleShift = TWO_PI/nbEdges;
			const qreal ofsXRadius = radius/(qreal)width;
			const qreal ofsYRadius = radius/(qreal)height;
			qreal angle = PI_ON_FOUR;
			for ( i=0 ; i<nbEdges ; ++i ) {
				offsets.append( rCoord2D( qCos(angle)*ofsXRadius, qSin(angle)*ofsYRadius ) );
				angle += angleShift;
			}

			stream << endl;
			stream << nbEdges*(lastMarrow-firstMarrow+1) << endl;
			for ( k=firstMarrow ; k<=lastMarrow ; ++k ) {
				const iCoord2D &coord = marrow[k];
				const qreal xOfs = coord.x/(qreal)width - 0.5;
				const qreal yOfs = coord.y/(qreal)height - 0.5;
				for ( i=0 ; i<nbEdges ; ++i ) {
					stream << xOfs+offsets[i].x << ' ' << yOfs+offsets[i].y << ' ' << depth << endl;
				}
				depth += depthShift;
			}
		}

		void computeEgesLinks( const int &nbEdges, const int &nbSlices, QTextStream &stream ) {
			stream << endl;
			stream << nbEdges*2*(nbSlices-1) + 2*(nbEdges-2) << endl;
			// La face de devant
			for ( int i=1 ; i<nbEdges-1 ; ++i ) {
				stream << 0 << ' ' << i << ' ' << i+1 << endl;
			}
			// Les autres faces
			QList<qint32> tubes;
			int i;
			for ( i=0 ; i<nbEdges-1 ; ++i ) {
				tubes.append(i);
				tubes.append(i+nbEdges);
				tubes.append((i+nbEdges+1)%(2*nbEdges));
				tubes.append((i+1)%nbEdges);
			}
			tubes.append(i);
			tubes.append(i+nbEdges);
			tubes.append(nbEdges);
			tubes.append(0);
			for ( int base = 0 ; base<(nbSlices-1)*nbEdges ; base += nbEdges ) {
				for ( int i=0 ; i<4*nbEdges ; i+=4 ) {
					stream << base+tubes[i] << ' ' << base+tubes[i+1] << ' ' << base+tubes[i+2] << endl;
					stream << base+tubes[i] << ' ' << base+tubes[i+2] << ' ' << base+tubes[i+3] << endl;
				}
			}
			// La face de derrière
			const int lasBase = nbEdges*(nbSlices-1);
			for ( int i=lasBase+1 ; i<nbSlices*nbEdges-1 ; ++i ) {
				stream << lasBase << ' ' << i+1 << ' ' << i << endl;
			}
		}
	}
}
