#include "inc/ofsexport.h"

#include "def/def_coordinate.h"
#include "inc/coordinate.h"
#include "inc/billon.h"
#include "inc/billonalgorithms.h"
#include "inc/pith.h"

#include <QTextStream>
#include <QFile>
#include <QDebug>

namespace OfsExport
{
	namespace
	{
		// Calcul les coordonnées des sommets du maillage de la moelle
		void computeAndWriteAllEdges( QTextStream &stream,const Billon &billon, const Interval<uint> &sliceInterval, const uint &nbEdges, const uint &radius, const bool &normalized );
		void computeAndWriteSectorEdges( QTextStream &stream,const Billon &billon, const Interval<uint> &sliceInterval, const uint &nbEdges, const qreal &rightAngle, const qreal &leftAngle, const bool &normalized);
		void computeAndWriteAllSectorInAllIntervalsEdges( QTextStream &stream, const Billon &billon, const QVector< QPair< Interval<uint>, QPair<qreal, qreal> > > &intervals, const uint &nbEdges, const bool &normalized );

		// Calcul les faces du maillages de la moelle
		void computeAndWriteEdgesLinks( QTextStream &stream, const int &nbEdges, const int &nbSlices, bool displayBegEndFaces );

		//Rajout BK: Affiche les coordonnées des sommets pour l'export OFS (utile iuniquement pour le maillage de la zone réduite)
		void writeExportedVertex( QTextStream &stream, const Billon &billon, const QVector<rCoord2D> &vectVertex, const Interval<uint> &interval, const uint &resolutionCercle, const bool &normalized, const bool &displayBegEndFaces );
	}

	void process( const Billon &billon, const Interval<uint> &sliceInterval, const QString &fileName, const int &nbEdgesPerSlice, const int &radiusOfTubes, const bool &normalized )
	{
		QFile file(fileName);
		if ( file.open(QIODevice::WriteOnly) )
		{
			QTextStream stream(&file);
			stream << "OFS MHD" << endl;
			computeAndWriteAllEdges( stream, billon, sliceInterval, nbEdgesPerSlice, radiusOfTubes, normalized );
			computeAndWriteEdgesLinks( stream, nbEdgesPerSlice, sliceInterval.width()+1, true );
			file.close();
		}
		else
		{
			qDebug() << QObject::tr("%1 file writing error.").arg(fileName);
		}
	}

	void processOnSector( const Billon &billon, const Interval<uint> &interval, const QString &fileName, const qreal &rightAngle, const qreal &leftAngle, const int &nbEdgesPerSlice, const bool &normalized )
	{
		QFile file(fileName);
		if ( file.open(QIODevice::WriteOnly) )
		{
			QTextStream stream(&file);
			stream << "OFS MHD" << endl;
			computeAndWriteSectorEdges( stream, billon, interval, nbEdgesPerSlice, rightAngle, leftAngle, normalized );
			computeAndWriteEdgesLinks( stream, nbEdgesPerSlice, interval.width()+1, true );
			file.close();
		}
		else
		{
			qDebug() << QObject::tr("%1 file writing error.").arg(fileName);
		}
	}

	void processOnAllSectorInAllIntervals( const Billon &billon, const QVector<QPair<Interval<uint>, QPair<qreal, qreal> > > &intervals, const QString &fileName, const int &nbEdgesPerSlice, const bool &normalized )
	{
		QFile file(fileName);
		if ( file.open(QIODevice::WriteOnly) )
		{
			QTextStream stream(&file);
			stream << "OFS MHD" << endl;
			computeAndWriteAllSectorInAllIntervalsEdges( stream, billon, intervals, nbEdgesPerSlice, normalized );

			QString fullStream;
			int sumNbLinks = 0;
			int currentBase = 0;
			const int nbPointsFoTube = 4*nbEdgesPerSlice;
			for ( int k=0 ; k<intervals.size() ; ++k )
			{
				const int nbSlices = intervals[k].first.size();
				const int nbPoints = nbEdgesPerSlice*(nbSlices-1);

				sumNbLinks += 2*(nbEdgesPerSlice+nbPoints-2);

				// La face de devant
				for ( int i=1 ; i<nbEdgesPerSlice-1 ; ++i )
				{
					stream << currentBase << ' ' << currentBase+i << ' ' << currentBase+i+1 << endl;
				}
				// Les autres faces
				QList<qint32> tubes;
				int i;
				for ( i=0 ; i<nbEdgesPerSlice-1 ; ++i )
				{
					tubes.append(currentBase+i);
					tubes.append(currentBase+i+nbEdgesPerSlice);
					tubes.append(currentBase+i+nbEdgesPerSlice+1);
					tubes.append(currentBase+i+1);
				}
				tubes.append(currentBase+i);
				tubes.append(currentBase+i+nbEdgesPerSlice);
				tubes.append(currentBase+nbEdgesPerSlice);
				tubes.append(currentBase);
				for ( int base = 0 ; base<nbPoints ; base += nbEdgesPerSlice )
				{
					for ( int i=0 ; i<nbPointsFoTube ; i+=4 ) {
						stream << base+tubes[i] << ' ' << base+tubes[i+1] << ' ' << base+tubes[i+2] << endl;
						stream << base+tubes[i] << ' ' << base+tubes[i+2] << ' ' << base+tubes[i+3] << endl;
					}
				}
				// La face de derrière
				const int lastBase = currentBase+nbPoints;
				for ( int i=lastBase+1 ; i<nbSlices*nbEdgesPerSlice-1 ; ++i )
				{
					stream << lastBase << ' ' << i+1 << ' ' << i << endl;
				}

				currentBase += nbSlices*nbEdgesPerSlice;
			}
			stream << endl;
			stream << sumNbLinks << endl;
			stream << fullStream;

			file.close();
		}
		else
		{
			qDebug() << QObject::tr("%1 file writing error.").arg(fileName);
		}
	}

	void processOnRestrictedMesh( const Billon &billon, const Interval<uint> & sliceInterval, const QString &fileName, const uint & circleResolution, const int &contourThreshold, const bool &normalized, const bool &displayBegEndFaces )
	{
		QVector<rCoord2D> vectVertex = BillonAlgorithms::restrictedAreaVertex( billon, sliceInterval, circleResolution, contourThreshold );
		if ( !vectVertex.isEmpty() )
		{
			QFile file(fileName);
			if ( file.open(QIODevice::WriteOnly) )
			{
				QTextStream stream(&file);
				stream << "OFS MHD" << endl;
				writeExportedVertex(stream, billon, vectVertex, sliceInterval, circleResolution, normalized, displayBegEndFaces);
				computeAndWriteEdgesLinks( stream, circleResolution, sliceInterval.width()+1, displayBegEndFaces );
				file.close();
			}
			else
			{
				qDebug() << QObject::tr("%1 file writing error.").arg(fileName);
			}
		}
		else
		{
			qDebug() << QObject::tr("No vertex to write");
		}
	}


	namespace
	{
		void computeAndWriteAllEdges( QTextStream &stream, const Billon &billon, const Interval<uint> &sliceInterval, const uint &nbEdges, const uint &radius, const bool &normalized )
		{
			const int width = billon.n_cols;
			const int height = billon.n_rows;
			const int nbSlices = sliceInterval.size();
			const qreal depthShift = 1./(normalized? (qreal)nbSlices: 1.0);
			const qreal angleShift = TWO_PI/(qreal)nbEdges;
			const rCoord2D norm = normalized ? rCoord2D(width,height) : rCoord2D(1.,1.);
			const rCoord2D ofsRadius = rCoord2D( radius, radius )/norm;

			uint i, k;

			QVector<rCoord2D> offsets;
			offsets.reserve(nbEdges);
			qreal angle = 0;

			while ( angle < TWO_PI )
			{
				offsets.append( ofsRadius * rCoord2D( qCos(angle), qSin(angle) ) );
				angle += angleShift;
			}

			const rCoord2D ofsStart(normalized?0.5:width/2,normalized?0.5:height/2);
			rCoord2D *offsetsIterator, ofs;
			qreal depth = normalized ? -0.5 : -(qreal)(sliceInterval.size()/2);

			stream << endl;
			stream << nbEdges*(sliceInterval.width()+1) << endl;
			for ( k=sliceInterval.min() ; k<=sliceInterval.max() ; ++k )
			{
				ofs = billon.pithCoord(k)/norm - ofsStart;
				offsetsIterator = offsets.data();
				for ( i=0 ; i<nbEdges ; ++i )
				{
					stream << ofs.x+offsetsIterator->x << ' ' << ofs.y+offsetsIterator->y << ' ' << depth << endl;
					offsetsIterator++;
				}
				depth += depthShift;
			}
		}

		// Les angles doivent être compris entre 0 et 360
		void computeAndWriteSectorEdges( QTextStream &stream, const Billon &billon, const Interval<uint> &sliceInterval, const uint &nbEdges, const qreal &rightAngle, const qreal &leftAngle, const bool &normalized )
		{
			const int width = billon.n_cols;
			const int height = billon.n_rows;
			const int nbSlices = sliceInterval.size();
			const qreal depthShift = 1./(normalized? (qreal)nbSlices:1.);
			const qreal angleShift = (rightAngle<leftAngle?leftAngle-rightAngle:leftAngle+(TWO_PI-rightAngle))/(qreal)(nbEdges-1);
			uint i, k;

			stream << endl;
			stream << nbEdges*(nbSlices+1) << endl;

			const rCoord2D ofsStart(normalized?0.5:width/2,normalized?0.5:height/2);
			const rCoord2D norm = normalized ? rCoord2D(width,height) : rCoord2D(1.,1.);
			QVector<rCoord2D> offsets;
			rCoord2D *offsetsIterator, ofs, ofsRadius;
			iCoord2D coord;
			qreal depth = normalized ? -0.5 : -(qreal)(sliceInterval.size()/2);
			qreal angle;
			for ( k=sliceInterval.min() ; k<=sliceInterval.max() ; ++k )
			{
				coord = billon.pithCoord(k);
				ofs = coord/norm - ofsStart;
				ofsRadius = rCoord2D( qMin(coord.x,width-coord.x), qMin(coord.y,width-coord.y) )/norm;
				angle = rightAngle<leftAngle?rightAngle:-TWO_PI+rightAngle;
				offsets.clear();
				while ( angle < leftAngle )
				{
					offsets.append( ofsRadius * rCoord2D( qCos(angle), qSin(angle) ) );
					angle += angleShift;
				}
				offsetsIterator = offsets.data();
				stream << ofs.x << ' ' << ofs.y << ' ' << depth << endl;
				for ( i=0 ; i<nbEdges-1 ; ++i )
				{
					stream << ofs.x+offsetsIterator->x << ' ' << ofs.y+offsetsIterator->y << ' ' << depth << endl;
					offsetsIterator++;
				}
				depth += depthShift;
			}
		}



		void computeAndWriteAllSectorInAllIntervalsEdges( QTextStream &stream, const Billon &billon, const QVector< QPair< Interval<uint>, QPair<qreal,qreal> > > &intervals, const uint &nbEdges, const bool &normalized )
		{
			const int width = billon.n_cols;
			const int height = billon.n_rows;

			QTextStream fullStream;
			int sumOfnbEdges = 0;
			QVector<rCoord2D> offsets;
			rCoord2D *offsetsIterator, ofs, ofsRadius;
			qreal depth, angle;
			uint l;

			const rCoord2D ofsStart(normalized?0.5:width/2,normalized?0.5:height/2);
			const rCoord2D norm = normalized ? rCoord2D(width,height) : rCoord2D(1.,1.);

			for ( int k = 0; k<intervals.size() ; ++k )
			{
				const Interval<uint> interval = intervals[k].first;
				const qreal rightAngle = intervals[k].second.first;
				const qreal leftAngle = intervals[k].second.second;

				const qreal depthShift = 1./(normalized? static_cast<qreal>(interval.size()):1.0);
				const qreal angleShift = (rightAngle<leftAngle?leftAngle-rightAngle:leftAngle+(TWO_PI-rightAngle))/(qreal)(nbEdges-1);

				sumOfnbEdges += nbEdges*(interval.size()+1);

				depth = normalized ? -0.5 : -interval.size()/2.;
				for ( l=interval.min() ; l<=interval.max() ; ++l )
				{
					const iCoord2D &coord = billon.pithCoord(l);
					ofs = coord/norm - ofsStart;
					ofsRadius = rCoord2D( qMin(coord.x,width-coord.x), qMin(coord.y,height-coord.y) )/norm;
					angle = rightAngle<leftAngle?rightAngle:-TWO_PI+rightAngle;
					offsets.clear();
					while ( angle < leftAngle )
					{
						offsets.append( ofsRadius * rCoord2D( qCos(angle), qSin(angle) ) );
						angle += angleShift;
					}
					offsetsIterator = offsets.data();
					fullStream << ofs.x << ' ' << ofs.y << ' ' << depth << endl;
					for ( uint i=0 ; i<nbEdges-1 ; ++i )
					{
						stream << ofs.x+offsetsIterator->x << ' ' << ofs.y+offsetsIterator->y << ' ' << depth << endl;
						offsetsIterator++;
					}
					depth += depthShift;
				}

			}
			stream << endl;
			stream << sumOfnbEdges << endl;
			stream << *(fullStream.string());
		}

		void writeExportedVertex( QTextStream &stream, const Billon &billon, const QVector<rCoord2D> &vectVertex, const Interval<uint> &sliceInterval,const uint &resolutionCercle, const bool &normalized, const bool &displayBegEndFaces )
		{
			const int width = billon.n_cols;
			const int height = billon.n_rows;
			const qreal depthShift = 1./(normalized ? (qreal)sliceInterval.width() : 1.);
			int pos=0;

			const rCoord2D ofsStart = normalized ? rCoord2D(0.5,0.5) : rCoord2D(width/2,height/2);
			const rCoord2D norm = normalized ? rCoord2D( width, height ) : rCoord2D(1.,1.);
			qreal depth = normalized ? -0.5 : -(qreal)(sliceInterval.size()/2);
			uint i, k;

			stream << endl;
			stream << resolutionCercle*(sliceInterval.width()+1) << endl;
			for ( k=sliceInterval.min() ; k<=sliceInterval.max() ; ++k )
			{
				pos = k*resolutionCercle;
				for ( i=0 ; i<resolutionCercle ; i++ )
				{
					if ( displayBegEndFaces && i==0 && (k==sliceInterval.min() || k==sliceInterval.max()) )
						stream << 0 << ' ' << 0 << ' ' << depth << endl;
					else
						stream << ( vectVertex.at(pos).x /norm.x - ofsStart.x ) << ' ' << ( vectVertex.at(pos).y/norm.y - ofsStart.y ) << ' ' << depth << endl;
					pos++;
				}
				depth += depthShift;
			}
		}



		void computeAndWriteEdgesLinks( QTextStream &stream, const int &nbEdges, const int &nbSlices, bool displayBegEndFaces )
		{
			const int nbPoints = nbEdges*(nbSlices-1);
			const int nbPointsFoTube = 4*nbEdges;
			int i, base;

			stream << endl;
			stream << (displayBegEndFaces ? 2*(nbEdges+nbPoints-2) : 2*(nbPoints)) << endl;

			// La face de devant
			if ( displayBegEndFaces )
			{
				for ( i=1 ; i<nbEdges-1 ; ++i )
				{
					stream << 0 << ' ' << i << ' ' << i+1 << endl;
				}
			}

			// Les autres faces
			QVector<int> tubes;
			tubes.reserve( 4*nbEdges );
			for ( i=0 ; i<nbEdges-1 ; ++i )
			{
				tubes.append(i);
				tubes.append(i+nbEdges);
				tubes.append(i+nbEdges+1);
				tubes.append(i+1);
			}
			tubes.append(i);
			tubes.append(i+nbEdges);
			tubes.append(nbEdges);
			tubes.append(0);
			for ( base = 0 ; base<nbPoints ; base += nbEdges )
			{
				for ( i=0 ; i<nbPointsFoTube ; i+=4 )
				{
					stream << base+tubes[i] << ' ' << base+tubes[i+1] << ' ' << base+tubes[i+2] << endl;
					stream << base+tubes[i] << ' ' << base+tubes[i+2] << ' ' << base+tubes[i+3] << endl;
				}
			}

			// La face de derrière
			if ( displayBegEndFaces )
			{
				for ( i=nbPoints+1 ; i<nbSlices*nbEdges-1 ; ++i )
				{
					stream << nbPoints << ' ' << i+1 << ' ' << i << endl;
				}
			}
		}
	}
}
