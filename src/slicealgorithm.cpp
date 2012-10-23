#include "inc/slicealgorithm.h"

#include "inc/billon.h"

#include <QPainter>

namespace SliceAlgorithm
{
	void draw( const Slice &slice, QImage &image, const int &intensityThreshold )
	{
		QPainter painter(&image);
		painter.setPen(QColor(255,255,255,127));

		uint i, j;
		for ( j=0 ; j<slice.n_rows ; ++j )
		{
			for ( i=0 ; i<slice.n_cols ; ++i )
			{
				if ( slice.at(j,i) > intensityThreshold ) painter.drawPoint(i,j);
			}
		}
	}

	void writeInPgm3D( const Slice &slice, QDataStream &stream )
	{
		uint i, j;
		for ( j=0 ; j<slice.n_rows ; ++j )
		{
			for ( i=0 ; i<slice.n_cols ; ++i )
			{
				stream << static_cast<qint16>(slice.at(j,i));
			}
		}
	}

	void writeInSDP( const Slice &slice, QTextStream &stream, const uint &sliceNum, const int &intensityThreshold )
	{
		uint i, j;
		for ( j=0 ; j<slice.n_rows ; ++j )
		{
			for ( i=0 ; i<slice.n_cols ; ++i )
			{
				if ( slice(j,i) > intensityThreshold )
				{
					stream << i << ' ' << j << ' ' << sliceNum  << endl;
				}
			}
		}
	}
}
