#include "inc/ofsexport.h"

#include "inc/coordinate.h"
#include "inc/billon.h"
#include "inc/billonalgorithms.h"

#include <QTextStream>
#include <QFile>
#include <QDebug>

namespace OfsExport
{
	namespace
	{
		// Calcul les coordonnées des sommets du maillage de la moelle
		void computeAndWriteSectorEdges( QTextStream &stream, const Billon &billon, const uint &nbEdges, const uint &radius,
										 const Interval<uint> &sliceInterval, const Interval<qreal> &angleInterval, const bool &normalized );
		void computeAndWriteAllSectorInAllIntervalsEdges(QTextStream &stream, const Billon &billon, const uint &nbEdges, const uint &radius,
														  const QVector< Interval<uint> > &sliceIntervals, const QVector<QVector<Interval<qreal> > > &angleIntervals, const bool &normalized );

		// Calcul les faces du maillages de la moelle
		void computeAndWriteEdgesLinks( QTextStream &stream, const int &nbEdges, const int &nbSlices, bool displayBegEndFaces );

		//Rajout BK: Affiche les coordonnées des sommets pour l'export OFS (utile iuniquement pour le maillage de la zone réduite)
		void writeExportedVertex( QTextStream &stream, const Billon &billon, const QVector<rCoord2D> &vectVertex, const Interval<uint> &interval,
								  const uint &resolutionCercle, const bool &normalized, const bool &displayBegEndFaces );
	}

	void writeHeader( QTextStream &stream )
	{
		stream << "OFS MHD" << endl;
	}

	void processOnPith( QTextStream &stream, const Billon &billon, const Interval<uint> &sliceInterval, const int &nbEdgesPerSlice, const int &radiusOfTubes, const bool &normalized )
	{
		computeAndWriteSectorEdges( stream, billon, nbEdgesPerSlice, radiusOfTubes, sliceInterval, Interval<qreal>(0,TWO_PI), normalized );
		computeAndWriteEdgesLinks( stream, nbEdgesPerSlice, sliceInterval.width()+1, true );
	}

	void processOnSector( QTextStream &stream, const Billon &billon, const int &nbEdgesPerSlice, const uint &radius, const Interval<uint> &sliceInterval,
						  const Interval<qreal> &angleInterval, const bool &normalized )
	{
		computeAndWriteSectorEdges( stream, billon, nbEdgesPerSlice, radius, sliceInterval, angleInterval, normalized );
		computeAndWriteEdgesLinks( stream, nbEdgesPerSlice, sliceInterval.width()+1, true );
	}

	void processOnAllSectorInAllIntervals( QTextStream &stream, const Billon &billon, const uint &nbEdgesPerSlice, const uint &radius,
										   const QVector< Interval<uint> > &sliceIntervals, const QVector< QVector< Interval<qreal> > > &angleIntervals, const bool &normalized )
	{
		computeAndWriteAllSectorInAllIntervalsEdges( stream, billon, nbEdgesPerSlice, radius, sliceIntervals, angleIntervals, normalized );

		QTextStream fullStream;
		uint i, base;
		int k;

		int sumNbLinks = 0;
		int currentBase = 0;
		const uint nbPointsFoTube = 4*nbEdgesPerSlice;

		for ( k=0 ; k<sliceIntervals.size() ; ++k )
		{
			const int nbSlices = sliceIntervals[k].size()+1;
			const uint nbPoints = nbEdgesPerSlice*nbSlices;

			sumNbLinks += 2*(nbEdgesPerSlice+nbPoints);

			// La face de devant
			for ( i=1 ; i<nbEdgesPerSlice-1 ; ++i )
			{
				fullStream << currentBase << ' ' << currentBase+i << ' ' << currentBase+i+1 << endl;
			}
			// Les autres faces
			QList<qint32> tubes;
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
			for ( base = 0 ; base<nbPoints ; base += nbEdgesPerSlice )
			{
				for ( i=0 ; i<nbPointsFoTube ; i+=4 ) {
					fullStream << base+tubes[i] << ' ' << base+tubes[i+1] << ' ' << base+tubes[i+2] << endl;
					fullStream << base+tubes[i] << ' ' << base+tubes[i+2] << ' ' << base+tubes[i+3] << endl;
				}
			}
			// La face de derrière
			const int lastBase = currentBase+nbPoints;
			for ( i=lastBase+1 ; i<nbSlices*nbEdgesPerSlice-1 ; ++i )
			{
				fullStream << lastBase << ' ' << i+1 << ' ' << i << endl;
			}

			currentBase += nbSlices*nbEdgesPerSlice;
		}
		stream << endl;
		stream << sumNbLinks << endl;
		stream << *(fullStream.string());
	}

	void processOnRestrictedMesh( QTextStream &stream, const Billon &billon, const Interval<uint> & sliceInterval, const QVector<rCoord2D> &vectVertex,
								  const uint & circleResolution, const bool &normalized, const bool &displayBegEndFaces )
	{
		writeExportedVertex(stream, billon, vectVertex, sliceInterval, circleResolution, normalized, displayBegEndFaces);
		computeAndWriteEdgesLinks( stream, circleResolution, sliceInterval.width()+1, displayBegEndFaces );
	}


	namespace
	{
		// Les angles doivent être compris entre 0 et 360
		void computeAndWriteSectorEdges( QTextStream &stream, const Billon &billon, const uint &nbEdges, const uint &radius,
										 const Interval<uint> &sliceInterval, const Interval<qreal> &angleInterval, const bool &normalized )
		{
			const int width = billon.n_cols;
			const int height = billon.n_rows;
			const qreal nbSlices = sliceInterval.width()+1;
			const qreal depthShift = 1./( normalized ? qreal(nbSlices-1) : 1. );
			const qreal angleShift = (angleInterval.min()<angleInterval.max()?angleInterval.max()-angleInterval.min():angleInterval.max()+TWO_PI-angleInterval.min())/(qreal)(nbEdges);
			const rCoord2D norm = normalized ? rCoord2D(width,height) : rCoord2D(1.,1.);
			const rCoord2D ofsRadius = rCoord2D( radius, radius )/norm;
			const rCoord2D ofsStart(normalized?-0.5:0,normalized?-0.5:0);

			QVector<rCoord2D> offsets;
			rCoord2D *offsetsIterator, ofs;
			uint i, k;

			qreal angle = angleInterval.min()<angleInterval.max()?angleInterval.min():-TWO_PI+angleInterval.min();
			offsets.reserve(nbEdges);
			while ( angle < angleInterval.max() )
			{
				offsets.append( ofsRadius * rCoord2D( qCos(angle), qSin(angle) ) );
				angle += angleShift;
			}

			stream << endl;
			stream << nbEdges*nbSlices << endl;

			qreal depth = normalized ? -0.5+depthShift*sliceInterval.min() : sliceInterval.min();
			for ( k=sliceInterval.min() ; k<=sliceInterval.max() ; ++k )
			{
				ofs = billon.pithCoord(k)/norm + ofsStart;
				offsetsIterator = offsets.data();
				//if ( !qFuzzyCompare( angleInterval.max()-angleInterval.min(), TWO_PI ) ) stream << ofs.x << ' ' << ofs.y << ' ' << depth << endl;
				for ( i=0 ; i<nbEdges ; ++i )
				{
					stream << ofs.x+offsetsIterator->x << ' ' << ofs.y+offsetsIterator->y << ' ' << depth << endl;
					offsetsIterator++;
				}
				depth += depthShift;
			}
		}



		void computeAndWriteAllSectorInAllIntervalsEdges( QTextStream &stream, const Billon &billon, const uint &nbEdges, const uint &radius,
														  const QVector< Interval<uint> > &sliceIntervals, const QVector< QVector< Interval<qreal> > > &angleIntervals, const bool &normalized )
		{
			Q_ASSERT_X(sliceIntervals.size() == angleIntervals.size(), "OfsExport::computeAndWriteAllSectorInAllIntervalsEdges", QObject::tr("Pas de correspondance entre intervalles de coupes et intervalles d'angles !").toStdString().c_str());

			const int width = billon.n_cols;
			const int height = billon.n_rows;
			const qreal depthShift = 1./(normalized?(billon.n_slices):1.0);
			const rCoord2D norm = normalized ? rCoord2D(width,height) : rCoord2D(1.,1.);
			const rCoord2D ofsRadius = rCoord2D( radius, radius )/norm;
			const rCoord2D ofsStart(normalized?-0.5:0,normalized?-0.5:0);

			QTextStream fullStream;
			QVector<rCoord2D> offsets;
			rCoord2D *offsetsIterator, ofs;
			qreal depth, angle;
			uint k, n;
			int i, j;

			int sumOfnbEdges = 0;
			for ( i=0 ; i<sliceIntervals.size() ; ++i )
			{
				const Interval<uint> &sliceInterval = sliceIntervals[i];
				const qreal nbSlices = sliceInterval.width()+1;

				const QVector< Interval<qreal> > &angleIntervalsOfCurrentSliceInterval = angleIntervals[i];
				for ( j=0 ; j<angleIntervalsOfCurrentSliceInterval.size() ; j++ )
				{
					const Interval<qreal> &angleInterval = angleIntervalsOfCurrentSliceInterval[j];
					const qreal angleShift = (angleInterval.min()<angleInterval.max()?angleInterval.max()-angleInterval.min():angleInterval.max()+(TWO_PI-angleInterval.min()))/(qreal)(nbEdges);

					sumOfnbEdges += nbEdges*nbSlices;

					angle = angleInterval.min()<angleInterval.max()?angleInterval.min():-TWO_PI+angleInterval.min();
					offsets.clear();
					offsets.reserve(nbEdges);
					while ( angle < angleInterval.max() )
					{
						offsets.append( ofsRadius * rCoord2D( qCos(angle), qSin(angle) ) );
						angle += angleShift;
					}

					depth = normalized ? -0.5+depthShift*sliceInterval.min() : sliceInterval.min();
					for ( k=sliceInterval.min() ; k<=sliceInterval.max() ; ++k )
					{
						ofs = billon.pithCoord(k)/norm - ofsStart;
						offsetsIterator = offsets.data();
						if ( !qFuzzyCompare( sliceInterval.max() - sliceInterval.min(), TWO_PI ) ) fullStream << ofs.x << ' ' << ofs.y << ' ' << depth << endl;
						for ( n=0 ; n<nbEdges-1 ; ++n )
						{
							fullStream << ofs.x+offsetsIterator->x << ' ' << ofs.y+offsetsIterator->y << ' ' << depth << endl;
							offsetsIterator++;
						}
						depth += depthShift;
					}
				}

			}

			stream << endl;
			stream << sumOfnbEdges << endl;
			stream << *(fullStream.string());
		}

		void writeExportedVertex( QTextStream &stream, const Billon &billon, const QVector<rCoord2D> &vectVertex, const Interval<uint> &sliceInterval,
								  const uint &resolutionCercle, const bool &normalized, const bool &displayBegEndFaces )
		{
			const int width = billon.n_cols;
			const int height = billon.n_rows;
			const qreal nbSlices = sliceInterval.width()+1;
			const qreal depthShift = 1./(normalized ? (qreal)nbSlices : 1.);
			int pos=0;

			const rCoord2D ofsStart = normalized ? rCoord2D(0.5,0.5) : rCoord2D(0,0);
			const rCoord2D norm = normalized ? rCoord2D( width, height ) : rCoord2D(1.,1.);
			qreal depth = normalized ? -0.5 : 0.;
			uint i, k;

			stream << endl;
			stream << resolutionCercle*nbSlices << endl;
			pos = sliceInterval.min()*resolutionCercle;
			for ( k=sliceInterval.min() ; k<=sliceInterval.max() ; ++k )
			{
				for ( i=0 ; i<resolutionCercle ; i++ )
				{
					if ( displayBegEndFaces && !i && (k==sliceInterval.min() || k==sliceInterval.max()) )
						stream << 0 << ' ' << 0 << ' ' << depth << endl;
					else
						stream << ( vectVertex[pos].x /norm.x - ofsStart.x ) << ' ' << ( vectVertex[pos].y/norm.y - ofsStart.y ) << ' ' << depth << endl;
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
