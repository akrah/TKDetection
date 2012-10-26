#include "inc/datexport.h"

#include "inc/billon.h"
#include "inc/interval.h"
#include "inc/globalfunctions.h"

#include <QFile>
#include <QDataStream>
#include <QTextStream>

namespace DatExport
{
	void process( QTextStream &stream, const Billon &billon, const Interval<int> &slicesInterval, const Interval<int> &intensityInterval, const int &resolution, const qreal &contrastFactor )
	{
		const int firstSlice = slicesInterval.min();
		const int lastSlice = slicesInterval.max();
		const int width = billon.n_cols;
		const int height = billon.n_rows;
		const int minValue = intensityInterval.min();
		const int shift = resolution-1;
		const qreal fact = 255.0/(intensityInterval.size());

		stream << "1 " << width/resolution << " " << height/resolution << " " << slicesInterval.width()+1 << " 1 1 1 1";

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
