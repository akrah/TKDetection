#include "inc/v3dreader.h"

#include "inc/billon.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QDataStream>

namespace V3DReader
{
	namespace
	{
		void readTags( QXmlStreamReader &stream, Billon &billon );
		void readComponents( QXmlStreamReader &stream, Billon &billon );
		void readPith( QXmlStreamReader &stream, Billon &billon );
	}

	Billon * read( const QString &fileName )
	{
		QFile file(fileName);

		if( !file.open(QIODevice::ReadOnly) )
		{
			qDebug() << QObject::tr("ERREUR : Impossible de lire le ficher XML %1.").arg(fileName);
			return 0;
		}

		Billon *billon = new Billon;

		QXmlStreamReader stream( &file );
		if ( stream.readNextStartElement() )
		{
			if ( stream.name() == "image" )
			{
				while ( stream.readNextStartElement() )
				{
					if ( stream.name() == "tags" ) readTags( stream, *billon );
					else if (stream.name() == "components")	readComponents( stream, *billon );
					else if (stream.name() == "pith") readPith( stream, *billon );
					else stream.skipCurrentElement();
				}
			}
		}

		file.close();

		return billon;
	}

	namespace
	{
		void readTags( QXmlStreamReader &stream, Billon &billon )
		{
			Q_ASSERT(stream.isStartElement() && stream.name() == "tags");

			uint width, height, depth;
			qreal voxelwidth, voxelheight, voxeldepth;
			width = height = depth = 0;
			voxelwidth = voxelheight = voxeldepth = 1.;

			while (stream.readNextStartElement())
			{
				if ( stream.name() == "tag" )
				{
					if ( stream.attributes().value("name") == "width" )
					{
						width = stream.readElementText().toInt();
					}
					else if (stream.attributes().value("name") == "height")
					{
						height = stream.readElementText().toInt();
					}
					else if (stream.attributes().value("name") == "depth")
					{
						depth = stream.readElementText().toInt();
					}
					else if (stream.attributes().value("name") == "voxelwidth")
					{
						voxelwidth = stream.readElementText().toDouble();
					}
					else if (stream.attributes().value("name") == "voxelheight")
					{
						voxelheight = stream.readElementText().toDouble();
					}
					else if (stream.attributes().value("name") == "voxeldepth")
					{
						voxeldepth = stream.readElementText().toDouble();
					}
					else stream.skipCurrentElement();
				}
				else stream.skipCurrentElement();
			}

			billon.resize(height,width,depth);
			billon.setVoxelSize(voxelwidth,voxelheight,voxeldepth);
		}

		void readComponents( QXmlStreamReader &stream, Billon &billon )
		{
			Q_ASSERT(stream.isStartElement() && stream.name() == "components");

			uint i, j, k;

			while (stream.readNextStartElement())
			{
				if ( stream.name() == "component" )
				{
					if ( !stream.attributes().value("valmax").isNull() ) billon.setMaxValue(stream.attributes().value("valmax").string()->toInt());
					if ( !stream.attributes().value("valmin").isNull() ) billon.setMinValue(stream.attributes().value("valmin").string()->toInt());

					while (stream.readNextStartElement())
					{
						if ( stream.name() == "binarydata" )
						{
							QDataStream dataStream(stream.device());
							qint16 val;
							for ( k=0; k<billon.n_slices; ++k )
							{
								Slice &slice = billon.slice(k);
								for ( j=0; j<billon.n_rows; ++j )
								{
									for ( i=0; i<billon.n_cols; ++i )
									{
										dataStream >> val;
										slice.at(j,i) = val;
									}
								}
							}
						}
						else stream.skipCurrentElement();
					}
				}
				else stream.skipCurrentElement();
			}
		}

		void readPith( QXmlStreamReader &stream, Billon &billon )
		{
			Q_ASSERT(stream.isStartElement() && stream.name() == "pith");

			billon.pith().resize(billon.n_slices);

			iCoord3D coord;
			while (stream.readNextStartElement())
			{
				if ( stream.name() == "coord" )
				{
					coord.z = -1;
					while (stream.readNextStartElement())
					{
						if ( stream.name() == "x" ) coord.x = stream.readElementText().toInt();
						else if ( stream.name() == "y" ) coord.y = stream.readElementText().toInt();
						else if ( stream.name() == "z" ) coord.z = stream.readElementText().toInt();
						else stream.skipCurrentElement();
					}
					if ( coord.z != -1 )
					{
						billon.pithCoord(coord.z).x = coord.x;
						billon.pithCoord(coord.z).y = coord.y;
					}
				}
				else stream.skipCurrentElement();
			}
		}
	}
}
