#include "inc/contourslice.h"

#include "inc/billon.h"
#include "inc/slicealgorithm.h"
#include "inc/curvaturehistogram.h"

#include <QProcess>
#include <QTemporaryFile>
#include <QPainter>

ContourSlice::ContourSlice() : _maxConcavityPointsIndex(-1), _minConcavityPointsIndex(-1)
{
}

ContourSlice::~ContourSlice()
{
}

/**********************************
 * Public getters
 **********************************/

const Contour &ContourSlice::contour() const
{
	return _contour;
}

const ContourDistancesHistogram &ContourSlice::contourDistancesHistogram() const
{
	return _contourDistancesHistogram;
}

const CurvatureHistogram &ContourSlice::curvatureHistogram() const
{
	return _curvatureHistogram;
}

const iCoord2D &ContourSlice::maxConcavityPoint() const
{
	return _maxConcavityPointsIndex != -1 ? _contour[_maxConcavityPointsIndex] : invalidICoord2D;
}

const iCoord2D &ContourSlice::minConcavityPoint() const
{
	return _minConcavityPointsIndex != -1 ? _contour[_minConcavityPointsIndex] : invalidICoord2D;
}

const int &ContourSlice::maxConcavityPointIndex() const
{
	return _maxConcavityPointsIndex;
}

void ContourSlice::setMaxConcavityPointIndex( int maxConcavityIndex )
{
	_maxConcavityPointsIndex = maxConcavityIndex;
}

const int &ContourSlice::minConcavityPointIndex() const
{
	return _minConcavityPointsIndex;
}

void ContourSlice::setMinConcavityPointIndex( int minConcavityIndex )
{
	_minConcavityPointsIndex = minConcavityIndex;
}

const rCoord2D &ContourSlice::maxSupportPoint() const
{
	return _maxSupportPoint;
}

void ContourSlice::setMaxSupportPoint( const rCoord2D &maxSupportPoint )
{
	_maxSupportPoint = maxSupportPoint;
}

const rCoord2D &ContourSlice::minSupportPoint() const
{
	return _minSupportPoint;
}

void ContourSlice::setMinSupportPoint( const rCoord2D &minSupportPoint )
{
	_minSupportPoint = minSupportPoint;
}

const uiCoord2D &ContourSlice::sliceCenter() const
{
	return _sliceCenter;
}

/**********************************
 * Public setters
 **********************************/

void ContourSlice::compute( Slice &resultSlice, const Slice &initialSlice, const uiCoord2D &sliceCenter, const int &intensityThreshold,
							const int &smoothingRadius, const int &curvatureWidth, const qreal &curvatureThreshold, const iCoord2D &startPoint )
{
	computeStart( initialSlice, sliceCenter, intensityThreshold, smoothingRadius, curvatureWidth, curvatureThreshold, startPoint );
	computeEnd( resultSlice, initialSlice, intensityThreshold );
}

void ContourSlice::computeStart( const Slice &initialSlice, const uiCoord2D &sliceCenter, const int &intensityThreshold,
				   const int &smoothingRadius, const int &curvatureWidth, const qreal &curvatureThreshold, const iCoord2D &startPoint )
{
	_contour.compute( initialSlice, sliceCenter, intensityThreshold, startPoint );
	_originalContour = _contour;
	_contour.smooth(smoothingRadius);

	_contourDistancesHistogram.construct( _contour, sliceCenter );
	_curvatureHistogram.construct( _contour, curvatureWidth  );

	_sliceCenter = sliceCenter;

	computeConcavityPoints( curvatureThreshold );
	computeSupportPoints( 10 );
}

void ContourSlice::computeEnd( Slice &resultSlice, const Slice &initialSlice, const int &intensityThreshold )
{
	computeContourPolygons();
	updateSlice( initialSlice, resultSlice, intensityThreshold );
}

void ContourSlice::draw( QPainter &painter, const int &cursorPosition, const TKD::ViewType &viewType ) const
{
	const int nbContourPoints = _contour.size();
	const uint width = painter.window().width();
	const uint height = painter.window().height();
	const qreal angularFactor = width/TWO_PI;

	if ( nbContourPoints > 0 )
	{
		painter.save();
		_contour.draw(painter,cursorPosition,_sliceCenter,viewType);

		int i, j, x, y, i2, j2, x2, y2;

		// Dessins des points de concavité
		qreal a, b;
		if ( maxConcavityPointIndex() != -1 )
		{
			// Dessin du point de concavité gauche
			const iCoord2D &maxConcavityPoint = this->maxConcavityPoint();

			painter.setPen(Qt::green);

			if ( viewType == TKD::Z_VIEW )
			{
				painter.drawEllipse(maxConcavityPoint.x-2,maxConcavityPoint.y-2,4,4);
			}
			else if ( viewType == TKD::CARTESIAN_VIEW )
			{
				i = maxConcavityPoint.x - _sliceCenter.x;
				j = maxConcavityPoint.y - _sliceCenter.y;
				y = qSqrt(qPow(i,2) + qPow(j,2));
				x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
				painter.drawEllipse(x-2,y-2,4,4);
			}

			// Dessins de la droite de coupe issue du point de concavité gauche
			const rCoord2D &maxSupportPoint = this->maxSupportPoint();
			if ( maxSupportPoint.x != -1 || maxSupportPoint.y != -1 )
			{
				painter.setPen(Qt::gray);
				a = ( maxConcavityPoint.y - maxSupportPoint.y ) / static_cast<qreal>( maxConcavityPoint.x - maxSupportPoint.x );
				b = ( maxConcavityPoint.y * maxSupportPoint.x - maxConcavityPoint.x * maxSupportPoint.y ) / static_cast<qreal>( maxSupportPoint.x - maxConcavityPoint.x );
				if ( qFuzzyCompare(maxConcavityPoint.x, maxSupportPoint.x) )
				{
					if ( maxSupportPoint.y < maxConcavityPoint.y ) painter.drawLine(maxConcavityPoint.x, maxConcavityPoint.y, maxConcavityPoint.x, height );
					else painter.drawLine(maxConcavityPoint.x, maxConcavityPoint.y, maxConcavityPoint.x, 0 );
				}
				else if ( maxSupportPoint.x < maxConcavityPoint.x )
				{
					if ( viewType == TKD::Z_VIEW )
					{
						painter.drawLine(maxConcavityPoint.x, maxConcavityPoint.y, width, a * width + b );
					}
					else if ( viewType == TKD::CARTESIAN_VIEW )
					{
						i = maxConcavityPoint.x - _sliceCenter.x;
						j = maxConcavityPoint.y - _sliceCenter.y;
						y = qSqrt(qPow(i,2) + qPow(j,2));
						x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
						i2 = width - _sliceCenter.x;
						j2 = a * width + b - _sliceCenter.y;
						y2 = qSqrt(qPow(i2,2) + qPow(j2,2));
						x2 = 2. * qAtan( j2 / (qreal)(i2 + y2) ) * angularFactor;
						painter.drawLine( x, y, x2, y2 );
					}
				}
				else
				{
					if ( viewType == TKD::Z_VIEW )
					{
						painter.drawLine(maxConcavityPoint.x, maxConcavityPoint.y, 0., b );
					}
					else if ( viewType == TKD::CARTESIAN_VIEW )
					{
						i = maxConcavityPoint.x - _sliceCenter.x;
						j = maxConcavityPoint.y - _sliceCenter.y;
						y = qSqrt(qPow(i,2) + qPow(j,2));
						x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
						i2 = -_sliceCenter.x;
						j2 = b - _sliceCenter.y;
						y2 = qSqrt(qPow(i2,2) + qPow(j2,2));
						x2 = 2. * qAtan( j2 / (qreal)(i2 + y2) ) * angularFactor;
						painter.drawLine( x, y, x2, y2 );
					}
				}
			}
		}
		if ( minConcavityPointIndex() != -1 )
		{
			// Dessin du point de concavité droit
			const iCoord2D &minConcavityPoint = this->minConcavityPoint();

			painter.setPen(Qt::green);
			if ( viewType == TKD::Z_VIEW )
			{
				painter.drawEllipse(minConcavityPoint.x-2,minConcavityPoint.y-2,4,4);
			}
			else if ( viewType == TKD::CARTESIAN_VIEW )
			{
				i = minConcavityPoint.x - _sliceCenter.x;
				j = minConcavityPoint.y - _sliceCenter.y;
				y = qSqrt(qPow(i,2) + qPow(j,2));
				x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
				painter.drawEllipse(x-2,y-2,4,4);
			}

			// Dessins de la droite de coupe issue du point de concavité droit
			const rCoord2D &minSupportPoint = this->minSupportPoint();
			if ( minSupportPoint.x != -1 || minSupportPoint.y != -1 )
			{
				painter.setPen(Qt::gray);
				a = ( minConcavityPoint.y - minSupportPoint.y ) / static_cast<qreal>( minConcavityPoint.x - minSupportPoint.x );
				b = ( minConcavityPoint.y * minSupportPoint.x - minConcavityPoint.x * minSupportPoint.y ) / static_cast<qreal>( minSupportPoint.x - minConcavityPoint.x );
				if ( qFuzzyCompare(minConcavityPoint.x, minSupportPoint.x) )
				{
					if ( viewType == TKD::Z_VIEW )
					{
						if ( minSupportPoint.y < minConcavityPoint.y ) painter.drawLine(minConcavityPoint.x, minConcavityPoint.y, minConcavityPoint.x, height );
						else painter.drawLine(minConcavityPoint.x, minConcavityPoint.y, minConcavityPoint.x, 0 );
					}
				}
				else if ( minSupportPoint.x < minConcavityPoint.x )
				{
					if ( viewType == TKD::Z_VIEW )
					{
						painter.drawLine(minConcavityPoint.x, minConcavityPoint.y, width, a * width + b );
					}
					else if ( viewType == TKD::CARTESIAN_VIEW )
					{
						i = minConcavityPoint.x - _sliceCenter.x;
						j = minConcavityPoint.y - _sliceCenter.y;
						y = qSqrt(qPow(i,2) + qPow(j,2));
						x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
						i2 = width - _sliceCenter.x;
						j2 = a * width + b - _sliceCenter.y;
						y2 = qSqrt(qPow(i2,2) + qPow(j2,2));
						x2 = 2. * qAtan( j2 / (qreal)(i2 + y2) ) * angularFactor;
						painter.drawLine( x, y, x2, y2 );
					}
				}
				else
				{
					if ( viewType == TKD::Z_VIEW )
					{
						painter.drawLine(minConcavityPoint.x, minConcavityPoint.y, 0., b );
					}
					else if ( viewType == TKD::CARTESIAN_VIEW )
					{
						i = minConcavityPoint.x - _sliceCenter.x;
						j = minConcavityPoint.y - _sliceCenter.y;
						y = qSqrt(qPow(i,2) + qPow(j,2));
						x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
						i2 = -_sliceCenter.x;
						j2 = b - _sliceCenter.y;
						y2 = qSqrt(qPow(i2,2) + qPow(j2,2));
						x2 = 2. * qAtan( j2 / (qreal)(i2 + y2) ) * angularFactor;
						painter.drawLine( x, y, x2, y2 );
					}
				}
			}
		}

		// Dessin du point de contour initial
		painter.setPen(Qt::red);
		if ( viewType == TKD::Z_VIEW )
		{
			painter.drawEllipse(_contour[0].x-2,_contour[0].y-2,4,4);
		}
		else if ( viewType == TKD::CARTESIAN_VIEW )
		{
			i = _contour[0].x - _sliceCenter.x;
			j = _contour[0].y - _sliceCenter.y;
			y = qSqrt(qPow(i,2) + qPow(j,2));
			x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
			painter.drawEllipse(x-2,y-2,4,4);
		}

		painter.restore();
	}
}

/**********************************
 * Private setters
 **********************************/

void ContourSlice::computeConcavityPoints( const qreal &curvatureThreshold )
{
	_maxConcavityPointsIndex = _minConcavityPointsIndex = -1;

	int nbPoints, index, indexToCompare;

	nbPoints = _contour.size();
	indexToCompare = nbPoints*0.4;

	if ( nbPoints > 0 && _contourDistancesHistogram.size() == nbPoints && _curvatureHistogram.size() == nbPoints )
	{
		// Min concavity point
		index = 1;
		while ( (index < indexToCompare) && (_curvatureHistogram[index] > curvatureThreshold || _contour[0].euclideanDistance(_contour[index]) < 10) )
		{
			index++;
		}
		if ( index < indexToCompare ) _minConcavityPointsIndex = index;

		// Max concavity point
		index = nbPoints-2;
		indexToCompare = nbPoints-indexToCompare;
		while ( (index > indexToCompare) && (_curvatureHistogram[index] > curvatureThreshold || _contour[0].euclideanDistance(_contour[index]) < 10) )
		{
			index--;
		}
		if ( index > indexToCompare ) _maxConcavityPointsIndex = index;
	}
}

void ContourSlice::computeSupportPoints( const int &meansMaskSize )
{
	_maxSupportPoint = _minSupportPoint = rCoord2D(-1,-1);

	int index, counter;

	// Support du point de concavité min
	index = _minConcavityPointsIndex;
	if ( index != -1 )
	{
		_minSupportPoint.x = _minSupportPoint.y = 0.;
		counter = 0;
		while ( index >= qMax(_minConcavityPointsIndex-meansMaskSize,0) )
		{
			_minSupportPoint.x += _contour[index].x;
			_minSupportPoint.y += _contour[index].y;
			--index;
			++counter;
		}
		_minSupportPoint /= counter;
	}

	// Support du point de concavité max
	index = _maxConcavityPointsIndex;
	if ( index != -1 )
	{
		const int nbPoints = _contour.size();
		_maxSupportPoint.x = _maxSupportPoint.y = 0.;
		counter = 0;
		while ( index < qMin(_maxConcavityPointsIndex+meansMaskSize,nbPoints) )
		{
			_maxSupportPoint.x += _contour[index].x;
			_maxSupportPoint.y += _contour[index].y;
			++index;
			++counter;
		}
		_maxSupportPoint /= counter;
	}
}

void ContourSlice::computeContourPolygons()
{
	_contourPolygonBottom.clear();
	_contourPolygonTop.clear();

	const int nbContourPoints = _contour.size();
	if ( !nbContourPoints ) return;

	int i;

	// S'il y a deux points de concavité
	if ( _maxConcavityPointsIndex != -1 && _minConcavityPointsIndex != -1 )
	{
		const iCoord2D &maxConcavityPoint = this->maxConcavityPoint();
		const iCoord2D &minConcavityPoint = this->minConcavityPoint();

		// Création du polygone qui servira à tester l'appartenance d'un pixel en dessous de la droite des deux points de concavité au noeud.
		// Ce polygone est constitué :
		//     - des points compris entre le premier point du contour lissé et le point de concavité gauche
		//     - du point de concavité gauche
		//     - du point de concavité droit
		//     - des points compris entre le point de concavité droit et le dernier points du contour lissé.
		for ( i=0 ; i<=_minConcavityPointsIndex ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
		_contourPolygonBottom << QPoint(minConcavityPoint.x,minConcavityPoint.y)
							 << QPoint(maxConcavityPoint.x,maxConcavityPoint.y);
		for ( i=_maxConcavityPointsIndex ; i<nbContourPoints ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
		_contourPolygonBottom << QPoint(_contour[0].x,_contour[0].y);

		// Création du polygone qui servira à tester l'appartenance d'un pixel au dessus de la droite des deux points de concavité au noeud.
		// Ce polygone est constitué :
		//     - du point le plus proche du point de concavité
		//     - des points du contour situés entre le point le point de concavité gauche et le point de concavité droit
		//     - du point de concavité droit
		_contourPolygonTop << QPoint(minConcavityPoint.x,minConcavityPoint.y);
		for ( i=_minConcavityPointsIndex ; i<=_maxConcavityPointsIndex ; ++i ) _contourPolygonTop << QPoint(_contour[i].x,_contour[i].y);
		_contourPolygonTop << QPoint(maxConcavityPoint.x,maxConcavityPoint.y)
						  << QPoint(minConcavityPoint.x,minConcavityPoint.y);
	}
	else
	{
		if ( _maxConcavityPointsIndex != -1 )
		{
			const iCoord2D &maxConcavityPoint = this->maxConcavityPoint();
			const int minConcavityPointsIndexComputed = nbContourPoints-_maxConcavityPointsIndex;
			const iCoord2D &minConcavityPoint = _contour[minConcavityPointsIndexComputed];

			// Création du polygone qui servira à tester l'appartenance d'un pixel en dessous de la droite des deux points de concavité au noeud.
			// Ce polygone est constitué :
			//     - des points compris entre le premier point du contour lissé et le point de concavité gauche
			//     - du point de concavité gauche
			//     - du pointde concavité droit calculé
			//     - des points compris entre le point de concavité droit principal calculé et le dernier points du contour lissé.
			for ( i=0 ; i<=minConcavityPointsIndexComputed ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
			_contourPolygonBottom << QPoint(minConcavityPoint.x,minConcavityPoint.y)
								  << QPoint(maxConcavityPoint.x,maxConcavityPoint.y);
			for ( i=_maxConcavityPointsIndex ; i<nbContourPoints ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
			_contourPolygonBottom << QPoint(_contour[0].x,_contour[0].y);
		}
		else if ( _minConcavityPointsIndex != -1 )
		{
			const iCoord2D &minConcavityPoint = this->minConcavityPoint();
			const int maxConcavityPointsIndexComputed = nbContourPoints-_minConcavityPointsIndex;
			const iCoord2D &maxConcavityPoint = _contour[maxConcavityPointsIndexComputed];

			// Création du polygone qui servira à tester l'appartenance d'un pixel en dessous de la droite des deux points de concavité au noeud.
			// Ce polygone est constitué :
			//     - des points compris entre le premier point du contour lissé et le point de concavité principal gauche
			//     - du point de concavité gauche
			//     - du point de concavité droit calculé
			//     - des points compris entre le point de concavité droit principal calculé et le dernier points du contour lissé.
			for ( i=0 ; i<=_minConcavityPointsIndex ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
			_contourPolygonBottom << QPoint(minConcavityPoint.x,minConcavityPoint.y)
								  << QPoint(maxConcavityPoint.x,maxConcavityPoint.y);
			for ( i=maxConcavityPointsIndexComputed ; i<nbContourPoints ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
			_contourPolygonBottom << QPoint(_contour[0].x,_contour[0].y);
		}

		// Création du polygone qui servira à tester l'appartenance d'un pixel au noeud.
		// Ce polygone est constitué des pixels du contour lissé
		for ( i=0 ; i<nbContourPoints ; ++i )
		{
			_contourPolygonTop << QPoint(_contour[i].x,_contour[i].y);
		}
		_contourPolygonTop << QPoint(_contour[0].x,_contour[0].y);
	}
}

void ContourSlice::updateSlice( const Slice &initialSlice, Slice &resultSlice, const int &intensityThreshold )
{
	const int width = initialSlice.n_cols;
	const int height = initialSlice.n_rows;
	const int nbPointsContour = _contour.size();

	resultSlice.fill(0);

	const iCoord2D &maxConcavityPoint = this->maxConcavityPoint();
	const iCoord2D &minConcavityPoint = this->minConcavityPoint();
	const rCoord2D &maxSupportPoint = this->maxSupportPoint();
	const rCoord2D &minSupportPoint = this->minSupportPoint();

	const bool hasMaxConcavity = _maxConcavityPointsIndex != -1;
	const bool hasMinConcavity = _minConcavityPointsIndex != -1;

	int i,j;

	// S'il y a deux points de concavité
	if ( hasMaxConcavity && hasMinConcavity )
	{
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( initialSlice.at(j,i) > intensityThreshold &&
					 ( _contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill) ||
					   (
						   maxSupportPoint.vectorProduct( maxConcavityPoint, iCoord2D(i,j) ) <= 0 &&
						   minSupportPoint.vectorProduct( minConcavityPoint, iCoord2D(i,j) ) >= 0 &&
						   maxConcavityPoint.vectorProduct( minConcavityPoint, iCoord2D(i,j) ) >= 0 &&
						   _contourPolygonTop.containsPoint(QPoint(i,j),Qt::OddEvenFill)
					   )
					 )
				   )
				{
					resultSlice.at(j,i) = 1;
				}
			}
		}
	}
	else if ( hasMaxConcavity )
	{
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( initialSlice.at(j,i) > intensityThreshold &&
					 ( _contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill) ||
					   (
						   maxSupportPoint.vectorProduct( maxConcavityPoint, iCoord2D(i,j) ) <= 0 &&
						   _contourPolygonTop.containsPoint(QPoint(i,j),Qt::OddEvenFill)
					   )
					 )
				   )
				{
					resultSlice.at(j,i) = 1;
				}
			}
		}
	}
	else if ( hasMinConcavity )
	{
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( initialSlice.at(j,i) > intensityThreshold &&
					 ( _contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill) ||
					   (
						   minSupportPoint.vectorProduct( minConcavityPoint, iCoord2D(i,j) ) >= 0 &&
						   _contourPolygonTop.containsPoint(QPoint(i,j),Qt::OddEvenFill)
					   )
					 )
				   )
				{
					resultSlice.at(j,i) = 1;
				}
			}
		}
	}
	else if ( nbPointsContour > 3 )
	{
		// Sinon on ajoute la composante en entier
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( initialSlice.at(j,i) > intensityThreshold &&
					 _contourPolygonTop.containsPoint(QPoint(i,j),Qt::OddEvenFill) )
				{
					resultSlice.at(j,i) = 1;
				}
			}
		}
	}
}
