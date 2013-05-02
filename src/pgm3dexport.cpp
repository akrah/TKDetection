#include "inc/pgm3dexport.h"

#include "inc/billon.h"
#include "inc/interval.h"
#include "inc/globalfunctions.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

namespace Pgm3dExport
{
	void process( const Billon &billon, const QString &fileName, const qreal &contrastFactor, const int &componentNumber )
	{
		QFile file(fileName);

		if( !file.open(QIODevice::WriteOnly) )
		{
			qDebug() << QObject::tr("ERREUR : Impossible de créer le ficher PGM %1.").arg(fileName);
			return;
		}

		const bool oneComponent = (componentNumber > 0);
		const uint width = billon.n_cols;
		const uint height = billon.n_rows;
		const uint depth = billon.n_slices;
		const int minValue = billon.minValue();
		const int maxValue = oneComponent?minValue+1:billon.maxValue();
		const int midValue = (maxValue-minValue)/2;

		QTextStream stream(&file);

		stream << "P3D" << endl;
		stream << "#!VoxelDim " << billon.voxelWidth() << ' ' << billon.voxelHeight() << ' ' << billon.voxelDepth() << endl;
		stream << width << " " << height << " " << depth << endl;
		stream << (oneComponent?1:(maxValue-minValue)) << endl;

		QDataStream dstream(&file);
		uint i, j, k;
		if ( !oneComponent )
		{
			for ( k=0 ; k<depth ; ++k )
			{
				const Slice &slice = billon.slice(k);
				for ( j=0 ; j<height ; ++j )
				{
					for ( i=0 ; i<width ; ++i )
					{
						//dstream << (qint16)(slice.at(j,i)-minValue);
						dstream << (qint16)qBound(minValue,static_cast<int>((((slice.at(j,i)-minValue)-midValue)*contrastFactor)+midValue),maxValue);
					}
				}
			}
		}
		else
		{
			for ( k=0 ; k<depth ; ++k )
			{
				const Slice &slice = billon.slice(k);
				for ( j=0 ; j<height ; ++j )
				{
					for ( i=0 ; i<width ; ++i )
					{
						dstream << (qint16)(slice.at(j,i) == componentNumber);
					}
				}
			}
		}

		file.close();
	}


	void processImage( QTextStream &stream, const Billon &billon, const Interval<int> &slicesInterval, 
			   const Interval<int> &intensityInterval, const int &resolution, const qreal &contrastFactor )
	{
		const int firstSlice = slicesInterval.min();
		const int lastSlice = slicesInterval.max();
		const int width = billon.n_cols;
		const int height = billon.n_rows;
		const int minValue = intensityInterval.min();
		const int shift = resolution-1;
		const qreal fact = 255.0/(intensityInterval.size());

		stream << "P3D" << endl;
		stream << width/resolution << " " << height/resolution << " " <<  slicesInterval.width()+1 << endl;
		stream << "255" << endl;

		QDataStream dStream(stream.device());
		int value;
		for ( int k=firstSlice ; k<=lastSlice ; ++k )
		{
			const Slice &slice = billon.slice(k);
			for ( int j=0 ; j<height-shift ; j+=resolution )
			{
				for ( int i=0 ; i<width-shift ; i+=resolution )
				{
					value = TKD::restrictedValue(arma::mean(arma::mean(slice.submat(j,i,j+shift,i+shift))),intensityInterval);
					dStream << static_cast<qint8>( qBound(0., ((((value-minValue)*fact)-128.)*contrastFactor)+128, 255.) );
				}
			}
		}
	}


}
