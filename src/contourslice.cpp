#include "inc/contourslice.h"

#include "inc/billon.h"
#include "inc/billonalgorithms.h"

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

const QVector<iCoord2D> &ContourSlice::contourPoints() const
{
	return _datasContourPoints;
}

const iCoord2D &ContourSlice::dominantPoint( const uint &index ) const
{
	Q_ASSERT_X( index<static_cast<uint>(_datasDominantPointsIndex.size()), "Histogram::mainDominantPoint", "Le point dominants demandé n'existe pas" );
	return _datasContourPoints[_datasDominantPointsIndex[index]];
}

const QVector<uint> &ContourSlice::dominantPointIndex() const
{
	return _datasDominantPointsIndex;
}

const iCoord2D &ContourSlice::leftMainDominantPoint() const
{
	return _datasLeftMainDominantPointsIndex != -1 ? _datasContourPoints[_datasLeftMainDominantPointsIndex] : invalidICoord2D;
}

const iCoord2D &ContourSlice::rightMainDominantPoint() const
{
	return _datasRightMainDominantPointsIndex != -1 ? _datasContourPoints[_datasRightMainDominantPointsIndex] : invalidICoord2D;
}

const int &ContourSlice::leftMainDominantPointIndex() const
{
	return _datasLeftMainDominantPointsIndex;
}

const int &ContourSlice::rightMainDominantPointIndex() const
{
	return _datasRightMainDominantPointsIndex;
}

const rCoord2D &ContourSlice::leftMainSupportPoint() const
{
	return _datasLeftMainSupportPoint;
}

const rCoord2D &ContourSlice::rightMainSupportPoint() const
{
	return _datasRightMainSupportPoint;
}

/**********************************
 * Public setters
 **********************************/

void ContourSlice::compute( Slice &resultSlice, const Slice &initialSlice, const iCoord2D &sliceCenter, const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius, const iCoord2D &startPoint )
{
	clear();
	extractContourPointsAndDominantPoints( initialSlice, sliceCenter, intensityThreshold, blurredSegmentThickness, smoothingRadius, startPoint );
	computeMainDominantPoints(sliceCenter);
	computeContourPolygons();
	updateSlice( initialSlice, resultSlice, sliceCenter, intensityThreshold );
}

void ContourSlice::computeOldMethod( Slice &resultSlice, const Slice &initialSlice, const iCoord2D &sliceCenter, const int &intensityThreshold, const int &smoothingRadius, const iCoord2D &startPoint )
{
	clear();

	_datasOriginalContourPoints = BillonAlgorithms::extractContour( initialSlice, sliceCenter, intensityThreshold, startPoint );
	_datasContourPoints = _datasOriginalContourPoints;
	smoothCurve( _datasContourPoints, smoothingRadius );

	const int width = initialSlice.n_cols;
	const int height = initialSlice.n_rows;
	resultSlice.resize(height,width);
	resultSlice.fill(0);

	const int nbOriginalPointsContour = _datasContourPoints.size();

	int i,j, minXIndex, maxXIndex, minYIndex, maxYIndex;
	QPolygon contourPolygonBottom;

	// Sinon on ajoute la composante en entier
	minXIndex = maxXIndex = _datasContourPoints[0].x;
	minYIndex = maxYIndex = _datasContourPoints[0].y;
	for ( i=0 ; i<nbOriginalPointsContour ; ++i )
	{
		contourPolygonBottom << QPoint(_datasContourPoints[i].x,_datasContourPoints[i].y);
		minXIndex = qMin(minXIndex,_datasContourPoints[i].x);
		maxXIndex = qMax(maxXIndex,_datasContourPoints[i].x);
		minYIndex = qMin(minYIndex,_datasContourPoints[i].y);
		maxYIndex = qMax(maxYIndex,_datasContourPoints[i].y);
	}
	contourPolygonBottom << QPoint(_datasContourPoints[0].x,_datasContourPoints[0].y);

	for ( j = minYIndex ; j<maxYIndex ; ++j )
	{
		for ( i = minXIndex ; i<maxXIndex ; ++i )
		{
			if ( initialSlice.at(j,i) > intensityThreshold && contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill) )
			{
				resultSlice.at(j,i) = 1;
			}
		}
	}
}

void ContourSlice::draw( QImage &image, const int &cursorPosition ) const
{
	QPainter painter(&image);
	int i;

	const int nbContourPoints = _datasContourPoints.size();
	if ( nbContourPoints > 0 )
	{
		// Dessin des points de contours
		painter.setPen(Qt::blue);
		for ( i=0 ; i<nbContourPoints ; ++i )
		{
			painter.drawPoint(_datasContourPoints[i].x,_datasContourPoints[i].y);
		}

		const int nbDominantPoints = _datasDominantPointsIndex.size();
		if ( nbDominantPoints > 0 )
		{
			// Dessin des points dominants
			painter.setPen(Qt::green);
			for ( i=0 ; i<nbDominantPoints ; ++i )
			{
				painter.drawEllipse(_datasContourPoints[_datasDominantPointsIndex[i]].x-2,_datasContourPoints[_datasDominantPointsIndex[i]].y-2,4,4);
			}

			// Dessin des points dominants principaux
			const iCoord2D &leftMainPoint = leftMainDominantPoint();
			const iCoord2D &rightMainPoint = rightMainDominantPoint();

			painter.setPen(Qt::red);
			if ( leftMainDominantPointIndex() != -1 ) painter.drawEllipse(leftMainPoint.x-3,leftMainPoint.y-3,6,6);
			if ( rightMainDominantPointIndex() != -1 ) painter.drawEllipse(rightMainPoint.x-3,rightMainPoint.y-3,6,6);

			// Dessins des droites de coupe issues des points de support dominants principaux
			const rCoord2D &leftSupportPoint = leftMainSupportPoint();
			const rCoord2D &rightSupportPoint = rightMainSupportPoint();

			painter.setPen(Qt::gray);
			qreal a, b;
			if ( leftSupportPoint.x != -1 || leftSupportPoint.y != -1 )
			{
				a = ( leftMainPoint.y - leftSupportPoint.y ) / static_cast<qreal>( leftMainPoint.x - leftSupportPoint.x );
				b = ( leftMainPoint.y * leftSupportPoint.x - leftMainPoint.x * leftSupportPoint.y ) / static_cast<qreal>( leftSupportPoint.x - leftMainPoint.x );
				//				painter.drawLine(0, b, image.width(), a * image.width() + b );
				if ( leftSupportPoint.x < leftMainPoint.x )
				{
					painter.drawLine(leftMainPoint.x, leftMainPoint.y, image.width(), a * image.width() + b );
				}
				else
				{
					painter.drawLine(leftMainPoint.x, leftMainPoint.y, 0., b );
				}
			}
			if ( rightSupportPoint.x != -1 || rightSupportPoint.y != -1 )
			{
				a = ( rightMainPoint.y - rightSupportPoint.y ) / static_cast<qreal>( rightMainPoint.x - rightSupportPoint.x );
				b = ( rightMainPoint.y * rightSupportPoint.x - rightMainPoint.x * rightSupportPoint.y ) / static_cast<qreal>( rightSupportPoint.x - rightMainPoint.x );
				//				painter.drawLine(0, b, image.width(), a * image.width() + b );
				if ( rightSupportPoint.x < rightMainPoint.x )
				{
					painter.drawLine(rightMainPoint.x, rightMainPoint.y, image.width(), a * image.width() + b );
				}
				else
				{
					painter.drawLine(rightMainPoint.x, rightMainPoint.y, 0., b );
				}
			}
		}

		// Dessin du point de contour initial (également point dominant initial)
		painter.setPen(Qt::red);
		painter.drawEllipse(_datasContourPoints[0].x-1,_datasContourPoints[0].y-1,2,2);

		// Dessin du curseur
		if ( cursorPosition >= 0 )
		{
			painter.setPen(Qt::cyan);
			painter.drawEllipse(_datasContourPoints[cursorPosition].x-1,_datasContourPoints[cursorPosition].y-1,2,2);
		}
	}
}

/**********************************
 * Private setters
 **********************************/

void ContourSlice::clear()
{
	_datasContourPoints.clear();
	_datasOriginalContourPoints.clear();
	_datasDominantPointsIndex.clear();
	_datasLeftMainDominantPointsIndex = _datasRightMainDominantPointsIndex = -1;
	_datasLeftMainSupportPoint = _datasRightMainSupportPoint = rCoord2D(-1,-1);

	_contourPolygonBottom.clear();
	_contourPolygonTop.clear();
}

void ContourSlice::smoothCurve( QVector<iCoord2D> &curve, int smoothingRadius )
{
	if ( smoothingRadius > 0 )
	{
		const int nbPoints = curve.size();
		smoothingRadius = qMin(smoothingRadius,nbPoints);
		const int smoothingDiameter = 2*smoothingRadius+1;
		const qreal qSmoothingDiameter = smoothingDiameter;
		if ( nbPoints > smoothingDiameter )
		{
			QVector<iCoord2D> datasContourForSmoothing;
			datasContourForSmoothing.reserve(nbPoints+2*smoothingRadius);
			datasContourForSmoothing << curve.mid(nbPoints-smoothingRadius) <<  curve << curve.mid(0,smoothingRadius);

			int i, smoothingValueX, smoothingValueY;
			smoothingValueX = smoothingValueY = 0;

			for ( i=0 ; i<smoothingDiameter ; ++i )
			{
				smoothingValueX += datasContourForSmoothing[i].x;
				smoothingValueY += datasContourForSmoothing[i].y;
			}

			curve.clear();
			curve << iCoord2D( smoothingValueX / qSmoothingDiameter, smoothingValueY / qSmoothingDiameter );
			iCoord2D currentCoord;
			for ( int i=1 ; i<nbPoints ; ++i )
			{
				smoothingValueX = smoothingValueX - datasContourForSmoothing[i-1].x + datasContourForSmoothing[i+smoothingDiameter-1].x;
				smoothingValueY = smoothingValueY - datasContourForSmoothing[i-1].y + datasContourForSmoothing[i+smoothingDiameter-1].y;
				currentCoord.x = smoothingValueX / qSmoothingDiameter;
				currentCoord.y = smoothingValueY / qSmoothingDiameter;
				if ( curve.last() != currentCoord ) curve << currentCoord;
			}
		}
	}
}

void ContourSlice::extractContourPointsAndDominantPoints( const Slice &initialSlice, const iCoord2D &sliceCenter, const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius, const iCoord2D &startPoint )
{
	_datasOriginalContourPoints = BillonAlgorithms::extractContour( initialSlice, sliceCenter, intensityThreshold, startPoint );
	_datasContourPoints = _datasOriginalContourPoints;
	smoothCurve( _datasContourPoints, smoothingRadius );

	int nbPoints = _datasContourPoints.size();

	// Ecriture des données de points de contours
	QTemporaryFile fileContours("TKDetection_XXXXXX.ctr");
	if ( !fileContours.open() )
	{
		qDebug() << QObject::tr("ERREUR : Impossible de créer le ficher de contours %1.").arg(fileContours.fileName());
		return;
	}

	QTextStream streamContours(&fileContours);
	for ( int i=0 ; i<nbPoints ; ++i )
	{
		streamContours << _datasContourPoints.at(i).x << " " << _datasContourPoints.at(i).y << endl;
	}
	fileContours.close();

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
		nbPoints = streamDominantPoints.readLine().toInt();
		_datasDominantPointsIndex.resize(nbPoints);
		for ( int i=0 ; i<nbPoints ; ++i )
		{
			streamDominantPoints >> _datasDominantPointsIndex[i];
		}

		fileDominantPoint.close();
	}
}

void ContourSlice::computeMainDominantPoints( const iCoord2D &sliceCenter )
{
	// On détermine les points dominants principaux en comparant la distance à la moelle d'un point dominant
	// à la distance à la moelle des deux points dominants précdents et du point dominant suivant.
	int nbDominantPoints, index, oldIndex;
	qreal distanceP0Pm2, distanceP0Pm1, distanceP0Pp1, distanceP0Pp2, distanceP0Px;
	bool angleOk;

	nbDominantPoints = _datasDominantPointsIndex.size();

	if ( nbDominantPoints > 3 )
	{
		_datasDominantPointsIndex << _datasDominantPointsIndex[0];
		nbDominantPoints++;

		// Point dominant dans le sens du contour
		index = 1;
		distanceP0Pm1 = sliceCenter.euclideanDistance(dominantPoint(index-1));
		distanceP0Px = sliceCenter.euclideanDistance(dominantPoint(index));
		distanceP0Pp1 = sliceCenter.euclideanDistance(dominantPoint(index+1));
		distanceP0Pp2 = sliceCenter.euclideanDistance(dominantPoint(index+2));
		angleOk = (distanceP0Pp1-distanceP0Px>10) || (distanceP0Pm1<distanceP0Pp1 && (distanceP0Px<distanceP0Pp1 || distanceP0Px<distanceP0Pp2) );
		if ( angleOk ) ++index;
		while ( angleOk && index < nbDominantPoints-1 )
		{
			distanceP0Pm2 = distanceP0Pm1;
			distanceP0Pm1 = distanceP0Px;
			distanceP0Px = distanceP0Pp1;
			distanceP0Pp1 = sliceCenter.euclideanDistance(dominantPoint(index+1));
			angleOk = (distanceP0Pp1-distanceP0Px>10) || (distanceP0Pm2<distanceP0Pm1 && distanceP0Pm1<distanceP0Pp1 && distanceP0Px<distanceP0Pp1);
			if ( angleOk ) ++index;
		}
		oldIndex = index;
		// Si le point dominant trouvé est correct
		if ( index < nbDominantPoints-1 )
		{
			_datasLeftMainDominantPointsIndex = _datasDominantPointsIndex[index];

			// Calcul du point dominant support du point dominant principal
			_datasLeftMainSupportPoint = rCoord2D(0.,0.);
			while ( index >= 0 )
			{
				_datasLeftMainSupportPoint.x += dominantPoint(index).x;
				_datasLeftMainSupportPoint.y += dominantPoint(index).y;
				index--;
			}
			_datasLeftMainSupportPoint.x /= (oldIndex + 1);
			_datasLeftMainSupportPoint.y /= (oldIndex + 1);
		}

		// Point dominant dans le sens contraire du contour
		index = nbDominantPoints-2;
		distanceP0Pm1 = sliceCenter.euclideanDistance(dominantPoint(index+1));
		distanceP0Px = sliceCenter.euclideanDistance(dominantPoint(index));
		distanceP0Pp1 = sliceCenter.euclideanDistance(dominantPoint(index-1));
		distanceP0Pp2 = sliceCenter.euclideanDistance(dominantPoint(index-2));
		angleOk = (distanceP0Pp1-distanceP0Px>10) || (distanceP0Pm1<distanceP0Pp1 && (distanceP0Px<distanceP0Pp1 || distanceP0Px<distanceP0Pp2) );
		if ( angleOk ) index--;
		while ( angleOk && index > oldIndex )
		{
			distanceP0Pm2 = distanceP0Pm1;
			distanceP0Pm1 = distanceP0Px;
			distanceP0Px = distanceP0Pp1;
			distanceP0Pp1 = sliceCenter.euclideanDistance(dominantPoint(index-1));
			angleOk = (distanceP0Pp1-distanceP0Px>10) || (distanceP0Pm2<distanceP0Pm1 && distanceP0Pm1<distanceP0Pp1 && distanceP0Px<distanceP0Pp1);
			if ( angleOk ) index--;
		}
		// Si le point dominant trouvé est correct
		if ( index > oldIndex )
		{
			oldIndex = index;
			_datasRightMainDominantPointsIndex = _datasDominantPointsIndex[index];

			// Calcul du point dominant support du point dominant principal
			_datasRightMainSupportPoint = rCoord2D(0.,0.);
			while ( index != 1 )
			{
				_datasRightMainSupportPoint.x += dominantPoint(index).x;
				_datasRightMainSupportPoint.y += dominantPoint(index).y;
				index = (index+1)%nbDominantPoints;
			}
			_datasRightMainSupportPoint.x /= (nbDominantPoints - oldIndex + 1);
			_datasRightMainSupportPoint.y /= (nbDominantPoints - oldIndex + 1);
		}

		_datasDominantPointsIndex.remove(nbDominantPoints-1);
	}
}

void ContourSlice::computeContourPolygons()
{
	const int nbOriginalPointsContour = _datasOriginalContourPoints.size();

	const iCoord2D &leftMainPoint = leftMainDominantPoint();
	const iCoord2D &rightMainPoint = rightMainDominantPoint();

	const bool hasMain1 = _datasLeftMainDominantPointsIndex != -1;
	const bool hasMain2 = _datasRightMainDominantPointsIndex != -1;

	int i, index, originalMain1Index, originalMain2Index, minXIndex, maxXIndex, minYIndex, maxYIndex;
	qreal currentDistanceMain1, currentDistanceMain2;

	// S'il y a deux points dominants principaux
	if ( hasMain1 && hasMain2 )
	{
		// Calcul des point du contour original les plus proches des points dominants principaux
		// et de la boite englobante du contour original
		currentDistanceMain1 = leftMainDominantPoint().euclideanDistance(_datasOriginalContourPoints[0]);
		currentDistanceMain2 = rightMainDominantPoint().euclideanDistance(_datasOriginalContourPoints[0]);
		originalMain1Index = originalMain2Index = 0;
		minXIndex = maxXIndex = _datasOriginalContourPoints[0].x;
		minYIndex = maxYIndex = _datasOriginalContourPoints[0].y;
		for ( index=1 ; index<nbOriginalPointsContour ; ++index )
		{
			const iCoord2D &currentContourPoint = _datasOriginalContourPoints[index];
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
			minXIndex = qMin(minXIndex,currentContourPoint.x);
			maxXIndex = qMax(maxXIndex,currentContourPoint.x);
			minYIndex = qMin(minYIndex,currentContourPoint.y);
			maxYIndex = qMax(maxYIndex,currentContourPoint.y);
		}

		// Création du polygone qui servira à tester l'appartenance d'un pixel en dessous de la droite des deux points dominants principaux au noeud.
		// Ce polygone est constitué :
		//     - des points compris entre le premier point du contour initial (non lissé) et le plus proche du premier point dominant
		//     - du point le plus proche du premier point dominant
		//     - du point le plus proche du second point dominant
		//     - des points compris entre le point du contour initial le plus proche du second point dominant et le dernier points du contour initial.
		for ( i=0 ; i<=originalMain1Index ; ++i ) _contourPolygonBottom << QPoint(_datasOriginalContourPoints[i].x,_datasOriginalContourPoints[i].y);
		_contourPolygonBottom << QPoint(leftMainPoint.x,leftMainPoint.y)
							 << QPoint(rightMainPoint.x,rightMainPoint.y);
		for ( i=originalMain2Index ; i<nbOriginalPointsContour ; ++i ) _contourPolygonBottom << QPoint(_datasOriginalContourPoints[i].x,_datasOriginalContourPoints[i].y);
		_contourPolygonBottom << QPoint(_datasOriginalContourPoints[0].x,_datasOriginalContourPoints[0].y);

		// Création du polygone qui servira à tester l'appartenance d'un pixel au dessus de la droite des deux points dominants principaux au noeud.
		// Ce polygone est constitué :
		//     - du point le plus proche du premier point dominant
		//     - des points du contour initial (non lissé) situés entre le point le plus proche du premier point dominant et le point le plus proche du second point dominant
		//     - du point le plus proche du second point dominant
		_contourPolygonTop << QPoint(leftMainPoint.x,leftMainPoint.y);
		for ( i=originalMain1Index ; i<=originalMain2Index ; ++i ) _contourPolygonTop << QPoint(_datasOriginalContourPoints[i].x,_datasOriginalContourPoints[i].y);
		_contourPolygonTop << QPoint(rightMainPoint.x,rightMainPoint.y)
						  << QPoint(leftMainPoint.x,leftMainPoint.y);
	}
	else
	{
		// Création du polygone qui servira à tester l'appartenance d'un pixel au noeud.
		// Ce polygone est constitué des pixels du contour initial (non lissé)
		minXIndex = maxXIndex = _datasOriginalContourPoints[0].x;
		minYIndex = maxYIndex = _datasOriginalContourPoints[0].y;
		for ( i=0 ; i<nbOriginalPointsContour ; ++i )
		{
			_contourPolygonBottom << QPoint(_datasOriginalContourPoints[i].x,_datasOriginalContourPoints[i].y);
			minXIndex = qMin(minXIndex,_datasOriginalContourPoints[i].x);
			maxXIndex = qMax(maxXIndex,_datasOriginalContourPoints[i].x);
			minYIndex = qMin(minYIndex,_datasOriginalContourPoints[i].y);
			maxYIndex = qMax(maxYIndex,_datasOriginalContourPoints[i].y);
		}
		_contourPolygonBottom << QPoint(_datasOriginalContourPoints[0].x,_datasOriginalContourPoints[0].y);
	}
}

void ContourSlice::updateSlice( const Slice &initialSlice, Slice &resultSlice, const iCoord2D &sliceCenter, const int &intensityThreshold )
{
	const int width = initialSlice.n_cols;
	const int height = initialSlice.n_rows;
	const int nbOriginalPointsContour = _datasOriginalContourPoints.size();

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

	const bool hasMainLeft = _datasLeftMainDominantPointsIndex != -1;
	const bool hasMainRight = _datasRightMainDominantPointsIndex != -1;

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
