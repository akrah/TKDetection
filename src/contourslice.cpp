#include "inc/contourslice.h"

#include "inc/billon.h"
#include "inc/slicealgorithm.h"
#include "inc/curvaturehistogram.h"

#include <QProcess>
#include <QTemporaryFile>
#include <QPainter>

iCoord2D ContourSlice::invalidICoord2D = iCoord2D(-1,-1);

ContourSlice::ContourSlice()
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

const iCoord2D &ContourSlice::leftMainDominantPoint() const
{
	return _leftMainDominantPointsIndex != -1 ? _contour[_leftMainDominantPointsIndex] : invalidICoord2D;
}

const iCoord2D &ContourSlice::rightMainDominantPoint() const
{
	return _rightMainDominantPointsIndex != -1 ? _contour[_rightMainDominantPointsIndex] : invalidICoord2D;
}

const int &ContourSlice::leftMainDominantPointIndex() const
{
	return _leftMainDominantPointsIndex;
}

const int &ContourSlice::rightMainDominantPointIndex() const
{
	return _rightMainDominantPointsIndex;
}

const rCoord2D &ContourSlice::leftMainSupportPoint() const
{
	return _leftMainSupportPoint;
}

const rCoord2D &ContourSlice::rightMainSupportPoint() const
{
	return _rightMainSupportPoint;
}

/**********************************
 * Public setters
 **********************************/

void ContourSlice::compute( Slice &resultSlice, const Slice &initialSlice, const uiCoord2D &sliceCenter, const int &intensityThreshold,
							const int &smoothingRadius, const int &curvatureWidth, const qreal &curvatureThreshold, const iCoord2D &startPoint )
{
	_contour.compute( initialSlice, sliceCenter, intensityThreshold, startPoint );
	_originalContour = _contour;
	_contour.smooth(smoothingRadius);

	_contourDistancesHistogram.construct( _contour, sliceCenter );
	_curvatureHistogram.construct( _contour, curvatureWidth  );

	_sliceCenter = sliceCenter;

	computeMainDominantPoints( curvatureThreshold );
	computeSupportsOfMainDominantPoints( 10 );
	computeContourPolygons();
	updateSlice( initialSlice, resultSlice, intensityThreshold );
}

void ContourSlice::draw( QPainter &painter, const int &cursorPosition, const TKD::ViewType &viewType ) const
{
	const int nbContourPoints = _contour.size();
	const uint width = painter.window().width();
	const qreal angularFactor = width/TWO_PI;

	if ( nbContourPoints > 0 )
	{
		painter.save();
		_contour.draw(painter,cursorPosition,_sliceCenter,viewType);

		int i, j, x, y, i2, j2, x2, y2;

		// Dessins des points dominants principaux
		qreal a, b;
		if ( leftMainDominantPointIndex() != -1 )
		{
			// Dessin du point dominants principal gauche
			const iCoord2D &leftMainPoint = leftMainDominantPoint();

			painter.setPen(Qt::green);

			if ( viewType == TKD::Z_VIEW )
			{
				painter.drawEllipse(leftMainPoint.x-2,leftMainPoint.y-2,4,4);
			}
			else if ( viewType == TKD::CARTESIAN_VIEW )
			{
				i = leftMainPoint.x - _sliceCenter.x;
				j = leftMainPoint.y - _sliceCenter.y;
				y = qSqrt(qPow(i,2) + qPow(j,2));
				x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
				painter.drawEllipse(x-2,y-2,4,4);
			}

			// Dessins de la droite de coupe issue du point de dominant principal gauche
			const rCoord2D &leftSupportPoint = leftMainSupportPoint();
			if ( leftSupportPoint.x != -1 || leftSupportPoint.y != -1 )
			{
				painter.setPen(Qt::gray);
				a = ( leftMainPoint.y - leftSupportPoint.y ) / static_cast<qreal>( leftMainPoint.x - leftSupportPoint.x );
				b = ( leftMainPoint.y * leftSupportPoint.x - leftMainPoint.x * leftSupportPoint.y ) / static_cast<qreal>( leftSupportPoint.x - leftMainPoint.x );
				if ( leftSupportPoint.x < leftMainPoint.x )
				{
					if ( viewType == TKD::Z_VIEW )
					{
						painter.drawLine(leftMainPoint.x, leftMainPoint.y, width, a * width + b );
					}
					else if ( viewType == TKD::CARTESIAN_VIEW )
					{
						i = leftMainPoint.x - _sliceCenter.x;
						j = leftMainPoint.y - _sliceCenter.y;
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
						painter.drawLine(leftMainPoint.x, leftMainPoint.y, 0., b );
					}
					else if ( viewType == TKD::CARTESIAN_VIEW )
					{
						i = leftMainPoint.x - _sliceCenter.x;
						j = leftMainPoint.y - _sliceCenter.y;
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
		if ( rightMainDominantPointIndex() != -1 )
		{
			// Dessin du point dominants principal droit
			const iCoord2D &rightMainPoint = rightMainDominantPoint();

			painter.setPen(Qt::green);
			if ( viewType == TKD::Z_VIEW )
			{
				painter.drawEllipse(rightMainPoint.x-2,rightMainPoint.y-2,4,4);
			}
			else if ( viewType == TKD::CARTESIAN_VIEW )
			{
				i = rightMainPoint.x - _sliceCenter.x;
				j = rightMainPoint.y - _sliceCenter.y;
				y = qSqrt(qPow(i,2) + qPow(j,2));
				x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
				painter.drawEllipse(x-2,y-2,4,4);
			}

			// Dessins de la droite de coupe issue du point de dominant principal droit
			const rCoord2D &rightSupportPoint = rightMainSupportPoint();
			if ( rightSupportPoint.x != -1 || rightSupportPoint.y != -1 )
			{
				painter.setPen(Qt::gray);
				a = ( rightMainPoint.y - rightSupportPoint.y ) / static_cast<qreal>( rightMainPoint.x - rightSupportPoint.x );
				b = ( rightMainPoint.y * rightSupportPoint.x - rightMainPoint.x * rightSupportPoint.y ) / static_cast<qreal>( rightSupportPoint.x - rightMainPoint.x );
				if ( rightSupportPoint.x < rightMainPoint.x )
				{
					if ( viewType == TKD::Z_VIEW )
					{
						painter.drawLine(rightMainPoint.x, rightMainPoint.y, width, a * width + b );
					}
					else if ( viewType == TKD::CARTESIAN_VIEW )
					{
						i = rightMainPoint.x - _sliceCenter.x;
						j = rightMainPoint.y - _sliceCenter.y;
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
						painter.drawLine(rightMainPoint.x, rightMainPoint.y, 0., b );
					}
					else if ( viewType == TKD::CARTESIAN_VIEW )
					{
						i = rightMainPoint.x - _sliceCenter.x;
						j = rightMainPoint.y - _sliceCenter.y;
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

void ContourSlice::computeMainDominantPoints( const qreal &curvatureThreshold )
{
	_leftMainDominantPointsIndex = _rightMainDominantPointsIndex = -1;

	int nbPoints, index, indexToCompare;

	nbPoints = _contour.size();
	indexToCompare = nbPoints*0.3;

	if ( _contourDistancesHistogram.size() == nbPoints && _curvatureHistogram.size() == nbPoints )
	{
		// Left main dominant point
		index = 1;
		while ( (index < indexToCompare) && (_curvatureHistogram[index] > curvatureThreshold) )
		{
			index++;
		}
		if ( index < indexToCompare ) _leftMainDominantPointsIndex = index;

		// Right main dominant point
		index = nbPoints-2;
		indexToCompare = nbPoints-indexToCompare;
		while ( (index > indexToCompare) && (_curvatureHistogram[index] > curvatureThreshold) )
		{
			index--;
		}
		if ( index > indexToCompare ) _rightMainDominantPointsIndex = index;
	}
}

void ContourSlice::computeSupportsOfMainDominantPoints( const int &meansMaskSize )
{
	_leftMainSupportPoint = _rightMainSupportPoint = rCoord2D(-1,-1);

	int index, counter;

	// Support du MDP gauche
	index = _leftMainDominantPointsIndex;
	if ( index != -1 )
	{
		_leftMainSupportPoint.x = _leftMainSupportPoint.y = 0.;
		counter = 0;
		while ( index >= qMax(_leftMainDominantPointsIndex-meansMaskSize,0) )
		{
			_leftMainSupportPoint.x += _contour[index].x;
			_leftMainSupportPoint.y += _contour[index].y;
			--index;
			++counter;
		}
		_leftMainSupportPoint /= counter;
	}

	// Support du MDP droit
	index = _rightMainDominantPointsIndex;
	if ( index != -1 )
	{
		int nbPoints = _contour.size();
		counter = 0;
		while ( index < qMin(_rightMainDominantPointsIndex+meansMaskSize,nbPoints) )
		{
			_rightMainSupportPoint.x += _contour[index].x;
			_rightMainSupportPoint.y += _contour[index].y;
			++index;
			++counter;
		}
		_rightMainSupportPoint /= counter;
	}
}

void ContourSlice::computeContourPolygons()
{
	_contourPolygonBottom.clear();
	_contourPolygonTop.clear();

	const int nbContourPoints = _contour.size();
	if ( !nbContourPoints ) return;

	int i;

	// S'il y a deux points dominants principaux
	if ( _leftMainDominantPointsIndex != -1 && _rightMainDominantPointsIndex != -1 )
	{
		const iCoord2D &leftMainPoint = leftMainDominantPoint();
		const iCoord2D &rightMainPoint = rightMainDominantPoint();

		// Création du polygone qui servira à tester l'appartenance d'un pixel en dessous de la droite des deux points dominants principaux au noeud.
		// Ce polygone est constitué :
		//     - des points compris entre le premier point du contour lissé et le point dominant principal gauche
		//     - du point dominant principal gauche
		//     - du point dominant principal droit
		//     - des points compris entre le point dominant principal droit et le dernier points du contour lissé.
		for ( i=0 ; i<=_leftMainDominantPointsIndex ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
		_contourPolygonBottom << QPoint(leftMainPoint.x,leftMainPoint.y)
							 << QPoint(rightMainPoint.x,rightMainPoint.y);
		for ( i=_rightMainDominantPointsIndex ; i<nbContourPoints ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
		_contourPolygonBottom << QPoint(_contour[0].x,_contour[0].y);

		// Création du polygone qui servira à tester l'appartenance d'un pixel au dessus de la droite des deux points dominants principaux au noeud.
		// Ce polygone est constitué :
		//     - du point le plus proche du point dominant
		//     - des points du contour situés entre le point le point dominant principal gauche et le point dominant principal droit
		//     - du point dominant principal droit
		_contourPolygonTop << QPoint(leftMainPoint.x,leftMainPoint.y);
		for ( i=_leftMainDominantPointsIndex ; i<=_rightMainDominantPointsIndex ; ++i ) _contourPolygonTop << QPoint(_contour[i].x,_contour[i].y);
		_contourPolygonTop << QPoint(rightMainPoint.x,rightMainPoint.y)
						  << QPoint(leftMainPoint.x,leftMainPoint.y);
	}
	else
	{
		if ( _leftMainDominantPointsIndex != -1 )
		{
			const iCoord2D &leftMainPoint = leftMainDominantPoint();
			const int rightMainDominantPointsIndexComputed = nbContourPoints-_leftMainDominantPointsIndex;
			const iCoord2D &rightMainPoint = _contour[rightMainDominantPointsIndexComputed];

			// Création du polygone qui servira à tester l'appartenance d'un pixel en dessous de la droite des deux points dominants principaux au noeud.
			// Ce polygone est constitué :
			//     - des points compris entre le premier point du contour lissé et le point dominant principal gauche
			//     - du point dominant principal gauche
			//     - du point dominant principal droit calculé
			//     - des points compris entre le point dominant principal droit principal calculé et le dernier points du contour lissé.
			for ( i=0 ; i<=_leftMainDominantPointsIndex ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
			_contourPolygonBottom << QPoint(leftMainPoint.x,leftMainPoint.y)
								  << QPoint(rightMainPoint.x,rightMainPoint.y);
			for ( i=rightMainDominantPointsIndexComputed ; i<nbContourPoints ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
			_contourPolygonBottom << QPoint(_contour[0].x,_contour[0].y);
		}
		else if ( _rightMainDominantPointsIndex != -1 )
		{
			const iCoord2D &rightMainPoint = rightMainDominantPoint();
			const int leftMainDominantPointsIndexComputed = nbContourPoints-_rightMainDominantPointsIndex;
			const iCoord2D &leftMainPoint = _contour[leftMainDominantPointsIndexComputed];

			// Création du polygone qui servira à tester l'appartenance d'un pixel en dessous de la droite des deux points dominants principaux au noeud.
			// Ce polygone est constitué :
			//     - des points compris entre le premier point du contour lissé et le point dominant principal gauche
			//     - du point dominant principal gauche
			//     - du point dominant principal droit calculé
			//     - des points compris entre le point dominant principal droit principal calculé et le dernier points du contour lissé.
			for ( i=0 ; i<=leftMainDominantPointsIndexComputed ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
			_contourPolygonBottom << QPoint(leftMainPoint.x,leftMainPoint.y)
								  << QPoint(rightMainPoint.x,rightMainPoint.y);
			for ( i=_rightMainDominantPointsIndex ; i<nbContourPoints ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
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

	const iCoord2D &leftMainPoint = leftMainDominantPoint();
	const iCoord2D &rightMainPoint = rightMainDominantPoint();
	const rCoord2D &leftSupportPoint = leftMainSupportPoint();
	const rCoord2D &rightSupportPoint = rightMainSupportPoint();

	const bool hasMainLeft = _leftMainDominantPointsIndex != -1;
	const bool hasMainRight = _rightMainDominantPointsIndex != -1;

	int i,j;

	// S'il y a deux points dominants principaux
	if ( hasMainLeft && hasMainRight )
	{
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( initialSlice.at(j,i) > intensityThreshold &&
					 ( _contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill) ||
					   (
						   leftSupportPoint.vectorProduct( leftMainPoint, iCoord2D(i,j) ) >= 0 &&
						   rightSupportPoint.vectorProduct( rightMainPoint, iCoord2D(i,j) ) <= 0 &&
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
	else if ( hasMainLeft )
	{
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( initialSlice.at(j,i) > intensityThreshold &&
					 ( _contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill) ||
					   (
						   leftSupportPoint.vectorProduct( leftMainPoint, iCoord2D(i,j) ) >= 0 &&
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
	else if ( hasMainRight )
	{
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( initialSlice.at(j,i) > intensityThreshold &&
					 ( _contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill) ||
					   (
						   rightSupportPoint.vectorProduct( rightMainPoint, iCoord2D(i,j) ) <= 0 &&
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
