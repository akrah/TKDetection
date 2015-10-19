#include "inc/v3dexport.h"

#include "inc/billon.h"
#include "inc/knotarea.h"
#include "inc/segmentation/tangentialgenerator.h"
#include "inc/segmentation/pithprofile.h"
#include "inc/segmentation/ellipseradiihistogram.h"
#include "inc/coordinate.h"
#include "inc/segmentation/pithprofile.h"
#include "inc/segmentation/ellipseradiihistogram.h"

#include <QXmlStreamWriter>
#include <QFile>
#include <QDebug>

namespace V3DExport
{
	namespace
	{
		void writeTag( QXmlStreamWriter &stream, const QString &name, const QString &value );
	}

	void init( QFile &file, QXmlStreamWriter &stream )
	{
		stream.setDevice( &file );
		stream.setAutoFormatting(true);
		stream.writeStartDocument("1.0");
		stream.writeDTD("<!DOCTYPE IMAGE>");

		stream.writeStartElement("image");
	}

	void close( QXmlStreamWriter &stream )
	{
		stream.writeEndElement();
		stream.writeEndDocument();
	}

	void appendTags( QXmlStreamWriter &stream, const Billon &billon )
	{
		stream.writeStartElement("tags");
		writeTag(stream,"width",QString::number(billon.n_cols));
		writeTag(stream,"height",QString::number(billon.n_rows));
		writeTag(stream,"depth",QString::number(billon.n_slices));
		writeTag(stream,"xspacing",QString::number(billon.voxelWidth()));
		writeTag(stream,"yspacing",QString::number(billon.voxelHeight()));
		writeTag(stream,"zspacing",QString::number(billon.voxelDepth()));
		writeTag(stream,"voxelwidth",QString::number(billon.voxelWidth()));
		writeTag(stream,"voxelheight",QString::number(billon.voxelHeight()));
		writeTag(stream,"voxeldepth",QString::number(billon.voxelDepth()));
		writeTag(stream,"minvalue",QString::number(billon.minValue()));
		writeTag(stream,"maxvalue",QString::number(billon.maxValue()));
		stream.writeEndElement();
	}

	void appendKnotArea( QXmlStreamWriter &stream, const uint &knotAreaIndex,
						 const Billon &tangentialBillon, const TangentialGenerator &tangentialGenerator,
						 const PithProfile &knotPithProfile, const EllipseRadiiHistogram &knotEllipseRadiiHistogram )
	{
		stream.writeStartElement("knotarea");
		stream.writeAttribute("id",QString::number(knotAreaIndex));
		stream.writeAttribute("nbtangentialslices",QString::number(tangentialBillon.n_slices));

		// Recherche des coordonnées min et max de la zone de nœud en x et y
		const QVector3D &origin = tangentialGenerator.origin();
		const QVector3D destLeftCoord = tangentialGenerator.rotate( QVector3D(0, 0, tangentialBillon.n_slices) );
		const QVector3D destRightCoord = tangentialGenerator.rotate( QVector3D(0, tangentialGenerator.width()-1, tangentialBillon.n_slices) );

		// position de la boite englobante du nœud
		const int minX = qMin(origin.x(),qMin(destLeftCoord.x(),destRightCoord.x()));
		const int minY = qMin(origin.y(),qMin(destLeftCoord.y(),destRightCoord.y()));
		const int minZ = tangentialGenerator.currentSliceInterval().min();
		stream.writeStartElement("boundingBoxPosition");
			stream.writeTextElement("x",QString::number(minX));
			stream.writeTextElement("y",QString::number(minY));
			stream.writeTextElement("z",QString::number(minZ));
		stream.writeEndElement();

		// dimension du le boite englobante du nœud
		stream.writeStartElement("boundingBoxSize");
			stream.writeTextElement("x",QString::number(qMax(origin.x(),qMax(destLeftCoord.x(),destRightCoord.x()))-minX));
			stream.writeTextElement("y",QString::number(qMax(origin.y(),qMax(destLeftCoord.y(),destRightCoord.y()))-minY));
			stream.writeTextElement("z",QString::number(tangentialGenerator.currentSliceInterval().max()-minZ));
		stream.writeEndElement();

		// dimension des voxels
		stream.writeStartElement("voxelDimensions");
			stream.writeTextElement("x",QString::number(tangentialBillon.voxelWidth()));
			stream.writeTextElement("y",QString::number(tangentialBillon.voxelHeight()));
			stream.writeTextElement("z",QString::number(tangentialBillon.voxelDepth()));
		stream.writeEndElement();

		// moelle du nœud
		appendTangentialPith( stream, tangentialBillon, tangentialGenerator, knotPithProfile, knotEllipseRadiiHistogram );

		stream.writeEndElement();
	}

	void appendBillonPith( QXmlStreamWriter &stream, const Billon &billon )
	{
		if ( !billon.pith().isEmpty() )
		{
			const Pith &pith = billon.pith();
			stream.writeStartElement("pith");
			for ( int k=0 ; k<pith.size() ; ++k )
			{
				stream.writeStartElement("coord");
				stream.writeTextElement("x",QString::number(pith[k].x));
				stream.writeTextElement("y",QString::number(pith[k].y));
				stream.writeTextElement("z",QString::number(k));
				stream.writeEndElement();
			}
			stream.writeEndElement();
		}
	}

	void appendTangentialPith( QXmlStreamWriter &stream, const Billon &tangentialBillon, const TangentialGenerator &tangentialGenerator,
							   const PithProfile &knotPithProfile, const EllipseRadiiHistogram &knotEllipseRadiiHistogram )
	{
		const Pith &pith = tangentialBillon.pith();
		if ( !pith.isEmpty() )
		{
			const qreal voxelRatio = tangentialBillon.voxelWidth()/tangentialBillon.voxelHeight();

			QVector3D initCoord, destCoord;

			stream.writeStartElement("pith");
			for ( int k=0 ; k<pith.size() ; ++k )
			{
				initCoord = QVector3D(pith[k].x, pith[k].y, k);
				destCoord = tangentialGenerator.rotate( QVector3D(pith[k].x, pith[k].y, k) );
				stream.writeStartElement("coord");
					stream.writeAttribute("ellipticityRate",QString::number(voxelRatio / knotPithProfile[k]));
					stream.writeAttribute("firstRadius",QString::number(knotEllipseRadiiHistogram[k]));
					stream.writeTextElement("x",QString::number(destCoord.x()));
					stream.writeTextElement("y",QString::number(destCoord.y()));
					stream.writeTextElement("z",QString::number(destCoord.z()));
				stream.writeEndElement();
			}
			stream.writeEndElement();
		}
	}

	namespace {
		void writeTag( QXmlStreamWriter &stream, const QString &name, const QString &value )
		{
			stream.writeStartElement("tag");
			stream.writeAttribute("name",name);
				stream.writeCharacters(value);
			stream.writeEndElement();
		}
	}
}
