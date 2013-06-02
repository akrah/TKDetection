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

const iCoord2D &ContourSlice::dominantPointFromLeft( const uint &index ) const
{
	Q_ASSERT_X( index<static_cast<uint>(_dominantPointsIndexFromLeft.size()), "Histogram::mainDominantPoint", "Le point dominants demandé n'existe pas" );
	return _contour[_dominantPointsIndexFromLeft[index]];
}

const iCoord2D &ContourSlice::dominantPointFromRight( const uint &index ) const
{
	Q_ASSERT_X( index<static_cast<uint>(_dominantPointsIndexFromRight.size()), "Histogram::mainDominantPoint", "Le point dominants demandé n'existe pas" );
	return _contour[_dominantPointsIndexFromRight[index]];
}

const QVector<int> &ContourSlice::dominantPointIndexFromLeft() const
{
	return _dominantPointsIndexFromLeft;
}

const QVector<int> &ContourSlice::dominantPointIndexFromRight() const
{
	return _dominantPointsIndexFromRight;
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
							const int &blurredSegmentThickness, const int &smoothingRadius, const int &curvatureWidth, const iCoord2D &startPoint )
{
	_contour.compute( initialSlice, sliceCenter, intensityThreshold, startPoint );
	_originalContour = _contour;
	_contour.smooth(smoothingRadius);

	_contourDistancesHistogram.construct( _contour, sliceCenter );
	_curvatureHistogram.construct( _contour, curvatureWidth  );

	_sliceCenter = sliceCenter;

	computeDominantPoints( blurredSegmentThickness );
	computeMainDominantPoints();
	computeSupportsOfMainDominantPoints();
	computeContourPolygons();
	updateSlice( initialSlice, resultSlice, intensityThreshold );
}

void ContourSlice::computeOldMethod( Slice &resultSlice, const Slice &initialSlice, const uiCoord2D &sliceCenter, const int &intensityThreshold, const int &smoothingRadius, const int &curvatureWidth, const iCoord2D &startPoint )
{
	_contour.clear();
	_originalContour.clear();
	_contourDistancesHistogram.clear();
	_curvatureHistogram.clear();
	_dominantPointsIndexFromLeft.clear();
	_leftMainDominantPointsIndex = _rightMainDominantPointsIndex = -1;
	_leftMainSupportPoint = _rightMainSupportPoint = rCoord2D(-1,-1);
	_contourPolygonBottom.clear();
	_contourPolygonTop.clear();

	_contour.compute( initialSlice, sliceCenter, intensityThreshold, startPoint );
	_originalContour = _contour;
	_contour.smooth(smoothingRadius);

	_contourDistancesHistogram.construct( _contour, sliceCenter );
	_curvatureHistogram.construct( _contour, curvatureWidth );

	const int width = initialSlice.n_cols;
	const int height = initialSlice.n_rows;
	const int nbOriginalPointsContour = _contour.size();

	int i, j;
	QPolygon contourPolygonBottom;

	for ( i=0 ; i<nbOriginalPointsContour ; ++i )
	{
		contourPolygonBottom << QPoint(_contour[i].x,_contour[i].y);
	}
	contourPolygonBottom << QPoint(_contour[0].x,_contour[0].y);

	resultSlice.set_size(height,width);
	for ( j = 0 ; j<height ; ++j )
	{
		for ( i=0 ; i<width ; ++i )
		{
			resultSlice.at(j,i) = (initialSlice.at(j,i) > intensityThreshold && contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill));
		}
	}
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

		const int nbDominantPoints = _dominantPointsIndexFromLeft.size();
		const int nbDominantPoints2 = _dominantPointsIndexFromRight.size();
		if ( nbDominantPoints > 0 && nbDominantPoints2 > 0 )
		{
			// Dessin des points dominants

			int i, j, k, x, y, i2, j2, x2, y2;
			painter.setPen(Qt::green);
			if ( viewType == TKD::Z_VIEW )
			{
				for ( i=0 ; i<nbDominantPoints ; ++i )
				{
					painter.drawEllipse(dominantPointFromLeft(i).x-2,dominantPointFromLeft(i).y-2,4,4);
				}
			}
			else if ( viewType == TKD::CARTESIAN_VIEW )
			{
				for ( k=0 ; k<nbDominantPoints ; ++k )
				{
					i = dominantPointFromLeft(k).x - _sliceCenter.x;
					j = dominantPointFromLeft(k).y - _sliceCenter.y;
					y = qSqrt(qPow(i,2) + qPow(j,2));
					x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
					painter.drawEllipse(x-2,y-2,4,4);
				}
			}

			painter.setPen(Qt::yellow);
			if ( viewType == TKD::Z_VIEW )
			{
				for ( i=0 ; i<nbDominantPoints2 ; ++i )
				{
					painter.drawEllipse(dominantPointFromRight(i).x-3,dominantPointFromRight(i).y-3,6,6);
				}
			}
			else if ( viewType == TKD::CARTESIAN_VIEW )
			{
				for ( k=0 ; k<nbDominantPoints2 ; ++k )
				{
					i = dominantPointFromRight(k).x - _sliceCenter.x;
					j = dominantPointFromRight(k).y - _sliceCenter.y;
					y = qSqrt(qPow(i,2) + qPow(j,2));
					x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
					painter.drawEllipse(x-3,y-3,6,6);
				}
			}

			// Dessins des points dominants principaux
			qreal a, b;
			if ( leftMainDominantPointIndex() != -1 )
			{
				// Dessin du point dominants principal gauche
				const iCoord2D &leftMainPoint = leftMainDominantPoint();

				painter.setPen(Qt::red);

				if ( viewType == TKD::Z_VIEW )
				{
					painter.drawLine(leftMainPoint.x-2,leftMainPoint.y-2,leftMainPoint.x+2,leftMainPoint.y+2);
					painter.drawLine(leftMainPoint.x-2,leftMainPoint.y+2,leftMainPoint.x+2,leftMainPoint.y-2);
				}
				else if ( viewType == TKD::CARTESIAN_VIEW )
				{
					i = leftMainPoint.x - _sliceCenter.x;
					j = leftMainPoint.y - _sliceCenter.y;
					y = qSqrt(qPow(i,2) + qPow(j,2));
					x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
					painter.drawLine(x-2,y-2,x+2,y+2);
					painter.drawLine(x-2,y+2,x+2,y-2);
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

				painter.setPen(Qt::red);
				if ( viewType == TKD::Z_VIEW )
				{
					painter.drawLine(rightMainPoint.x-2,rightMainPoint.y-2,rightMainPoint.x+2,rightMainPoint.y+2);
					painter.drawLine(rightMainPoint.x-2,rightMainPoint.y+2,rightMainPoint.x+2,rightMainPoint.y-2);
				}
				else if ( viewType == TKD::CARTESIAN_VIEW )
				{
					i = rightMainPoint.x - _sliceCenter.x;
					j = rightMainPoint.y - _sliceCenter.y;
					y = qSqrt(qPow(i,2) + qPow(j,2));
					x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
					painter.drawLine(x-2,y-2,x+2,y+2);
					painter.drawLine(x-2,y+2,x+2,y-2);
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

			// Dessin du point de contour initial (également point dominant initial)
			painter.setPen(Qt::red);
			if ( viewType == TKD::Z_VIEW )
			{
				painter.drawEllipse(_contour[0].x-1,_contour[0].y-1,2,2);
			}
			else if ( viewType == TKD::CARTESIAN_VIEW )
			{
				i = _contour[0].x - _sliceCenter.x;
				j = _contour[0].y - _sliceCenter.y;
				y = qSqrt(qPow(i,2) + qPow(j,2));
				x = 2. * qAtan( j / (qreal)(i + y) ) * angularFactor;
				painter.drawEllipse(x-1,y-1,2,2);
			}
		}

		painter.restore();
	}
}

/**********************************
 * Private setters
 **********************************/

void ContourSlice::computeDominantPoints( const int &blurredSegmentThickness )
{
	_dominantPointsIndexFromLeft.clear();
	_dominantPointsIndexFromRight.clear();

	int nbPoints, nbDominantPoints;
	nbPoints = _contour.size();
	if ( nbPoints > 0 )
	{
		// Ecriture des données de points de contours
		QTemporaryFile fileContours("TKDetection_XXXXXX.ctr");
		if ( !fileContours.open() )
		{
			qDebug() << QObject::tr("ERREUR : Impossible de créer le ficher de contours %1.").arg(fileContours.fileName());
			return;
		}
		QTemporaryFile fileContours2("TKDetection_XXXXXX_inverse.ctr");
		if ( !fileContours2.open() )
		{
			qDebug() << QObject::tr("ERREUR : Impossible de créer le ficher de contours %1.").arg(fileContours2.fileName());
			return;
		}

		QTextStream streamContours(&fileContours);
		QTextStream streamContours2(&fileContours2);
		streamContours2 << _contour[0].x << " " << _contour[0].y << endl;
		for ( int i=0 ; i<nbPoints ; ++i )
		{
			streamContours << _contour[i].x << " " << _contour[i].y << endl;
			streamContours2 << _contour[nbPoints-i-1].x << " " << _contour[nbPoints-i-1].y << endl;
		}
		streamContours << _contour[0].x << " " << _contour[0].y << endl;
		fileContours.close();
		fileContours2.close();

		// Extraction des points dominants à partir des points de contour
		QTemporaryFile fileDominantPoint("TKDetection_XXXXXX.dp");
		if( !fileDominantPoint.open() )
		{
			qDebug() << QObject::tr("ERREUR : Impossible de créer le ficher de points dominants %1.").arg(fileDominantPoint.fileName());
			return;
		}
		fileDominantPoint.close();

		QProcess dominantPointExtraction;
		dominantPointExtraction.setStandardInputFile(fileContours.fileName());
		dominantPointExtraction.start(QString("cornerdetection -epais %1 -pointFile %2").arg(blurredSegmentThickness).arg(fileDominantPoint.fileName()));

		if ( dominantPointExtraction.waitForFinished(3000) )
		{
			if( !fileDominantPoint.open() )
			{
				qDebug() << QObject::tr("ERREUR : Impossible de lire le ficher de points dominants %1.").arg(fileDominantPoint.fileName());
				return;
			}

			QTextStream streamDominantPoints(&fileDominantPoint);
			streamDominantPoints >> nbDominantPoints;
			_dominantPointsIndexFromLeft.resize(nbDominantPoints);
			for ( int i=0 ; i<nbDominantPoints ; ++i )
			{
				streamDominantPoints >> _dominantPointsIndexFromLeft[i];
			}

			fileDominantPoint.close();
		}

		// Extraction des points dominants à partir des points de contour
		QTemporaryFile fileDominantPoint2("TKDetection_XXXXXX_inverse.dp");
		if( !fileDominantPoint2.open() )
		{
			qDebug() << QObject::tr("ERREUR : Impossible de créer le ficher de points dominants %1.").arg(fileDominantPoint2.fileName());
			return;
		}
		fileDominantPoint2.close();

		QProcess dominantPointExtraction2;
		dominantPointExtraction2.setStandardInputFile(fileContours2.fileName());
		dominantPointExtraction2.start(QString("cornerdetection -epais %1 -pointFile %2").arg(blurredSegmentThickness).arg(fileDominantPoint2.fileName()));

		if ( dominantPointExtraction2.waitForFinished(3000) )
		{
			if( !fileDominantPoint2.open() )
			{
				qDebug() << QObject::tr("ERREUR : Impossible de lire le ficher de points dominants %1.").arg(fileDominantPoint2.fileName());
				return;
			}

			QTextStream streamDominantPoints2(&fileDominantPoint2);
			streamDominantPoints2 >> nbDominantPoints;
			_dominantPointsIndexFromRight.resize(nbDominantPoints);
			int index;
			for ( int i=0 ; i<nbDominantPoints ; ++i )
			{
				streamDominantPoints2 >> index;
				_dominantPointsIndexFromRight[i] = nbPoints-1 - index;
			}

			fileDominantPoint2.close();
		}
	}
}

void ContourSlice::computeMainDominantPoints()
{
	_leftMainDominantPointsIndex = _rightMainDominantPointsIndex = -1;

	int nbDominantPoints, nbDominantPointsToCompare, nbPoints, index, indexToCompare, increment;

	QVector<int> allDominantPointsIndex(_dominantPointsIndexFromLeft);
	allDominantPointsIndex << _dominantPointsIndexFromRight;
	qSort(allDominantPointsIndex);

	nbDominantPoints = allDominantPointsIndex.size();
	nbDominantPointsToCompare = nbDominantPoints/2;
	nbPoints = _contour.size();
	indexToCompare = nbPoints*0.35;

	if ( nbDominantPoints > 2 && _contourDistancesHistogram.size() == nbPoints && _curvatureHistogram.size() == nbPoints )
	{
		// Left main dominant point
		increment = 1;
		do
		{
			index = allDominantPointsIndex[increment++];
		}
		while ( (index < indexToCompare) && (_curvatureHistogram[index]>=0 ) );

		if ( index<indexToCompare && increment<nbDominantPointsToCompare )
		{
			while ( index && _curvatureHistogram[index]<0 ) index--;
			_leftMainDominantPointsIndex = index;
		}

		// Right main dominant point
		increment = nbDominantPoints-1;
		indexToCompare = nbPoints - indexToCompare;
		do
		{
			index = allDominantPointsIndex[increment--];
		}
		while ( (index > indexToCompare) && (_curvatureHistogram[index]>=0 ) );

		if ( index>indexToCompare && increment>nbDominantPointsToCompare )
		{
			while ( index<nbPoints && _curvatureHistogram[index]<0 ) index++;
			_rightMainDominantPointsIndex = index;
		}
	}
}

void ContourSlice::computeSupportsOfMainDominantPoints()
{
	_leftMainSupportPoint = _rightMainSupportPoint = rCoord2D(-1,-1);

	int index, counter;

	// Support du MDP gauche
	index = _leftMainDominantPointsIndex;
	if ( index != -1 )
	{
		_leftMainSupportPoint.x = _leftMainSupportPoint.y = 0.;
		counter = 0;
		while ( index >= qMax(_leftMainDominantPointsIndex-30,0) )
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
		while ( index < qMin(_rightMainDominantPointsIndex+30,nbPoints) )
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
