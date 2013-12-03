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
				if ( slice( sliceCenter.y+circlePoint.y, sliceCenter.x+circlePoint.x ) > intensityThreshold )
				{
					position.x = sliceCenter.x+circlePoint.x;
					position.y = sliceCenter.y+circlePoint.y;
				}
				else if ( slice( sliceCenter.y+circlePoint.y, sliceCenter.x-circlePoint.x ) > intensityThreshold )
				{
					position.x = sliceCenter.x-circlePoint.x;
					position.y = sliceCenter.y+circlePoint.y;
				}
				else if ( slice( sliceCenter.y+circlePoint.x, sliceCenter.x+circlePoint.y ) > intensityThreshold )
				{
					position.x = sliceCenter.x+circlePoint.y;
					position.y = sliceCenter.y+circlePoint.x;
				}
				else if ( slice( sliceCenter.y+circlePoint.x, sliceCenter.x-circlePoint.y ) > intensityThreshold )
				{
					position.x = sliceCenter.x-circlePoint.y;
					position.y = sliceCenter.y+circlePoint.x;
				}
				else if ( slice( sliceCenter.y-circlePoint.y, sliceCenter.x+circlePoint.x ) > intensityThreshold )
				{
					position.x = sliceCenter.x+circlePoint.x;
					position.y = sliceCenter.y-circlePoint.y;
				}
				else if ( slice( sliceCenter.y-circlePoint.y, sliceCenter.x-circlePoint.x ) > intensityThreshold )
				{
					position.x = sliceCenter.x-circlePoint.x;
					position.y = sliceCenter.y-circlePoint.y;
				}
				else if ( slice( sliceCenter.y-circlePoint.x, sliceCenter.x+circlePoint.y ) > intensityThreshold )
				{
					position.x = sliceCenter.x+circlePoint.y;
					position.y = sliceCenter.y-circlePoint.x;
				}
				else if ( slice( sliceCenter.y-circlePoint.x, sliceCenter.x-circlePoint.y ) > intensityThreshold )
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
			//qDebug() << "Pixel le plus proche de la moelle : ( " << position.x << ", " << position.y << " )";
			return position;
		}
		else
		{
			qDebug() << "Aucun pixel et donc aucune composante connexe";
			return iCoord2D(-1,-1);
		}
	}

	qreal restrictedAreaRadius( const Slice &slice, const uiCoord2D &pithCoord, const uint &nbPolygonPoints, const int &intensityThreshold )
	{
		Q_ASSERT_X( nbPolygonPoints>0 , "BillonTpl<T>::getRestrictedAreaMeansRadius", "nbPolygonPoints arguments equals to 0 => division by zero" );

		const int width = slice.n_cols;
		const int height = slice.n_rows;
		const qreal angleIncrement = TWO_PI/static_cast<qreal>(nbPolygonPoints);

		rCoord2D center, edge;
		rVec2D direction;
		qreal orientation, radius, currentNorm;

		radius = width;
		center.x = pithCoord.x;
		center.y = pithCoord.y;
		orientation = width;
		while (orientation < TWO_PI)
		{
			orientation += angleIncrement;
			direction.x = qCos(orientation);
			direction.y = qSin(orientation);
			edge = center + direction*20;
			while ( edge.x>0 && edge.y>0 && edge.x<width && edge.y<height && slice(edge.y,edge.x) > intensityThreshold )
			{
				edge += direction;
			}
			currentNorm = rVec2D(edge-center).norm();
			if ( currentNorm < radius ) radius = currentNorm;
		}
		return radius;
	}

	void draw( QPainter &painter, const Slice &slice, const uiCoord2D &pithCoord, const int &intensityThreshold, const TKD::ProjectionType &view )
	{
		painter.save();
		painter.setPen(QColor(255,255,255,127));

		const uint width = painter.window().width();
		const uint height = painter.window().height();

		const qreal angularIncrement = TWO_PI/(qreal)(width);

		uint i, j, x, y;

		if ( view == TKD::Z_PROJECTION )
		{
			for ( j=0 ; j<height ; ++j )
			{
				for ( i=0 ; i<width ; ++i )
				{
					if ( slice(j,i) > intensityThreshold ) painter.drawPoint(i,j);
				}
			}
		}
		else if ( view == TKD::POLAR_PROJECTION )
		{
			for ( j=0 ; j<height ; ++j )
			{
				for ( i=0 ; i<width ; ++i )
				{
					x = pithCoord.x + j * qCos(i*angularIncrement);
					y = pithCoord.y + j * qSin(i*angularIncrement);
					if ( slice(y,x) > intensityThreshold ) painter.drawPoint(i,j);
				}
			}
		}

		painter.restore();
	}

	void writeInPgm3D( const Slice &slice, QDataStream &stream )
	{
		uint i, j;
		for ( j=0 ; j<slice.n_rows ; ++j )
		{
			for ( i=0 ; i<slice.n_cols ; ++i )
			{
				stream << static_cast<qint16>(slice(j,i));
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
