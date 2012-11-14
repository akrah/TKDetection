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

const CurvatureHistogram &ContourSlice::curvatureHistogram() const
{
	return _curvatureHistogram;
}

const iCoord2D &ContourSlice::dominantPoint( const uint &index ) const
{
	Q_ASSERT_X( index<static_cast<uint>(_dominantPointsIndex.size()), "Histogram::mainDominantPoint", "Le point dominants demandé n'existe pas" );
	return _contour[_dominantPointsIndex[index]];
}

const QVector<uint> &ContourSlice::dominantPointIndex() const
{
	return _dominantPointsIndex;
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

void ContourSlice::compute( Slice &resultSlice, const Slice &initialSlice, const iCoord2D &sliceCenter, const int &intensityThreshold,
							const int &blurredSegmentThickness, const int &smoothingRadius, const int &curvatureWidth, const iCoord2D &startPoint )
{
	_contour.compute( initialSlice, sliceCenter, intensityThreshold, startPoint );
	_originalContour = _contour;
	_contour.smooth(smoothingRadius);

	_curvatureHistogram.construct( _contour, curvatureWidth  );

	computeDominantPoints( blurredSegmentThickness );
	computeMainDominantPoints();
	computeSupportsOfMainDominantPoints();
	computeContourPolygons();
	updateSlice( initialSlice, resultSlice, sliceCenter, intensityThreshold );
}

void ContourSlice::computeOldMethod( Slice &resultSlice, const Slice &initialSlice, const iCoord2D &sliceCenter, const int &intensityThreshold, const int &smoothingRadius, const int &curvatureWidth, const iCoord2D &startPoint )
{
	_contour.clear();
	_originalContour.clear();
	_curvatureHistogram.clear();
	_dominantPointsIndex.clear();
	_leftMainDominantPointsIndex = _rightMainDominantPointsIndex = -1;
	_leftMainSupportPoint = _rightMainSupportPoint = rCoord2D(-1,-1);
	_contourPolygonBottom.clear();
	_contourPolygonTop.clear();

	_contour.compute( initialSlice, sliceCenter, intensityThreshold, startPoint );
	_originalContour = _contour;
	_contour.smooth(smoothingRadius);

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

void ContourSlice::draw( QImage &image, const int &cursorPosition ) const
{
	QPainter painter(&image);
	int i;

	const int nbContourPoints = _contour.size();
	if ( nbContourPoints > 0 )
	{
		_contour.draw(image,cursorPosition);

		const int nbDominantPoints = _dominantPointsIndex.size();
		if ( nbDominantPoints > 0 )
		{
			// Dessin des points dominants
			painter.setPen(Qt::green);
			for ( i=0 ; i<nbDominantPoints ; ++i )
			{
				painter.drawEllipse(_contour[_dominantPointsIndex[i]].x-2,_contour[_dominantPointsIndex[i]].y-2,4,4);
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
		painter.drawEllipse(_contour[0].x-1,_contour[0].y-1,2,2);
	}
}

/**********************************
 * Private setters
 **********************************/

void ContourSlice::computeDominantPoints( const int &blurredSegmentThickness )
{
	_dominantPointsIndex.clear();

	int nbPoints = _contour.size();
	if ( nbPoints > 0 )
	{
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
			streamContours << _contour[i].x << " " << _contour[i].y << endl;
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
			streamDominantPoints >> nbPoints;
			_dominantPointsIndex.resize(nbPoints);
			for ( int i=0 ; i<nbPoints ; ++i )
			{
				streamDominantPoints >> _dominantPointsIndex[i];
			}

			fileDominantPoint.close();
		}
	}
}

void ContourSlice::computeMainDominantPoints()
{
	_leftMainDominantPointsIndex = _rightMainDominantPointsIndex = -1;

	int nbDominantPoints, index, firstIndex;

	nbDominantPoints = _dominantPointsIndex.size();
	if ( nbDominantPoints > 0 && _curvatureHistogram.size() == _contour.size() )
	{
		// Point dominant dans le sens du contour
		index = 0;
		while ( index<nbDominantPoints && _curvatureHistogram[_dominantPointsIndex[index]] > 0 ) ++index;
		firstIndex = index;
		// Si le point dominant trouvé est correct
		if ( index<nbDominantPoints ) _leftMainDominantPointsIndex = _dominantPointsIndex[index];

		// Point dominant dans le sens contraire du contour
		index = nbDominantPoints-1;
		while ( index>firstIndex && _curvatureHistogram[_dominantPointsIndex[index]] > 0 ) --index;
		// Si le point dominant trouvé est correct
		if ( index>firstIndex ) _rightMainDominantPointsIndex = _dominantPointsIndex[index];
	}
}

void ContourSlice::computeSupportsOfMainDominantPoints()
{
	_leftMainSupportPoint = _rightMainSupportPoint = rCoord2D(-1,-1);

	int index;

	// Support du MDP gauche
	index = _leftMainDominantPointsIndex;
	if ( index != -1 )
	{
		_leftMainSupportPoint.x = _leftMainSupportPoint.y = 0.;
		while ( index >= 0 )
		{
			_leftMainSupportPoint.x += dominantPoint(index).x;
			_leftMainSupportPoint.y += dominantPoint(index).y;
			--index;
		}
		_leftMainSupportPoint.x /= (_leftMainDominantPointsIndex + 1);
		_leftMainSupportPoint.y /= (_leftMainDominantPointsIndex + 1);
	}

	// Support du MDP droit
	index = _rightMainDominantPointsIndex;
	if ( index != -1 )
	{
		_rightMainSupportPoint = dominantPoint(0);
		int nbDominantPoints = _dominantPointsIndex.size();
		while ( index < nbDominantPoints )
		{
			_rightMainSupportPoint.x += dominantPoint(index).x;
			_rightMainSupportPoint.y += dominantPoint(index).y;
			++index;
		}
		_rightMainSupportPoint.x /= (nbDominantPoints - _rightMainDominantPointsIndex + 1);
		_rightMainSupportPoint.y /= (nbDominantPoints - _rightMainDominantPointsIndex + 1);
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
		_contourPolygonBottom << QPoint(_originalContour[0].x,_originalContour[0].y);
	}
}

void ContourSlice::updateSlice( const Slice &initialSlice, Slice &resultSlice, const iCoord2D &sliceCenter, const int &intensityThreshold )
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
