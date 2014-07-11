#include "inc/v3dexport.h"

#include "inc/billon.h"

#include <QXmlStreamWriter>
#include <QFile>
#include <QDebug>

namespace V3DExport
{
	namespace
	{
		void writeTag( QXmlStreamWriter &stream, const QString &name, const QString &value );
	}

	void process( QFile &file, const Billon &billon )
	{
		QXmlStreamWriter stream;
		init(file,stream);
		appendTags( stream, billon );
		appendPith( stream, billon, 0 );
		startComponents(stream);
		appendComponent( stream, billon, 0, 1 );
		endComponents(stream);
		close(stream);
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
		stream.writeEndElement();
	}

	void startComponents( QXmlStreamWriter &stream )
	{
		stream.writeStartElement("components");
	}

	void appendComponent( QXmlStreamWriter &stream, const Billon &billon, const uint &firstSlicePos, const int &index )
	{
		const uint &width = billon.n_cols;
		const uint &height = billon.n_rows;
		const uint &depth = billon.n_slices;

		uint i, j, k;

		stream.writeStartElement("component");
		stream.writeAttribute("id",QString::number(index));
		stream.writeAttribute("valmax",QString::number(billon.maxValue()));
		stream.writeAttribute("valmin",QString::number(billon.minValue()));

		//coord minimum
		stream.writeStartElement("coord");
		stream.writeAttribute("name","minimum");
		stream.writeTextElement("x",QString::number(0));
		stream.writeTextElement("y",QString::number(0));
		stream.writeTextElement("z",QString::number(firstSlicePos));
		stream.writeEndElement();

		//coord maximum
		stream.writeStartElement("coord");
		stream.writeAttribute("name","maximum");
		stream.writeTextElement("x",QString::number(width-1));
		stream.writeTextElement("y",QString::number(height-1));
		stream.writeTextElement("z",QString::number(firstSlicePos+billon.n_slices-1));
		stream.writeEndElement();

		//binarydata
		stream.writeStartElement("binarydata");
		stream.writeAttribute("encoding","16");
		stream.writeCharacters("");
		QDataStream voxelStream(stream.device());
		for ( k=0; k<depth; ++k )
		{
			const Slice &slice = billon.slice(k);
			for ( j=0; j<height; ++j )
			{
				for ( i=0; i<width; ++i )
				{
					voxelStream << (qint16)(slice(j,i));
				}
			}
		}
		stream.writeEndElement();

		stream.writeEndElement();
	}

	void endComponents( QXmlStreamWriter &stream )
	{
		stream.writeEndElement();
	}

	void appendPith( QXmlStreamWriter &stream, const Billon &billon, const uint &firstSlicePos )
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
				stream.writeTextElement("z",QString::number(k+firstSlicePos));
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
