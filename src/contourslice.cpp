#include "inc/contourslice.h"

#include "inc/billon.h"
#include "inc/slicealgorithm.h"
#include "inc/curvaturehistogram.h"

#include <QProcess>
#include <QTemporaryFile>
#include <QPainter>

ContourSlice::ContourSlice() : _leftConcavityPointsIndex(-1), _rightConcavityPointsIndex(-1)
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

const iCoord2D &ContourSlice::leftConcavityPoint() const
{
	return _leftConcavityPointsIndex != -1 ? _contour[_leftConcavityPointsIndex] : invalidICoord2D;
}

const iCoord2D &ContourSlice::rightConcavityPoint() const
{
	return _rightConcavityPointsIndex != -1 ? _contour[_rightConcavityPointsIndex] : invalidICoord2D;
}

const int &ContourSlice::leftConcavityPointIndex() const
{
	return _leftConcavityPointsIndex;
}

const int &ContourSlice::rightConcavityPointIndex() const
{
	return _rightConcavityPointsIndex;
}

const rCoord2D &ContourSlice::leftSupportPoint() const
{
	return _leftSupportPoint;
}

const rCoord2D &ContourSlice::rightSupportPoint() const
{
	return _rightSupportPoint;
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
	_contour.compute( initialSlice, sliceCenter, intensityThreshold, startPoint );
	_originalContour = _contour;
	_contour.smooth(smoothingRadius);

	_contourDistancesHistogram.construct( _contour, sliceCenter );
	_curvatureHistogram.construct( _contour, curvatureWidth  );

	_sliceCenter = sliceCenter;

	computeConcavityPoints( curvatureThreshold );
	computeSupportPoints( 10 );
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
		if ( leftConcavityPointIndex() != -1 )
		{
			// Dessin du point de concavité gauche
			const iCoord2D &leftConcavityPoint = this->leftConcavityPoint();

			painter.setPen(Qt::green);

			if ( viewType == TKD::Z_VIEW )
			{
				painter.drawEllipse(leftConcavityPoint.x-2,leftConcavityPoint.y-2,4,4);
			}
			else if ( viewType == TKD::CARTESIAN_VIEW )
			{
				i = leftConcavityPoint.x - _sliceCenter.x;
				j = leftConcavityPoint.y - _sliceCenter.y;
				y = qSqrt(qPow(i,2) + qPow(j,2));
				x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
				painter.drawEllipse(x-2,y-2,4,4);
			}

			// Dessins de la droite de coupe issue du point de concavité gauche
			const rCoord2D &leftSupportPoint = this->leftSupportPoint();
			if ( leftSupportPoint.x != -1 || leftSupportPoint.y != -1 )
			{
				painter.setPen(Qt::gray);
				a = ( leftConcavityPoint.y - leftSupportPoint.y ) / static_cast<qreal>( leftConcavityPoint.x - leftSupportPoint.x );
				b = ( leftConcavityPoint.y * leftSupportPoint.x - leftConcavityPoint.x * leftSupportPoint.y ) / static_cast<qreal>( leftSupportPoint.x - leftConcavityPoint.x );
				if ( qFuzzyCompare(leftConcavityPoint.x, leftSupportPoint.x) )
				{
					if ( leftSupportPoint.y < leftConcavityPoint.y ) painter.drawLine(leftConcavityPoint.x, leftConcavityPoint.y, leftConcavityPoint.x, height );
					else painter.drawLine(leftConcavityPoint.x, leftConcavityPoint.y, leftConcavityPoint.x, 0 );
				}
				else if ( leftSupportPoint.x < leftConcavityPoint.x )
				{
					if ( viewType == TKD::Z_VIEW )
					{
						painter.drawLine(leftConcavityPoint.x, leftConcavityPoint.y, width, a * width + b );
					}
					else if ( viewType == TKD::CARTESIAN_VIEW )
					{
						i = leftConcavityPoint.x - _sliceCenter.x;
						j = leftConcavityPoint.y - _sliceCenter.y;
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
						painter.drawLine(leftConcavityPoint.x, leftConcavityPoint.y, 0., b );
					}
					else if ( viewType == TKD::CARTESIAN_VIEW )
					{
						i = leftConcavityPoint.x - _sliceCenter.x;
						j = leftConcavityPoint.y - _sliceCenter.y;
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
		if ( rightConcavityPointIndex() != -1 )
		{
			// Dessin du point de concavité droit
			const iCoord2D &rightConcavityPoint = this->rightConcavityPoint();

			painter.setPen(Qt::green);
			if ( viewType == TKD::Z_VIEW )
			{
				painter.drawEllipse(rightConcavityPoint.x-2,rightConcavityPoint.y-2,4,4);
			}
			else if ( viewType == TKD::CARTESIAN_VIEW )
			{
				i = rightConcavityPoint.x - _sliceCenter.x;
				j = rightConcavityPoint.y - _sliceCenter.y;
				y = qSqrt(qPow(i,2) + qPow(j,2));
				x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
				painter.drawEllipse(x-2,y-2,4,4);
			}

			// Dessins de la droite de coupe issue du point de concavité droit
			const rCoord2D &rightSupportPoint = this->rightSupportPoint();
			if ( rightSupportPoint.x != -1 || rightSupportPoint.y != -1 )
			{
				painter.setPen(Qt::gray);
				a = ( rightConcavityPoint.y - rightSupportPoint.y ) / static_cast<qreal>( rightConcavityPoint.x - rightSupportPoint.x );
				b = ( rightConcavityPoint.y * rightSupportPoint.x - rightConcavityPoint.x * rightSupportPoint.y ) / static_cast<qreal>( rightSupportPoint.x - rightConcavityPoint.x );
				if ( qFuzzyCompare(rightConcavityPoint.x, rightSupportPoint.x) )
				{
					if ( viewType == TKD::Z_VIEW )
					{
						if ( rightSupportPoint.y < rightConcavityPoint.y ) painter.drawLine(rightConcavityPoint.x, rightConcavityPoint.y, rightConcavityPoint.x, height );
						else painter.drawLine(rightConcavityPoint.x, rightConcavityPoint.y, rightConcavityPoint.x, 0 );
					}
				}
				else if ( rightSupportPoint.x < rightConcavityPoint.x )
				{
					if ( viewType == TKD::Z_VIEW )
					{
						painter.drawLine(rightConcavityPoint.x, rightConcavityPoint.y, width, a * width + b );
					}
					else if ( viewType == TKD::CARTESIAN_VIEW )
					{
						i = rightConcavityPoint.x - _sliceCenter.x;
						j = rightConcavityPoint.y - _sliceCenter.y;
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
						painter.drawLine(rightConcavityPoint.x, rightConcavityPoint.y, 0., b );
					}
					else if ( viewType == TKD::CARTESIAN_VIEW )
					{
						i = rightConcavityPoint.x - _sliceCenter.x;
						j = rightConcavityPoint.y - _sliceCenter.y;
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
	_leftConcavityPointsIndex = _rightConcavityPointsIndex = -1;

	int nbPoints, index, indexToCompare;

	nbPoints = _contour.size();
	indexToCompare = nbPoints*0.3;

	if ( nbPoints > 0 && _contourDistancesHistogram.size() == nbPoints && _curvatureHistogram.size() == nbPoints )
	{
		// Left concavity point
		index = 1;
		while ( (index < indexToCompare) && (_curvatureHistogram[index] > curvatureThreshold) )
		{
			index++;
		}
		if ( index < indexToCompare ) _leftConcavityPointsIndex = index;

		// Right concavity point
		index = nbPoints-2;
		indexToCompare = nbPoints-indexToCompare;
		while ( (index > indexToCompare) && (_curvatureHistogram[index] > curvatureThreshold) )
		{
			index--;
		}
		if ( index > indexToCompare ) _rightConcavityPointsIndex = index;
	}
}

void ContourSlice::computeSupportPoints( const int &meansMaskSize )
{
	_leftSupportPoint = _rightSupportPoint = rCoord2D(-1,-1);

	int index, counter;

	// Support du MDP gauche
	index = _leftConcavityPointsIndex;
	if ( index != -1 )
	{
		_leftSupportPoint.x = _leftSupportPoint.y = 0.;
		counter = 0;
		while ( index >= qMax(_leftConcavityPointsIndex-meansMaskSize,0) )
		{
			_leftSupportPoint.x += _contour[index].x;
			_leftSupportPoint.y += _contour[index].y;
			--index;
			++counter;
		}
		_leftSupportPoint /= counter;
	}

	// Support du MDP droit
	index = _rightConcavityPointsIndex;
	if ( index != -1 )
	{
		int nbPoints = _contour.size();
		counter = 0;
		while ( index < qMin(_rightConcavityPointsIndex+meansMaskSize,nbPoints) )
		{
			_rightSupportPoint.x += _contour[index].x;
			_rightSupportPoint.y += _contour[index].y;
			++index;
			++counter;
		}
		_rightSupportPoint /= counter;
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
	if ( _leftConcavityPointsIndex != -1 && _rightConcavityPointsIndex != -1 )
	{
		const iCoord2D &leftConcavityPoint = this->leftConcavityPoint();
		const iCoord2D &rightConcavityPoint = this->rightConcavityPoint();

		// Création du polygone qui servira à tester l'appartenance d'un pixel en dessous de la droite des deux points de concavité au noeud.
		// Ce polygone est constitué :
		//     - des points compris entre le premier point du contour lissé et le point de concavité gauche
		//     - du point de concavité gauche
		//     - du point de concavité droit
		//     - des points compris entre le point de concavité droit et le dernier points du contour lissé.
		for ( i=0 ; i<=_leftConcavityPointsIndex ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
		_contourPolygonBottom << QPoint(leftConcavityPoint.x,leftConcavityPoint.y)
							 << QPoint(rightConcavityPoint.x,rightConcavityPoint.y);
		for ( i=_rightConcavityPointsIndex ; i<nbContourPoints ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
		_contourPolygonBottom << QPoint(_contour[0].x,_contour[0].y);

		// Création du polygone qui servira à tester l'appartenance d'un pixel au dessus de la droite des deux points de concavité au noeud.
		// Ce polygone est constitué :
		//     - du point le plus proche du point de concavité
		//     - des points du contour situés entre le point le point de concavité gauche et le point de concavité droit
		//     - du point de concavité droit
		_contourPolygonTop << QPoint(leftConcavityPoint.x,leftConcavityPoint.y);
		for ( i=_leftConcavityPointsIndex ; i<=_rightConcavityPointsIndex ; ++i ) _contourPolygonTop << QPoint(_contour[i].x,_contour[i].y);
		_contourPolygonTop << QPoint(rightConcavityPoint.x,rightConcavityPoint.y)
						  << QPoint(leftConcavityPoint.x,leftConcavityPoint.y);
	}
	else
	{
		if ( _leftConcavityPointsIndex != -1 )
		{
			const iCoord2D &leftConcavityPoint = this->leftConcavityPoint();
			const int rightConcavityPointsIndexComputed = nbContourPoints-_leftConcavityPointsIndex;
			const iCoord2D &rightConcavityPoint = _contour[rightConcavityPointsIndexComputed];

			// Création du polygone qui servira à tester l'appartenance d'un pixel en dessous de la droite des deux points de concavité au noeud.
			// Ce polygone est constitué :
			//     - des points compris entre le premier point du contour lissé et le point de concavité gauche
			//     - du point de concavité gauche
			//     - du pointde concavité droit calculé
			//     - des points compris entre le point de concavité droit principal calculé et le dernier points du contour lissé.
			for ( i=0 ; i<=_leftConcavityPointsIndex ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
			_contourPolygonBottom << QPoint(leftConcavityPoint.x,leftConcavityPoint.y)
								  << QPoint(rightConcavityPoint.x,rightConcavityPoint.y);
			for ( i=rightConcavityPointsIndexComputed ; i<nbContourPoints ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
			_contourPolygonBottom << QPoint(_contour[0].x,_contour[0].y);
		}
		else if ( _rightConcavityPointsIndex != -1 )
		{
			const iCoord2D &rightConcavityPoint = this->rightConcavityPoint();
			const int leftConcavityPointsIndexComputed = nbContourPoints-_rightConcavityPointsIndex;
			const iCoord2D &leftConcavityPoint = _contour[leftConcavityPointsIndexComputed];

			// Création du polygone qui servira à tester l'appartenance d'un pixel en dessous de la droite des deux points de concavité au noeud.
			// Ce polygone est constitué :
			//     - des points compris entre le premier point du contour lissé et le point de concavité principal gauche
			//     - du point de concavité gauche
			//     - du point de concavité droit calculé
			//     - des points compris entre le point de concavité droit principal calculé et le dernier points du contour lissé.
			for ( i=0 ; i<=leftConcavityPointsIndexComputed ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
			_contourPolygonBottom << QPoint(leftConcavityPoint.x,leftConcavityPoint.y)
								  << QPoint(rightConcavityPoint.x,rightConcavityPoint.y);
			for ( i=_rightConcavityPointsIndex ; i<nbContourPoints ; ++i ) _contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
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

	const iCoord2D &leftConcavityPoint = this->leftConcavityPoint();
	const iCoord2D &rightConcavityPoint = this->rightConcavityPoint();
	const rCoord2D &leftSupportPoint = this->leftSupportPoint();
	const rCoord2D &rightSupportPoint = this->rightSupportPoint();

	const bool hasLeftConcavity = _leftConcavityPointsIndex != -1;
	const bool hasRightConcavity = _rightConcavityPointsIndex != -1;

	int i,j;

	// S'il y a deux points de concavité
	if ( hasLeftConcavity && hasRightConcavity )
	{
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( initialSlice.at(j,i) > intensityThreshold &&
					 ( _contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill) ||
					   (
						   leftSupportPoint.vectorProduct( leftConcavityPoint, iCoord2D(i,j) ) >= 0 &&
						   rightSupportPoint.vectorProduct( rightConcavityPoint, iCoord2D(i,j) ) <= 0 &&
						   leftConcavityPoint.vectorProduct( rightConcavityPoint, iCoord2D(i,j) ) <= 0 &&
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
	else if ( hasLeftConcavity )
	{
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( initialSlice.at(j,i) > intensityThreshold &&
					 ( _contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill) ||
					   (
						   leftSupportPoint.vectorProduct( leftConcavityPoint, iCoord2D(i,j) ) >= 0 &&
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
	else if ( hasRightConcavity )
	{
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( initialSlice.at(j,i) > intensityThreshold &&
					 ( _contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill) ||
					   (
						   rightSupportPoint.vectorProduct( rightConcavityPoint, iCoord2D(i,j) ) <= 0 &&
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
