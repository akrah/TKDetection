#include "inc/datexport.h"

#include "inc/billon.h"
#include "inc/interval.h"

#include <QFile>
#include <QDataStream>
#include <QTextStream>

namespace DatExport
{
	void process( const Billon &billon, const Interval &slicesInterval, const Interval &intensityInterval, const QString &fileName, const int &resolution, const qreal &contrastFactor )
	{
		QFile file(fileName);
		if ( file.open(QIODevice::WriteOnly) )
		{
			const int firstSlice = slicesInterval.minValue();
			const int lastSlice = slicesInterval.maxValue();
			const int width = billon.n_cols;
			const int height = billon.n_rows;
			const int minValue = intensityInterval.minValue();
			const int maxValue = intensityInterval.maxValue();
			const int shift = resolution-1;
			const qreal fact = 255.0/(intensityInterval.size());

			QTextStream tStream(&file);
			tStream << "1 " << width/resolution << " " << height/resolution << " " << slicesInterval.count() << " 1 1 1 1";
			tStream.flush();

			QDataStream dStream(&file);
			for ( int k=firstSlice ; k<=lastSlice ; ++k )
			{
				const arma::Slice &slice = billon.slice(k);
				for ( int j=0 ; j<height-shift ; j+=resolution )
				{
					for ( int i=0 ; i<width-shift ; i+=resolution )
					{
						dStream << static_cast<qint8>( qBound(0., ((((qBound(minValue,arma::mean(arma::mean(slice.submat(j,i,j+shift,i+shift))),maxValue)-minValue)*fact)-128.)*contrastFactor)+128, 255.) );
					}
				}
			}

			file.close();
		}
	}
}
