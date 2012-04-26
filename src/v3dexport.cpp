#include "inc/v3dexport.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/slicesinterval.h"

#include <QDomDocument>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QXmlStreamWriter>

namespace V3DExport {

	namespace {
		void appendTags( const Billon &billon, QXmlStreamWriter &stream );
		void appendComponent( const Billon &billon, const SlicesInterval &interval, const int &index, const int &threshold, QXmlStreamWriter &stream );
		void appendMarrow( const Marrow &marrow, QXmlStreamWriter &stream );
		void writeXmlElementWithAttribute( QXmlStreamWriter &stream, const QString &elementName, const QString &attributeName, const QString &attributeValue, const QString &elementValue );
		void writeTag( QXmlStreamWriter &stream, const QString &name, const QString &value );
	}

	void process( const Billon &billon, const Marrow *marrow, const QString &fileName, const SlicesInterval &interval, const int &threshold ) {
		QFile file(fileName);

		if( !file.open(QIODevice::WriteOnly) ) {
			qDebug() << QObject::tr("ERREUR : Impossible de créer le ficher XML %1.").arg(fileName);
			return;
		}

		QXmlStreamWriter stream( &file );
		stream.setAutoFormatting(true);
		stream.writeStartDocument("1.0");
		stream.writeDTD("<!DOCTYPE IMAGE>");

			stream.writeStartElement("image");
				appendTags( billon, stream );
				appendComponent( billon, interval, 1, threshold, stream );
				if ( marrow != 0 ) appendMarrow( *marrow, stream );
			stream.writeEndElement();

		stream.writeEndDocument();

		file.close();
	}

	namespace {
		void appendTags( const Billon &billon, QXmlStreamWriter &stream ) {
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

		void appendComponent( const Billon &billon, const SlicesInterval &interval, const int &index, const int &threshold, QXmlStreamWriter &stream ) {
			int width = billon.n_cols;
			int height = billon.n_rows;
			int depth = interval.size();

			// components
			QByteArray data;
			QByteArray voxelValue;
			QDataStream voxelStream(&voxelValue,QIODevice::ReadWrite);

			stream.writeStartElement("components");
				stream.writeStartElement("component");
					stream.writeAttribute("id",QString::number(index));
					stream.writeAttribute("valmax",QString::number(billon.maxValue()));
					stream.writeAttribute("valmin",QString::number(billon.minValue()));

					//coord minimum
					stream.writeStartElement("coord");
						stream.writeAttribute("name","minimum");
						stream.writeStartElement("x");
							stream.writeCharacters("0");
						stream.writeEndElement();
						stream.writeStartElement("y");
							stream.writeCharacters("0");
						stream.writeEndElement();
						stream.writeStartElement("z");
							stream.writeCharacters(QString::number(interval.min()));
						stream.writeEndElement();
					stream.writeEndElement();

					//coord maximum
					stream.writeStartElement("coord");
						stream.writeAttribute("name","maximum");
						stream.writeStartElement("x");
							stream.writeCharacters(QString::number(width-1));
						stream.writeEndElement();
						stream.writeStartElement("y");
							stream.writeCharacters(QString::number(height-1));
						stream.writeEndElement();
						stream.writeStartElement("z");
							stream.writeCharacters(QString::number(interval.max()));
						stream.writeEndElement();
					stream.writeEndElement();

					//binarydata
					stream.writeStartElement("binarydata");
						stream.writeAttribute("encoding","16");
						stream.writeCharacters("");
						data.clear();
						data.reserve(width*height*depth*2);
						for ( int k=interval.min(); k<=interval.max(); k++ ) {
							const arma::Slice slice = billon.slice(k);
							for ( int j=0; j<height; j++ ) {
								for ( int i=0; i<width; i++ ) {
									voxelValue.clear();
									voxelStream << (slice.at(j,i) > threshold);
									voxelValue = voxelValue.right(2);
									data.append(voxelValue);
								}
							}
						}
						stream.device()->write(data.toHex());
					stream.writeEndElement();

				stream.writeEndElement();
			stream.writeEndElement();
		}

		void appendMarrow( const Marrow &marrow, QXmlStreamWriter &stream ) {
			stream.writeStartElement("marrow");
			for ( int k=0, sliceIdx = marrow.interval().min() ; k<marrow.size() ; ++k, ++sliceIdx ) {
				const iCoord2D &coord = marrow[k];
				stream.writeStartElement("coord");
					stream.writeTextElement("x",QString::number(coord.x));
					stream.writeTextElement("y",QString::number(coord.y));
					stream.writeTextElement("z",QString::number(sliceIdx));
				stream.writeEndElement();
			}
			stream.writeEndElement();
		}

		void writeXmlElementWithAttribute( QXmlStreamWriter &stream, const QString &elementName, const QString &attributeName, const QString &attributeValue, const QString &elementValue ) {
			stream.writeStartElement(elementName);
				stream.writeAttribute(attributeName,attributeValue);
				stream.writeCharacters(elementValue);
			stream.writeEndElement();
		}

		void writeTag( QXmlStreamWriter &stream, const QString &name, const QString &value ) {
			writeXmlElementWithAttribute(stream,"tag","name",name,value);
		}
	}
}
