#include "inc/ofsexport.h"

#include "def/def_coordinate.h"
#include "inc/coordinate.h"
#include "inc/billon.h"
#include "inc/billonalgorithms.h"
#include "inc/pith.h"

#include <QTextStream>
#include <QFile>
#include <QDebug>

namespace OfsExport {

	namespace {
	// Calcul les coordonnées des sommets du maillage de la moelle
	void computeAllEdges( const Billon &billon, const Pith &pith, const Interval<int> &interval,
							const int &nbEdges, const int &radius, QTextStream &stream, bool normalized = true );

	void computeSectorEdges( const Billon &billon, const Pith &pith, const Interval<int> &interval,
							 const int &nbEdges, const qreal &rightAngle, const qreal &leftAngle,
							 QTextStream &stream , bool normalized = true);

	void computeAllSectorInAllIntervalsEdges( const Billon &billon, const Pith &pith,
												const QVector<QPair<Interval<int>, QPair<qreal, qreal> > > &intervals,
												const int &nbEdges, QTextStream &stream,	bool normalized = true );


	//Rajout BK: Affiche les coordonnées des sommets pour l'export OFS (utile iuniquement pour le maillage de la zone réduite)
	void displayExportedVertex( const Billon &billon, const QVector<rCoord2D> &vectVertex, const Interval<uint> &interval, const uint &resolutionCercle, QTextStream &stream, bool normalized = true,	bool displayBegEndFaces= true );

	// Calcul les faces du maillages de la moelle
	void computeEgesLinks( const int &nbEdges, const int &nbSlices, QTextStream &stream, bool displayBegEndFaces=true );
	}

	void process( const Billon &billon, const Pith &pith, const Interval<int> &interval,
				const QString &fileName, const int &nbEdgesPerSlice,
				const int &radiusOfTubes,	bool normalized ) {
	if ( pith.interval().containsClosed(interval) ) {
		QFile file(fileName);
		if ( file.open(QIODevice::WriteOnly) ) {
		QTextStream stream(&file);
		stream << "OFS MHD" << endl;
		computeAllEdges( billon, pith, interval, nbEdgesPerSlice, radiusOfTubes, stream, normalized );
		computeEgesLinks( nbEdgesPerSlice, interval.width()+1, stream );
		file.close();
		}
	}

	else {
		qDebug() << QObject::tr("Saving not possible since the mesh is outside the SlicesInterval");
	}
	}

	void processOnSector( const Billon &billon, const Pith &pith, const Interval<int> &interval,
						const QString &fileName, const qreal &rightAngle, const qreal &leftAngle,
						const int &nbEdgesPerSlice,	bool normalized ) {
	if ( pith.interval().containsClosed(interval) ) {
		QFile file(fileName);
		if ( file.open(QIODevice::WriteOnly) ) {
		QTextStream stream(&file);
		stream << "OFS MHD" << endl;
		computeSectorEdges( billon, pith, interval, nbEdgesPerSlice, rightAngle, leftAngle, stream, normalized );
		computeEgesLinks( nbEdgesPerSlice, interval.width()+1, stream );
		file.close();
		}
	}
	else {
		qDebug() << QObject::tr("Saving not possible since the mesh is outside the SlicesInterval");
	}
	}

	void processOnAllSectorInAllIntervals( const Billon &billon, const Pith &pith,
										 const QVector<QPair<Interval<int>, QPair<qreal, qreal> > > &intervals,
										 const QString &fileName, const int &nbEdgesPerSlice,	bool normalized ) {
	QFile file(fileName);
	if ( file.open(QIODevice::WriteOnly) ) {
		QTextStream stream(&file);
		stream << "OFS MHD" << endl;
		computeAllSectorInAllIntervalsEdges( billon, pith, intervals, nbEdgesPerSlice, stream, normalized );

		QString fullStream;
		int sumNbLinks = 0;
		int currentBase = 0;
		const int nbPointsFoTube = 4*nbEdgesPerSlice;
		for ( int k=0 ; k<intervals.size() ; ++k ) {
		const int nbSlices = intervals[k].first.size();
		const int nbPoints = nbEdgesPerSlice*(nbSlices-1);

		sumNbLinks += 2*(nbEdgesPerSlice+nbPoints-2);

		// La face de devant
		for ( int i=1 ; i<nbEdgesPerSlice-1 ; ++i ) {
			fullStream.append( QString("%1 %2 %3%4").arg(currentBase).arg(currentBase+i).arg(currentBase+i+1).arg('\n') );
		}
		// Les autres faces
		QList<qint32> tubes;
		int i;
		for ( i=0 ; i<nbEdgesPerSlice-1 ; ++i ) {
			tubes.append(currentBase+i);
			tubes.append(currentBase+i+nbEdgesPerSlice);
			tubes.append(currentBase+i+nbEdgesPerSlice+1);
			tubes.append(currentBase+i+1);
		}
		tubes.append(currentBase+i);
		tubes.append(currentBase+i+nbEdgesPerSlice);
		tubes.append(currentBase+nbEdgesPerSlice);
		tubes.append(currentBase);
		for ( int base = 0 ; base<nbPoints ; base += nbEdgesPerSlice ) {
			for ( int i=0 ; i<nbPointsFoTube ; i+=4 ) {
			fullStream.append( QString("%1 %2 %3%4").arg(base+tubes[i]).arg(base+tubes[i+1]).arg(base+tubes[i+2]).arg('\n') );
			fullStream.append( QString("%1 %2 %3%4").arg(base+tubes[i]).arg(base+tubes[i+2]).arg(base+tubes[i+3]).arg('\n') );
			}
		}
		// La face de derrière
		const int lastBase = currentBase+nbPoints;
		for ( int i=lastBase+1 ; i<nbSlices*nbEdgesPerSlice-1 ; ++i ) {
			fullStream.append( QString("%1 %2 %3%4").arg(lastBase).arg(i+1).arg(i).arg('\n') );
		}

		currentBase += nbSlices*nbEdgesPerSlice;
		}
		stream << endl;
		stream << sumNbLinks << endl;
		stream << fullStream;

		file.close();
	}
	}

void processRestrictedMesh( const Billon &billon, const Pith &pith, const Interval<uint> & sliceInterval, const QString &fileName,
							const uint resolutionCercle, const int seuilContour, const bool normalized, const bool displayBegEndFaces )
{
	QVector<rCoord2D> vectVertex = BillonAlgorithms::getRestrictedAreaVertex( billon, pith, sliceInterval, resolutionCercle,seuilContour );
	if ( !vectVertex.isEmpty() ) {
		QFile file(fileName);
		if ( file.open(QIODevice::WriteOnly) )
		{
			QTextStream stream(&file);
			stream << "OFS MHD" << endl;
			displayExportedVertex(billon, vectVertex, sliceInterval, resolutionCercle, stream, normalized, displayBegEndFaces);
			computeEgesLinks( resolutionCercle, sliceInterval.width()+1, stream, displayBegEndFaces );
			file.close();
		}
	}
}


	namespace {


	void computeAllEdges( const Billon &billon, const Pith &pith, const Interval<int> &interval,
							const int &nbEdges, const int &radius, QTextStream &stream, bool normalized ) {
		const int width = billon.n_cols;
		const int height = billon.n_rows;
		const int nbSlices = interval.size();
		const int firstPith = interval.min() - pith.interval().min();
		const int lastPith = qMin(firstPith + nbSlices,pith.size());
		qreal depth = firstPith;
		const qreal depthShift = 1./(normalized? (qreal)nbSlices: 1.0);
		int i,k;

		QVector<rCoord2D> offsets;
		const qreal angleShift = TWO_PI/(qreal)nbEdges;
		const qreal ofsXRadius = radius/(normalized? (qreal)width: 1.0);
		const qreal ofsYRadius = radius/(normalized? (qreal)height: 1.0);
		qreal angle = 0;
		while ( angle < TWO_PI ) {
	offsets.append( rCoord2D( qCos(angle)*ofsXRadius, qSin(angle)*ofsYRadius ) );
	angle += angleShift;
		}

		stream << endl;
		stream << nbEdges*(lastPith-firstPith+1) << endl;
		rCoord2D *offsetsIterator = 0;
		for ( k=firstPith ; k<=lastPith ; ++k ) {
	const iCoord2D &coord = pith[k];
	const qreal xOfs = coord.x/(normalized? (qreal)width:1.0) - 0.5;
	const qreal yOfs = coord.y/(normalized? (qreal)height:1.0) - 0.5;
	offsetsIterator = offsets.data();
	for ( i=0 ; i<nbEdges ; ++i ) {
		stream << xOfs+offsetsIterator->x << ' ' << yOfs+offsetsIterator->y << ' ' << depth << endl;
		offsetsIterator++;
	}
	depth += depthShift;
		}		}




	void computeSectorEdges( const Billon &billon, const Pith &pith, const Interval<int> &interval, const int &nbEdges, const qreal &rightAngle, const qreal &leftAngle, QTextStream &stream, bool normalized ) {
		const int width = billon.n_cols;
		const int height = billon.n_rows;
		const int nbSlices = interval.size();
		const int firstPith = interval.min() - pith.interval().min();
		const int lastPith = qMin(firstPith + nbSlices,pith.size());
		const qreal depthShift = 1./(normalized? (qreal)nbSlices:1.0);
		const qreal angleShift = (rightAngle<leftAngle?leftAngle-rightAngle:leftAngle+(TWO_PI-rightAngle))/(qreal)(nbEdges-1);
		int i,k;

		stream << endl;
		stream << nbEdges*(lastPith-firstPith+1) << endl;

		QVector<rCoord2D> offsets;
		rCoord2D *offsetsIterator = 0;
		qreal depth = -0.5;
		qreal angle;
		for ( k=firstPith ; k<=lastPith ; ++k ) {
	const iCoord2D &coord = pith[k];
	const qreal xOfs = coord.x/(normalized? (qreal)width:1.0) - 0.5;
	const qreal yOfs = coord.y/(normalized? (qreal)height:1.0) - 0.5;
	const qreal ofsXRadius = qMin(coord.x,width-coord.x)/(normalized? (qreal)width:1.0);
	const qreal ofsYRadius = qMin(coord.y,width-coord.y)/(normalized? (qreal)height:1.0);
	angle = rightAngle<leftAngle?rightAngle:-TWO_PI+rightAngle;
	offsets.clear();
	while ( angle < leftAngle ) {
		offsets.append( rCoord2D( qCos(angle)*ofsXRadius, qSin(angle)*ofsYRadius ) );
		angle += angleShift;
	}
	offsetsIterator = offsets.data();
	stream << xOfs << ' ' << yOfs << ' ' << depth << endl;
	for ( i=0 ; i<nbEdges-1 ; ++i ) {
		stream << xOfs+offsetsIterator->x << ' ' << yOfs+offsetsIterator->y << ' ' << depth << endl;
		offsetsIterator++;
	}
	depth += depthShift;
		}
	}



	void computeAllSectorInAllIntervalsEdges( const Billon &billon, const Pith &pith,
												const QVector< QPair< Interval<int>, QPair<qreal,qreal> > > &intervals,
												const int &nbEdges, QTextStream &stream, bool normalized ) {
		const int width = billon.n_cols;
		const int height = billon.n_rows;

		QString fullStream;
		int sumOfnbEdges = 0;
		QVector<rCoord2D> offsets;
		rCoord2D *offsetsIterator;
		qreal depth, angle;
		for ( int k = 0; k<intervals.size() ; ++k ) {

	const Interval<int> interval = intervals[k].first;
	const int nbSlices = interval.size();
	const qreal rightAngle = intervals[k].second.first;
	const qreal leftAngle = intervals[k].second.second;

	const int firstPith = interval.min() - pith.interval().min();
	const int lastPith = qMin(firstPith + nbSlices,pith.size());
	const qreal depthShift = 1./(normalized? (qreal)nbSlices:1.0);
	const qreal angleShift = (rightAngle<leftAngle?leftAngle-rightAngle:leftAngle+(TWO_PI-rightAngle))/(qreal)(nbEdges-1);


	sumOfnbEdges += nbEdges*(lastPith-firstPith+1);

	depth = -0.5;
	for (int l=firstPith ; l<=lastPith ; ++l ) {
		const iCoord2D &coord = pith[l];
		const qreal xOfs = coord.x/(normalized? (qreal)width: 1.0) - 0.5;
		const qreal yOfs = coord.y/(normalized? (qreal)height: 1.0) - 0.5;
		const qreal ofsXRadius = qMin(coord.x,width-coord.x)/(normalized ? (qreal)width: 1.0);
		const qreal ofsYRadius = qMin(coord.y,width-coord.y)/(normalized ? (qreal)height: 1.0);
		angle = rightAngle<leftAngle?rightAngle:-TWO_PI+rightAngle;
		offsets.clear();
		while ( angle < leftAngle ) {
		offsets.append( rCoord2D( qCos(angle)*ofsXRadius, qSin(angle)*ofsYRadius ) );
		angle += angleShift;
		}
		offsetsIterator = offsets.data();
		fullStream.append( QString("%1 %2 %3%4").arg(xOfs).arg(yOfs).arg(depth).arg('\n') );
		for (int i=0 ; i<nbEdges-1 ; ++i ) {
		fullStream.append( QString("%1 %2 %3%4").arg(xOfs+offsetsIterator->x).arg(yOfs+offsetsIterator->y).arg(depth).arg('\n') );
		offsetsIterator++;
		}
		depth += depthShift;
	}

		}
		stream << endl;
		stream << sumOfnbEdges << endl;
		stream << fullStream;
	}

void displayExportedVertex( const Billon &billon, const QVector<rCoord2D> &vectVertex, const Interval<uint> &sliceInterval,const uint &resolutionCercle, QTextStream &stream, bool normalized,	bool displayBegEndFaces	)
{
	const int width = billon.n_cols;
	const int height = billon.n_rows;
	qreal depth = sliceInterval.min();
	const qreal depthShift = 1./(normalized ? (qreal)sliceInterval.width() : 1.0);
	int pos=0;
	stream << endl;
	stream << resolutionCercle*(sliceInterval.width()+1) << endl;
	uint i, k;
	for ( k=sliceInterval.min() ; k<=sliceInterval.max() ; ++k )
	{
		pos = k*resolutionCercle;
		for ( i=0 ; i<resolutionCercle ; i++ )
		{
			if ( (k==sliceInterval.min()) && i==0 && displayBegEndFaces )
				stream << (normalized? 0:(width/2.0)) << ' ' << (normalized? 0:(height/2.0)) << ' ' << -0.5 << endl;
			else if ( (k==sliceInterval.max()) && i==0 && displayBegEndFaces )
				stream << (normalized? 0:(width/2.0)) << ' ' << (normalized? 0:(height/2.0)) << ' ' << sliceInterval.min() << endl;
			else
				stream << ((vectVertex.at(pos).x /(normalized?static_cast<qreal>(width): 1.0)) -0.5)<< ' ' << ((vectVertex.at(pos).y/(normalized? static_cast<qreal>(height):1.0)) -0.5)<< ' ' << sliceInterval.min() << endl;
			pos++;
		}
		depth += depthShift;
	}
}



	void computeEgesLinks( const int &nbEdges, const int &nbSlices, QTextStream &stream, bool displayBegEndFaces ) {
		const int nbPoints = nbEdges*(nbSlices-1);
		const int nbPointsFoTube = 4*nbEdges;

		stream << endl;
		if( displayBegEndFaces){
		stream << 2*(nbEdges+nbPoints-2) << endl;
		}else{
		stream << 2*(nbPoints) << endl;
		}
		// La face de devant
		if( displayBegEndFaces){
		for ( int i=1 ; i<nbEdges-1 ; ++i ) {
			stream << 0 << ' ' << i << ' ' << i+1 << endl;
		}
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
		if( displayBegEndFaces){
		const int lasBase = nbPoints;
		for ( int i=lasBase+1 ; i<nbSlices*nbEdges-1 ; ++i ) {
			stream << lasBase << ' ' << i+1 << ' ' << i << endl;
		}
		}
	}
	}
}
