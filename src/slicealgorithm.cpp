#include "inc/slicealgorithm.h"

#include "inc/billon.h"

#include <QPainter>

namespace SliceAlgorithm
{
	iCoord2D findNearestPointOfThePith( const Slice &slice, const iCoord2D &sliceCenter, const int &intensityThreshold )
	{
		// Find the pixel closest to the pith
		const int width = slice.n_cols;
		const int height = slice.n_rows;
		const int radiusMax = qMin( qMin(sliceCenter.x,width-sliceCenter.x), qMin(sliceCenter.y,height-sliceCenter.y) );

		iCoord2D position, circlePoint;
		bool edgeFind = false;
		int currentRadius, d;

		currentRadius = 1;
		// Using Andres circle algorithm
		while ( !edgeFind && currentRadius < radiusMax )
		{
			circlePoint.x = 0;
			circlePoint.y = currentRadius;
			d = currentRadius - 1;
			while ( circlePoint.y>=circlePoint.x && !edgeFind )
			{
				edgeFind = true;
				if ( slice.at( sliceCenter.y+circlePoint.y, sliceCenter.x+circlePoint.x ) > intensityThreshold )
				{
					position.x = sliceCenter.x+circlePoint.x;
					position.y = sliceCenter.y+circlePoint.y;
				}
				else if ( slice.at( sliceCenter.y+circlePoint.y, sliceCenter.x-circlePoint.x ) > intensityThreshold )
				{
					position.x = sliceCenter.x-circlePoint.x;
					position.y = sliceCenter.y+circlePoint.y;
				}
				else if ( slice.at( sliceCenter.y+circlePoint.x, sliceCenter.x+circlePoint.y ) > intensityThreshold )
				{
					position.x = sliceCenter.x+circlePoint.y;
					position.y = sliceCenter.y+circlePoint.x;
				}
				else if ( slice.at( sliceCenter.y+circlePoint.x, sliceCenter.x-circlePoint.y ) > intensityThreshold )
				{
					position.x = sliceCenter.x-circlePoint.y;
					position.y = sliceCenter.y+circlePoint.x;
				}
				else if ( slice.at( sliceCenter.y-circlePoint.y, sliceCenter.x+circlePoint.x ) > intensityThreshold )
				{
					position.x = sliceCenter.x+circlePoint.x;
					position.y = sliceCenter.y-circlePoint.y;
				}
				else if ( slice.at( sliceCenter.y-circlePoint.y, sliceCenter.x-circlePoint.x ) > intensityThreshold )
				{
					position.x = sliceCenter.x-circlePoint.x;
					position.y = sliceCenter.y-circlePoint.y;
				}
				else if ( slice.at( sliceCenter.y-circlePoint.x, sliceCenter.x+circlePoint.y ) > intensityThreshold )
				{
					position.x = sliceCenter.x+circlePoint.y;
					position.y = sliceCenter.y-circlePoint.x;
				}
				else if ( slice.at( sliceCenter.y-circlePoint.x, sliceCenter.x-circlePoint.y ) > intensityThreshold )
				{
					position.x = sliceCenter.x-circlePoint.y;
					position.y = sliceCenter.y-circlePoint.x;
				}
				else
				{
					edgeFind = false;
					if ( d >= 2*circlePoint.x )
					{
						d -= 2*circlePoint.x;
						circlePoint.x++;
					}
					else if ( d <= 2*(currentRadius-circlePoint.y) )
					{
						d += 2*circlePoint.y;
						circlePoint.y--;
					}
					else
					{
						d += 2*(circlePoint.y-circlePoint.x);
						circlePoint.y--;
						circlePoint.x++;
					}
				}
			}
			currentRadius++;
		}

		if ( edgeFind )
		{
			qDebug() << "Pixel le plus proche de la moelle : ( " << position.x << ", " << position.y << " )";
			return position;
		}
		else
		{
			qDebug() << "Aucun pixel et donc aucune composante connexe";
			return iCoord2D(-1,-1);
		}
	}

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
