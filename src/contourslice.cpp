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
							const int &blurredSegmentThickness, const int &smoothingRadius, const int &curvatureWidth,
							const int &minimumOriginDistance, const iCoord2D &startPoint )
{
	_contour.compute( initialSlice, sliceCenter, intensityThreshold, startPoint );
	_originalContour = _contour;
	_contour.smooth(smoothingRadius);

	_contourDistancesHistogram.construct( _contour, sliceCenter );
	//_curvatureHistogram.construct( _contour, curvatureWidth  );

	_sliceCenter = sliceCenter;

	computeDominantPoints( blurredSegmentThickness );
	computeMainDominantPoints( minimumOriginDistance );
	computeSupportsOfMainDominantPoints();
	computeContourPolygons();
	updateSlice( initialSlice, resultSlice, sliceCenter, intensityThreshold );
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

void ContourSlice::computeMainDominantPoints( const int &minimumOriginDistance )
{
	_leftMainDominantPointsIndex = _rightMainDominantPointsIndex = -1;

	int nbDominantPoints, nbDominantPoints2, nbPoints, index, increment;
	qreal currentDistance, previousDistance;

	nbDominantPoints = _dominantPointsIndexFromLeft.size();
	nbDominantPoints2 = _dominantPointsIndexFromRight.size();
	nbPoints = _contour.size();

	if ( nbDominantPoints > 2 && nbDominantPoints2 > 2 && _contourDistancesHistogram.size() == nbPoints )
	{
		increment = 0;
		do
		{
			index = _dominantPointsIndexFromLeft[++increment];
		}
		while ( (_contourDistancesHistogram[index]-_contourDistancesHistogram[0] < minimumOriginDistance) && (increment < nbDominantPoints-1) );

		if ( increment<nbDominantPoints-1 )
		{
			currentDistance = _contourDistancesHistogram[index];
			previousDistance = _contourDistancesHistogram[index-2];
			while ( index>1 && previousDistance>currentDistance )
			{
				index--;
				currentDistance = _contourDistancesHistogram[index];
				previousDistance = _contourDistancesHistogram[index-2];
			}
			_leftMainDominantPointsIndex = index;
		}

		increment = 0;
		do
		{
			index = _dominantPointsIndexFromRight[++increment];
		}
		while ( (_contourDistancesHistogram[index]-_contourDistancesHistogram[0] < minimumOriginDistance) && (increment < nbDominantPoints2-1) );

		if ( increment<nbDominantPoints2-1 )
		{
			currentDistance = _contourDistancesHistogram[index];
			previousDistance = _contourDistancesHistogram[index+2];
			while ( index<nbPoints-2 && previousDistance>currentDistance )
			{
				index++;
				currentDistance = _contourDistancesHistogram[index];
				previousDistance = _contourDistancesHistogram[index+2];
			}
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
		while ( index >= 0 )
		{
			_leftMainSupportPoint.x += _contour[index].x;
			_leftMainSupportPoint.y += _contour[index].y;
			index -= 5;
			++counter;
		}
		_leftMainSupportPoint.x /= counter;
		_leftMainSupportPoint.y /= counter;
	}

	// Support du MDP droit
	index = _rightMainDominantPointsIndex;
	if ( index != -1 )
	{
		int nbPoints = _contour.size();
		counter = 0;
		while ( index < nbPoints )
		{
			_rightMainSupportPoint.x += _contour[index].x;
			_rightMainSupportPoint.y += _contour[index].y;
			index += 5;
			++counter;
		}
		_rightMainSupportPoint.x /= counter;
		_rightMainSupportPoint.y /= counter;
	}
}

void ContourSlice::computeContourPolygons()
{
	_contourPolygonBottom.clear();
	_contourPolygonTop.clear();

	const int nbOriginalPointsContour = _originalContour.size();

	int i, index, originalMain1Index, originalMain2Index;
	qreal currentDistanceMain1, currentDistanceMain2;

	// S'il y a deux points dominants principaux
	if ( _leftMainDominantPointsIndex != -1 && _rightMainDominantPointsIndex != -1 )
	{
		const iCoord2D &leftMainPoint = leftMainDominantPoint();
		const iCoord2D &rightMainPoint = rightMainDominantPoint();

		// Calcul des point du contour original les plus proches des points dominants principaux
		// et de la boite englobante du contour original
		currentDistanceMain1 = leftMainDominantPoint().euclideanDistance(_originalContour[0]);
		currentDistanceMain2 = rightMainDominantPoint().euclideanDistance(_originalContour[0]);
		originalMain1Index = originalMain2Index = 0;
		for ( index=1 ; index<nbOriginalPointsContour ; ++index )
		{
			const iCoord2D &currentContourPoint = _originalContour[index];
			if ( leftMainPoint.euclideanDistance(currentContourPoint) < currentDistanceMain1 )
			{
				currentDistanceMain1 = leftMainPoint.euclideanDistance(currentContourPoint);
				originalMain1Index = index;
			}
			if ( rightMainPoint.euclideanDistance(currentContourPoint) <= currentDistanceMain2 )
			{
				currentDistanceMain2 = rightMainPoint.euclideanDistance(currentContourPoint);
				originalMain2Index = index;
			}
		}

		// Création du polygone qui servira à tester l'appartenance d'un pixel en dessous de la droite des deux points dominants principaux au noeud.
		// Ce polygone est constitué :
		//     - des points compris entre le premier point du contour initial (non lissé) et le plus proche du premier point dominant
		//     - du point le plus proche du premier point dominant
		//     - du point le plus proche du second point dominant
		//     - des points compris entre le point du contour initial le plus proche du second point dominant et le dernier points du contour initial.
		for ( i=0 ; i<=originalMain1Index ; ++i ) _contourPolygonBottom << QPoint(_originalContour[i].x,_originalContour[i].y);
		_contourPolygonBottom << QPoint(leftMainPoint.x,leftMainPoint.y)
							 << QPoint(rightMainPoint.x,rightMainPoint.y);
		for ( i=originalMain2Index ; i<nbOriginalPointsContour ; ++i ) _contourPolygonBottom << QPoint(_originalContour[i].x,_originalContour[i].y);
		_contourPolygonBottom << QPoint(_originalContour[0].x,_originalContour[0].y);

		// Création du polygone qui servira à tester l'appartenance d'un pixel au dessus de la droite des deux points dominants principaux au noeud.
		// Ce polygone est constitué :
		//     - du point le plus proche du premier point dominant
		//     - des points du contour initial (non lissé) situés entre le point le plus proche du premier point dominant et le point le plus proche du second point dominant
		//     - du point le plus proche du second point dominant
		_contourPolygonTop << QPoint(leftMainPoint.x,leftMainPoint.y);
		for ( i=originalMain1Index ; i<=originalMain2Index ; ++i ) _contourPolygonTop << QPoint(_originalContour[i].x,_originalContour[i].y);
		_contourPolygonTop << QPoint(rightMainPoint.x,rightMainPoint.y)
						  << QPoint(leftMainPoint.x,leftMainPoint.y);
	}
	else
	{
		// Création du polygone qui servira à tester l'appartenance d'un pixel au noeud.
		// Ce polygone est constitué des pixels du contour initial (non lissé)
		for ( i=0 ; i<nbOriginalPointsContour ; ++i )
		{
			_contourPolygonBottom << QPoint(_originalContour[i].x,_originalContour[i].y);
		}
		if ( nbOriginalPointsContour ) _contourPolygonBottom << QPoint(_originalContour[0].x,_originalContour[0].y);
	}
}

void ContourSlice::updateSlice( const Slice &initialSlice, Slice &resultSlice, const uiCoord2D &sliceCenter, const int &intensityThreshold )
{
	const int width = initialSlice.n_cols;
	const int height = initialSlice.n_rows;
	const int nbOriginalPointsContour = _originalContour.size();

	resultSlice.fill(0);

	const iCoord2D &leftMainPoint = leftMainDominantPoint();
	const iCoord2D &rightMainPoint = rightMainDominantPoint();
	const rCoord2D &leftSupportPoint = leftMainSupportPoint();
	const rCoord2D &rightSupportPoint = rightMainSupportPoint();

	const qreal daMain1Main2 = leftMainPoint.y - rightMainPoint.y;
	const qreal dbMain1Main2 = rightMainPoint.x - leftMainPoint.x;
	const qreal dcMain1Main2 = daMain1Main2*leftMainPoint.x + dbMain1Main2*leftMainPoint.y;
	const bool supToMain1Main2 = ( daMain1Main2*sliceCenter.x + dbMain1Main2*sliceCenter.y ) > dcMain1Main2;

	const qreal daMain1Support1 = leftMainPoint.y - leftSupportPoint.y;
	const qreal dbMain1Support1 = leftSupportPoint.x - leftMainPoint.x;
	const qreal dcMain1Support1 = daMain1Support1*leftMainPoint.x + dbMain1Support1*leftMainPoint.y;
	const bool supToMain1Support1 = ( daMain1Support1*rightMainPoint.x + dbMain1Support1*rightMainPoint.y ) > dcMain1Support1;

	const qreal daMain2Support2 = rightMainPoint.y - rightSupportPoint.y;
	const qreal dbMain2Support2 = rightSupportPoint.x - rightMainPoint.x;
	const qreal dcMain2Support2 = daMain2Support2*rightMainPoint.x + dbMain2Support2*rightMainPoint.y;
	const bool supToMain2Support2 = ( daMain2Support2*leftMainPoint.x + dbMain2Support2*leftMainPoint.y ) > dcMain2Support2;

	const bool hasMainLeft = _leftMainDominantPointsIndex != -1;
	const bool hasMainRight = _rightMainDominantPointsIndex != -1;

	int i,j;

	// S'il y a deux points dominants principaux
	if ( hasMainLeft && hasMainRight )
	{
		// Ajout des pixel
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( initialSlice.at(j,i) > intensityThreshold &&
					 ( _contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill) ||
					   (
						   ((daMain1Main2*i+dbMain1Main2*j <= dcMain1Main2) == supToMain1Main2) &&
						   ((daMain1Support1*i+dbMain1Support1*j >= dcMain1Support1) == supToMain1Support1) &&
						   ((daMain2Support2*i+dbMain2Support2*j >= dcMain2Support2) == supToMain2Support2) &&
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
	else
	{
		if ( hasMainLeft )
		{
			// Ajout des pixels du noeud du bon côté de chaque la droite de prolongement
			// et à l'intérieur du contour original
			const iCoord2D nextMain1( leftMainPoint.x - daMain1Support1, leftMainPoint.y - dbMain1Support1 );
			const bool rightToMain1Support1 = ( daMain1Support1*nextMain1.x + dbMain1Support1*nextMain1.y ) > dcMain1Support1;
			for ( j=0 ; j<height ; ++j )
			{
				for ( i=0 ; i<width ; ++i )
				{
					if ( initialSlice.at(j,i) > intensityThreshold
						 && ((daMain1Support1*i+dbMain1Support1*j > dcMain1Support1) == rightToMain1Support1)
						 && _contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill) )
					{
						resultSlice.at(j,i) = 1;
					}
				}
			}
		}
		else if ( hasMainRight )
		{
			// Ajout des pixels du noeud du bon côté de chaque la droite de prolongement
			// et à l'intérieur du contour original
			const iCoord2D nextMain2( rightMainPoint.x + daMain2Support2, rightMainPoint.y + dbMain2Support2 );
			const bool leftToMain2Support2 = ( daMain2Support2*nextMain2.x + dbMain2Support2*nextMain2.y ) > dcMain2Support2;
			for ( j=0 ; j<height ; ++j )
			{
				for ( i=0 ; i<width ; ++i )
				{
					if ( initialSlice.at(j,i) > intensityThreshold
						 && ((daMain2Support2*i+dbMain2Support2*j > dcMain2Support2) == leftToMain2Support2)
						 && _contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill) )
					{
						resultSlice.at(j,i) = 1;
					}
				}
			}
		}
		else if ( nbOriginalPointsContour > 3 )
		{
			// Sinon on ajoute la composante en entier
			for ( j=0 ; j<height ; ++j )
			{
				for ( i=0 ; i<width ; ++i )
				{
					if ( initialSlice.at(j,i) > intensityThreshold && _contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill) )
					{
						resultSlice.at(j,i) = 1;
					}
				}
			}
		}
	}
}
