#include "inc/ofsexport.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/slicesinterval.h"
#include "inc/global.h"
#include "inc/piepart.h"

#include <QFile>
#include <QTextStream>
#include <QVector>
#include <qmath.h>

#include <QDebug>

namespace OfsExport {

	namespace {
		// Calcul les coordonnées des sommets du maillage de la moelle
		void computeAllEdges( const Billon &billon, const Marrow &marrow, const SlicesInterval &interval, const int &nbEdges, const int &radius, QTextStream &stream );
		void computeSectorEdges( const Billon &billon, const Marrow &marrow, const SlicesInterval &interval, const int &nbEdges, const qreal &rightAngle, const qreal &leftAngle, QTextStream &stream );

		//Rajout BK: Affiche les coordonnées des sommets pour l'export OFS (utile iuniquement pour le maillage de la zone réduite)
		void displayExportedVertex( const Billon &billon, const Marrow &marrow, QVector<rCoord2D> vectVertex, const SlicesInterval &interval, const int &resolutionCercle, QTextStream &stream );

		// Calcul les faces du maillages de la moelle
		void computeEgesLinks( const int &nbEdges, const int &nbSlices, QTextStream &stream );
	}

	void process( const Billon &billon, const Marrow &marrow, const SlicesInterval &interval, const QString &fileName, const int &nbEdgesPerSlice, const int &radiusOfTubes ) {
		if ( interval.containsClosed(marrow.interval()) ) {
			QFile file(fileName);
			if ( file.open(QIODevice::WriteOnly) ) {
				QTextStream stream(&file);
				stream << "OFS MHD" << endl;
				computeAllEdges( billon, marrow, interval, nbEdgesPerSlice, radiusOfTubes, stream );
				computeEgesLinks( nbEdgesPerSlice, interval.count(), stream );
				file.close();
			}
		}
		else {
			qDebug() << QObject::tr("Saving not possible since the mesh is outside the SlicesInterval");
		}
	}

	void processOnSector( const Billon &billon, const Marrow &marrow, const SlicesInterval &interval, const QString &fileName, const qreal &rightAngle, const qreal &leftAngle, const int &nbEdgesPerSlice ) {
		if ( interval.containsClosed(marrow.interval()) ) {
			QFile file(fileName);
			if ( file.open(QIODevice::WriteOnly) ) {
				QTextStream stream(&file);
				stream << "OFS MHD" << endl;
				computeSectorEdges( billon, marrow, interval, nbEdgesPerSlice, rightAngle, leftAngle, stream );
				computeEgesLinks( nbEdgesPerSlice, interval.count(), stream );
				file.close();
			}
		}
		else {
			qDebug() << QObject::tr("Saving not possible since the mesh is outside the SlicesInterval");
		}
	}

	void processRestrictedMesh( const Billon &billon, const Marrow &marrow, const SlicesInterval &interval, const QString &fileName, const int &resolutionCercle, const int &seuilContour ) {
		  QVector<rCoord2D> vectVertex = billon.getRestrictedAreaVertex( resolutionCercle,seuilContour, &marrow );

		  QFile file(fileName);
		  if ( file.open(QIODevice::WriteOnly) ) {
			QTextStream stream(&file);
			stream << "OFS MHD" << endl;
			displayExportedVertex(billon, marrow, vectVertex, interval, resolutionCercle, stream);
			computeEgesLinks( resolutionCercle, interval.count(), stream );
			file.close();
		  }

	}


	namespace {


		void computeAllEdges( const Billon &billon, const Marrow &marrow, const SlicesInterval &interval, const int &nbEdges, const int &radius, QTextStream &stream ) {
			const int width = billon.n_cols;
			const int height = billon.n_rows;
			const int nbSlices = interval.size();
			const int firstMarrow = interval.min() - marrow.interval().min();
			const int lastMarrow = qMin(firstMarrow + nbSlices,marrow.size());
			qreal depth = -0.5;
			const qreal depthShift = 1./(qreal)nbSlices;
			int i,k;

			QVector<rCoord2D> offsets;
			const qreal angleShift = TWO_PI/(qreal)nbEdges;
			const qreal ofsXRadius = radius/(qreal)width;
			const qreal ofsYRadius = radius/(qreal)height;
			qreal angle = 0;
			while ( angle < TWO_PI ) {
				offsets.append( rCoord2D( qCos(angle)*ofsXRadius, qSin(angle)*ofsYRadius ) );
				angle += angleShift;
			}

			stream << endl;
			stream << nbEdges*(lastMarrow-firstMarrow+1) << endl;
			rCoord2D *offsetsIterator = 0;
			for ( k=firstMarrow ; k<=lastMarrow ; ++k ) {
				const iCoord2D &coord = marrow[k];
				const qreal xOfs = coord.x/(qreal)width - 0.5;
				const qreal yOfs = coord.y/(qreal)height - 0.5;
				offsetsIterator = offsets.data();
				for ( i=0 ; i<nbEdges ; ++i ) {
					stream << xOfs+offsetsIterator->x << ' ' << yOfs+offsetsIterator->y << ' ' << depth << endl;
					offsetsIterator++;
				}
				depth += depthShift;
			}
		}

		void computeSectorEdges( const Billon &billon, const Marrow &marrow, const SlicesInterval &interval, const int &nbEdges, const qreal &rightAngle, const qreal &leftAngle, QTextStream &stream ) {
			const int width = billon.n_cols;
			const int height = billon.n_rows;
			const int nbSlices = interval.size();
			const int firstMarrow = interval.min() - marrow.interval().min();
			const int lastMarrow = qMin(firstMarrow + nbSlices,marrow.size());
			const qreal depthShift = 1./(qreal)nbSlices;
			const qreal angleShift = (rightAngle<leftAngle?leftAngle-rightAngle:leftAngle+(TWO_PI-rightAngle))/(qreal)nbEdges;
			int i,k;

			stream << endl;
			stream << nbEdges*(lastMarrow-firstMarrow+1) << endl;

			QVector<rCoord2D> offsets;
			rCoord2D *offsetsIterator = 0;
			qreal depth = -0.5;
			qreal angle;
			for ( k=firstMarrow ; k<=lastMarrow ; ++k ) {
				const iCoord2D &coord = marrow[k];
				const qreal xOfs = coord.x/(qreal)width - 0.5;
				const qreal yOfs = coord.y/(qreal)height - 0.5;
				const qreal ofsXRadius = qMin(coord.x,width-coord.x)/(qreal)width;
				const qreal ofsYRadius = qMin(coord.y,width-coord.y)/(qreal)height;
				angle = rightAngle<leftAngle?rightAngle:-TWO_PI+rightAngle;
				offsets.clear();
				while ( angle < leftAngle ) {
					offsets.append( rCoord2D( qCos(angle)*ofsXRadius, qSin(angle)*ofsYRadius ) );
					angle += angleShift;
				}
				offsetsIterator = offsets.data();
				for ( i=0 ; i<nbEdges ; ++i ) {
					stream << xOfs+offsetsIterator->x << ' ' << yOfs+offsetsIterator->y << ' ' << depth << endl;
					offsetsIterator++;
				}
				depth += depthShift;
			}
		}

		void displayExportedVertex( const Billon &billon, const Marrow &marrow, QVector<rCoord2D> vectVertex, const SlicesInterval &interval,const int &resolutionCercle, QTextStream &stream ){
			const int width = billon.n_cols;
			const int height = billon.n_rows;
			const int firstMarrow = interval.min() - marrow.interval().min();
			const int nbSlices=interval.size();
			const int lastMarrow = qMin(firstMarrow + nbSlices,marrow.size());
			qreal depth = -0.5;
			stream << endl;
			stream << resolutionCercle*(lastMarrow-firstMarrow+1) << endl;
			const qreal depthShift = 1./(qreal)nbSlices;
			int pos=0;
			for (int k=firstMarrow ; k<=lastMarrow ; ++k ) {
				pos=(k-firstMarrow)*resolutionCercle;
				for(int i=0; i< resolutionCercle; i++){
					if((k==firstMarrow) && i==0)
						stream << 0 << ' ' << 0 << ' ' << -0.5 << endl;
					else if((k==lastMarrow) && i==0)
						stream << 0 << ' ' << 0 << ' ' << depth << endl;
					else {
						stream << ((vectVertex.at(pos).x /(qreal)width) -0.5)<< ' ' << ((vectVertex.at(pos).y/(qreal)height) -0.5)<< ' ' << depth << endl;
					}
					 pos++;
				}
				depth += depthShift;
			}
		}

		void computeEgesLinks( const int &nbEdges, const int &nbSlices, QTextStream &stream ) {
			const int nbPoints = nbEdges*(nbSlices-1);
			const int nbPointsFoTube = 4*nbEdges;

			stream << endl;
			stream << 2*(nbEdges+nbPoints-2) << endl;
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
				tubes.append(i+nbEdges+1);
				tubes.append(i+1);
			}
			tubes.append(i);
			tubes.append(i+nbEdges);
			tubes.append(nbEdges);
			tubes.append(0);
			for ( int base = 0 ; base<nbPoints ; base += nbEdges ) {
				for ( int i=0 ; i<nbPointsFoTube ; i+=4 ) {
					stream << base+tubes[i] << ' ' << base+tubes[i+1] << ' ' << base+tubes[i+2] << endl;
					stream << base+tubes[i] << ' ' << base+tubes[i+2] << ' ' << base+tubes[i+3] << endl;
				}
			}
			// La face de derrière
						const int lasBase = nbPoints;
						for ( int i=lasBase+1 ; i<nbSlices*nbEdges-1 ; ++i ) {
				stream << lasBase << ' ' << i+1 << ' ' << i << endl;
			}
		}
	}
}
