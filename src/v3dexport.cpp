#include "inc/v3dexport.h"

#include "inc/billon.h"

#include <QXmlStreamWriter>
#include <QFile>
#include <QDebug>

namespace V3DExport
{
	namespace
	{
		void appendTags( QXmlStreamWriter &stream, const Billon &billon, const Interval<uint> &sliceInterval );
		void appendComponent( QXmlStreamWriter &stream, const Billon &billon, const Interval<uint> &sliceInterval, const int &index, const int &threshold );
		void appendPith(QXmlStreamWriter &stream, const Billon &billon , const Interval<uint> &sliceInterval );
		void writeTag( QXmlStreamWriter &stream, const QString &name, const QString &value );
	}

	void process( QFile &file, const Billon &billon, const Interval<uint> &sliceInterval, const int &threshold )
	{
		QXmlStreamWriter stream( &file );
		stream.setAutoFormatting(true);
		stream.writeStartDocument("1.0");
		stream.writeDTD("<!DOCTYPE IMAGE>");

			stream.writeStartElement("image");
				appendTags( stream, billon, sliceInterval );
				appendComponent( stream, billon, sliceInterval, 1, threshold );
				appendPith( stream, billon, sliceInterval );
			stream.writeEndElement();

		stream.writeEndDocument();
	}

	namespace
	{
		void appendTags( QXmlStreamWriter &stream, const Billon &billon, const Interval<uint> &sliceInterval )
		{
			stream.writeStartElement("tags");
				writeTag(stream,"width",QString::number(billon.n_cols));
				writeTag(stream,"height",QString::number(billon.n_rows));
				writeTag(stream,"depth",QString::number(sliceInterval.width()+1));
				writeTag(stream,"xspacing",QString::number(billon.voxelWidth()));
				writeTag(stream,"yspacing",QString::number(billon.voxelHeight()));
				writeTag(stream,"zspacing",QString::number(billon.voxelDepth()));
				writeTag(stream,"voxelwidth",QString::number(billon.voxelWidth()));
				writeTag(stream,"voxelheight",QString::number(billon.voxelHeight()));
				writeTag(stream,"voxeldepth",QString::number(billon.voxelDepth()));
			stream.writeEndElement();
		}

		void appendComponent(QXmlStreamWriter &stream, const Billon &billon, const Interval<uint> &sliceInterval, const int &index, const int &threshold )
		{
			const uint &width = billon.n_cols;
			const uint &height = billon.n_rows;

			uint i, j, k;

			stream.writeStartElement("components");
				stream.writeStartElement("component");
					stream.writeAttribute("id",QString::number(index));
					stream.writeAttribute("valmax",QString::number(billon.maxValue()));
					stream.writeAttribute("valmin",QString::number(billon.minValue()));

					//coord minimum
					stream.writeStartElement("coord");
						stream.writeAttribute("name","minimum");
						stream.writeTextElement("x",QString::number(0));
						stream.writeTextElement("y",QString::number(0));
						stream.writeTextElement("z",QString::number(sliceInterval.min()));
					stream.writeEndElement();

					//coord maximum
					stream.writeStartElement("coord");
						stream.writeAttribute("name","maximum");
						stream.writeTextElement("x",QString::number(width-1));
						stream.writeTextElement("y",QString::number(height-1));
						stream.writeTextElement("z",QString::number(sliceInterval.max()));
					stream.writeEndElement();

					//binarydata
					stream.writeStartElement("binarydata");
						stream.writeAttribute("encoding","16");
						stream.writeCharacters("");
						QDataStream voxelStream(stream.device());
						for ( k=sliceInterval.min(); k<=sliceInterval.max(); ++k )
						{
							const Slice &slice = billon.slice(k);
							for ( j=0; j<height; ++j )
							{
								for ( i=0; i<width; ++i )
								{
									voxelStream << (qint16)(slice.at(j,i));
								}
							}
						}
						voxelStream.unsetDevice();
					stream.writeEndElement();

				stream.writeEndElement();
			stream.writeEndElement();
		}

		void appendPith( QXmlStreamWriter &stream, const Billon &billon, const Interval<uint> &sliceInterval )
		{
			if ( !billon.pith().isEmpty() )
			{
				const Pith &pith = billon.pith();
				stream.writeStartElement("pith");
				for ( uint k=sliceInterval.min() ; k<=sliceInterval.max() ; ++k )
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

		void writeTag( QXmlStreamWriter &stream, const QString &name, const QString &value )
		{
			stream.writeStartElement("tag");
			stream.writeAttribute("name",name);
				stream.writeCharacters(value);
			stream.writeEndElement();
		}
	}
}
