#include "inc/contourcurve.h"

#include "inc/billon.h"
#include "inc/marrow.h"

#include <QProcess>
#include <QTemporaryFile>
#include <QPainter>

ContourCurve::ContourCurve() : _smoothingRadius(5)
{
}

ContourCurve::~ContourCurve()
{
}

const QVector<iCoord2D> &ContourCurve::contourPoints() const
{
	return _datasContourPoints;
}

const QVector<iCoord2D> &ContourCurve::dominantPoints() const
{
	return _datasDominantPoints;
}

const QVector<iCoord2D> &ContourCurve::mainDominantPoints() const
{
	return _datasMainDominantPoints;
}

int ContourCurve::indexOfMainPoint( const int &number ) const
{
	return number>-1&&number<2 ? _datasIndexMainDominantPoints[number] : -1;
}

const QVector<iCoord2D> &ContourCurve::mainSupportPoints() const
{
	return _datasMainSupportPoints;
}

void ContourCurve::setSmoothingRadius( const int &radius )
{
	_smoothingRadius = qAbs(radius);
}

void ContourCurve::constructCurve( const Billon &billon, const iCoord2D &billonCenter, const int &sliceNumber, const int &componentNumber, const int &blurredSegmentThickness, const iCoord2D &startPoint )
{
	_datasOriginalContourPoints.clear();
	_datasOriginalContourPoints = billon.extractContour( billonCenter, sliceNumber, componentNumber, startPoint );
	_datasContourPoints.clear();
	_datasContourPoints = _datasOriginalContourPoints;
	smoothCurve(_datasContourPoints,_smoothingRadius);

	_datasDominantPoints.clear();

	_datasMainDominantPoints.clear();
	_datasMainDominantPoints.fill(iCoord2D(-1,-1),2);
	_datasIndexMainDominantPoints.fill(-1,2);

	_datasMainSupportPoints.fill(iCoord2D(-1,-1),2);

	int nbPoints = _datasContourPoints.size();
	if ( nbPoints > 7 )
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
			int x, y;
			for ( int i=0 ; i<nbPoints ; ++i )
			{
				streamDominantPoints >> x >> y;
				_datasDominantPoints << iCoord2D(x,y);
			}
			fileDominantPoint.close();

			if ( nbPoints > 2 )
			{
				// On détermine les points dominnants principaux en analysant l'angle formé par 3 points dominants consécutifs.
				// Les angles sont analysés successivement à partir du premier point de contour (le plus proche de la moelle) et dans chacune des deux direction du contour.
				// On s'arrête :
				//    - dès que l'angle est supérieur à 3*PI/4 (+PI/4 dans le sens contraire de la convexité du noeud)
				//      ou que l'angle est inférieur à PI/4 (+3*PI/4 dans le sens de la convexité du noeud)
				//    - dès que la somme de la valeur absolue de deux angles consécutifs de même convexité est inférieure à 180.
				// Ces deux conditions sont vérifiées à partir du premier point dominant à plus de 20 pixels du point de départ
				const int padding = qMin(billon.n_cols,billon.n_rows)/20;
				QVector<iCoord2D> dominantPoints;
				dominantPoints.reserve(nbPoints+1);
				dominantPoints << _datasDominantPoints << _datasDominantPoints[0];
				int index, oldIndex;
				qreal angle, oldAngle;
				index = 1;
				angle = dominantPoints[index].angle(dominantPoints[index-1],dominantPoints[index+1]);
				while ( index < nbPoints )
				{
					oldAngle = angle!=0?180:angle;
					angle = dominantPoints[index].angle(dominantPoints[index-1],dominantPoints[index+1]);
					if ( ((angle < -PI_ON_FOUR || angle  > THREE_PI_ON_FOUR) && (angle/oldAngle<0 || qAbs(oldAngle)+qAbs(angle)>PI)) || dominantPoints[index].distance(dominantPoints[0]) < 20 ) index++;
					else break;
				}
				oldIndex = index;
				if ( index < nbPoints )
				{
					_datasMainDominantPoints[0] = dominantPoints[index];
					_datasIndexMainDominantPoints[0] = index;

					while ( index > 0 && _datasDominantPoints[index].distance(_datasMainDominantPoints[0]) < padding ) index--;
					_datasMainSupportPoints[0] = _datasDominantPoints[index];

				}
				index = nbPoints-1;
				angle = dominantPoints[index].angle(dominantPoints[index+1],dominantPoints[index-1]);
				while ( index > oldIndex )
				{
					oldAngle = angle;
					angle = dominantPoints[index].angle(dominantPoints[index+1],dominantPoints[index-1]);
					if ( ((angle > PI_ON_FOUR || angle < -THREE_PI_ON_FOUR) && (angle/oldAngle<0 || qAbs(oldAngle)+qAbs(angle)>PI)) || dominantPoints[index].distance(dominantPoints[0]) < 20 ) index--;
					else break;
				}
				if ( index > oldIndex )
				{
					_datasMainDominantPoints[1] = dominantPoints[index];
					_datasIndexMainDominantPoints[1] = index;

					index = (index+1)%nbPoints;
					while ( index>0 && index<nbPoints-1 && _datasDominantPoints[index].distance(_datasMainDominantPoints[1]) < padding ) index++;
					_datasMainSupportPoints[1] = _datasDominantPoints[index%nbPoints];
				}
			}
		}
	}
}

void ContourCurve::smoothCurve( QVector<iCoord2D> &contour, int smoothingRadius )
{
	const int nbPoints = contour.size();
	smoothingRadius = qMin(smoothingRadius,nbPoints);
	const int smoothingDiameter = 2*smoothingRadius+1;
	if ( nbPoints > smoothingDiameter )
	{
		QVector<iCoord2D> datasContourForSmoothing;
		datasContourForSmoothing.reserve(nbPoints+2*smoothingRadius);
		datasContourForSmoothing << contour.mid(nbPoints-smoothingRadius) <<  contour << contour.mid(0,smoothingRadius);

		int i, smoothingValueX, smoothingValueY;
		smoothingValueX = smoothingValueY = 0;

		for ( i=0 ; i<smoothingDiameter ; ++i )
		{
			smoothingValueX += datasContourForSmoothing[i].x;
			smoothingValueY += datasContourForSmoothing[i].y;
		}
		contour[0].x = smoothingValueX / static_cast<qreal>(smoothingDiameter);
		contour[0].y = smoothingValueY / static_cast<qreal>(smoothingDiameter);
		for ( int i=1 ; i<nbPoints ; ++i )
		{
			smoothingValueX = smoothingValueX - datasContourForSmoothing[i-1].x + datasContourForSmoothing[i+smoothingDiameter-1].x;
			smoothingValueY = smoothingValueY - datasContourForSmoothing[i-1].y + datasContourForSmoothing[i+smoothingDiameter-1].y;
			contour[i].x = smoothingValueX / static_cast<qreal>(smoothingDiameter);
			contour[i].y = smoothingValueY / static_cast<qreal>(smoothingDiameter);
		}
	}
}

void ContourCurve::draw( QImage &image ) const
{
	QPainter painter(&image);
	int i;

	const int nbContourPoints = _datasContourPoints.size();
	if ( nbContourPoints > 0 )
	{
		painter.setPen(Qt::blue);
		for ( i=0 ; i<nbContourPoints ; ++i ) {
			painter.drawPoint(_datasContourPoints[i].x,_datasContourPoints[i].y);
		}

		const int nbDominantPoints = _datasDominantPoints.size();
		if ( nbDominantPoints > 0 )
		{
			painter.setPen(Qt::green);
			for ( i=0 ; i<nbDominantPoints ; ++i ) {
				painter.drawEllipse(_datasDominantPoints[i].x-2,_datasDominantPoints[i].y-2,4,4);
			}


			const iCoord2D &mainPoint1 = _datasMainDominantPoints[0];
			const iCoord2D &mainPoint2 = _datasMainDominantPoints[1];

			painter.setPen(Qt::red);
			if ( _datasIndexMainDominantPoints[0] != -1 ) painter.drawEllipse(mainPoint1.x-3,mainPoint1.y-3,6,6);
			painter.setPen(Qt::green);
			if ( _datasIndexMainDominantPoints[1] != -1 ) painter.drawEllipse(mainPoint2.x-3,mainPoint2.y-3,6,6);

			painter.setPen(Qt::gray);

			qreal a, b;

			const iCoord2D &supportMain1 = _datasMainSupportPoints[0];
			if ( supportMain1.x != -1 || supportMain1.y != -1 )
			{
				a = ( mainPoint1.y - supportMain1.y ) / static_cast<qreal>( mainPoint1.x - supportMain1.x );
				b = ( mainPoint1.y * supportMain1.x - mainPoint1.x * supportMain1.y ) / static_cast<qreal>( supportMain1.x - mainPoint1.x );
				painter.drawLine(0, b, image.width(), a * image.width() + b );
//				if ( supportMain1.x < mainPoint1.x )
//				{
//					painter.drawLine(mainPoint1.x, mainPoint1.y, image.width(), a * image.width() + b );
//				}
//				else
//				{
//					painter.drawLine(mainPoint1.x, mainPoint1.y, 0., b );
//				}
			}
			const iCoord2D &supportMain2 = _datasMainSupportPoints[1];
			if ( supportMain2.x != -1 || supportMain2.y != -1 )
			{
				a = ( mainPoint2.y - supportMain2.y ) / static_cast<qreal>( mainPoint2.x - supportMain2.x );
				b = ( mainPoint2.y * supportMain2.x - mainPoint2.x * supportMain2.y ) / static_cast<qreal>( supportMain2.x - mainPoint2.x );
				painter.drawLine(0, b, image.width(), a * image.width() + b );
//				if ( supportMain2.x < mainPoint2.x )
//				{
//					painter.drawLine(mainPoint2.x, mainPoint2.y, image.width(), a * image.width() + b );
//				}
//				else
//				{
//					painter.drawLine(mainPoint2.x, mainPoint2.y, 0., b );
//				}
			}
		}

		painter.setPen(Qt::red);
		painter.drawEllipse(_datasContourPoints[0].x-1,_datasContourPoints[0].y-1,2,2);
	}
}


void ContourCurve::drawRestrictedComponent( QImage &image, const arma::Slice &slice, const iCoord2D &marrow ) const
{
	QPainter painter(&image);
	painter.setPen(QColor(255,255,255,127));

	const iCoord2D &mainPoint1 = _datasMainDominantPoints[0];
	const iCoord2D &mainPoint2 = _datasMainDominantPoints[1];

	// S'il y a deux points dominants principaux
	if ( (mainPoint1.x != -1 || mainPoint1.y != -1) && (mainPoint2.x != -1 || mainPoint2.y != -1) )
	{

		const iCoord2D &support1 = _datasMainSupportPoints[0];
		const iCoord2D &support2 = _datasMainSupportPoints[1];
		const int width = slice.n_cols;
		const int height = slice.n_rows;

		int i,j, index;

		const qreal daMain1Main2 = mainPoint1.y - mainPoint2.y;
		const qreal dbMain1Main2 = mainPoint2.x - mainPoint1.x;
		const qreal dcMain1Main2 = daMain1Main2*mainPoint1.x + dbMain1Main2*mainPoint1.y;
		const bool supToMain1Main2 = ( daMain1Main2*marrow.x + dbMain1Main2*marrow.y ) > dcMain1Main2;

		const qreal daMain1Support1 = mainPoint1.y - support1.y;
		const qreal dbMain1Support1 = support1.x - mainPoint1.x;
		const qreal dcMain1Support1 = daMain1Support1*mainPoint1.x + dbMain1Support1*mainPoint1.y;
		const bool supToMain1Support1 = ( daMain1Support1*mainPoint2.x + dbMain1Support1*mainPoint2.y ) > dcMain1Support1;

		const qreal daMain2Support2 = mainPoint2.y - support2.y;
		const qreal dbMain2Support2 = support2.x - mainPoint2.x;
		const qreal dcMain2Support2 = daMain2Support2*mainPoint2.x + dbMain2Support2*mainPoint2.y;
		const bool supToMain2Support2 = ( daMain2Support2*mainPoint1.x + dbMain2Support2*mainPoint1.y ) > dcMain2Support2;

		// Ajout des pixels du noeud dans l'aubier mais au-dessus de la droite passant par les deux points dominants
		// et du bon côté de chaque droite de prolongement
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( slice.at(j,i)
					 && ((daMain1Main2*i+dbMain1Main2*j < dcMain1Main2) == supToMain1Main2)
					 && ((daMain1Support1*i+dbMain1Support1*j > dcMain1Support1) == supToMain1Support1)
					 && ((daMain2Support2*i+dbMain2Support2*j > dcMain2Support2) == supToMain2Support2)
				   )
				{
					painter.drawPoint(i,j);
				}
			}
		}

		// Ajout des pixels du noeud en dehors de l'aubier.
		const int nbOriginalPointsContour = _datasOriginalContourPoints.size();
		int originalMain1Index, originalMain2Index, currentDistanceMain1, currentDistanceMain2, minXIndex, maxXIndex, minYIndex, maxYIndex;
		currentDistanceMain1 = currentDistanceMain2 = width+height;
		originalMain1Index = originalMain2Index = 0;
		minXIndex = maxXIndex = _datasOriginalContourPoints[0].x;
		minYIndex = maxYIndex = _datasOriginalContourPoints[0].y;
		for ( index=0 ; index<nbOriginalPointsContour ; index++ )
		{
			if ( _datasMainDominantPoints[0].distance(_datasOriginalContourPoints[index]) < currentDistanceMain1 )
			{
				currentDistanceMain1 = _datasMainDominantPoints[0].distance(_datasOriginalContourPoints[index]);
				originalMain1Index = index;
			}
			if ( _datasMainDominantPoints[1].distance(_datasOriginalContourPoints[index]) < currentDistanceMain2 )
			{
				currentDistanceMain2 = _datasMainDominantPoints[1].distance(_datasOriginalContourPoints[index]);
				originalMain2Index = index;
			}
			minXIndex = qMin(minXIndex,_datasOriginalContourPoints[index].x);
			maxXIndex = qMax(maxXIndex,_datasOriginalContourPoints[index].x);
			minYIndex = qMin(minYIndex,_datasOriginalContourPoints[index].y);
			maxYIndex = qMax(maxYIndex,_datasOriginalContourPoints[index].y);
		}

		// Création du polygone qui servira à tester l'appartenance d'un pixel au noeud.
		// Ce polygone est constitué :
		//     - du premier pixel du contour initial (non lissé) au pixel le plus proche du premier point dominant
		//     - du premier point dominant
		//     - du second point dominant
		//     - du pixel du contour initial le plus proche du second point dominant au dernier pixel du contour initial.

		QPolygon contourPolygon;
		for ( i=0 ; i<originalMain1Index ; ++i ) contourPolygon << QPoint(_datasOriginalContourPoints[i].x,_datasOriginalContourPoints[i].y);
		contourPolygon << QPoint(_datasMainDominantPoints[0].x,_datasMainDominantPoints[0].y);
		contourPolygon << QPoint(_datasMainDominantPoints[1].x,_datasMainDominantPoints[1].y);
		for ( i=originalMain2Index ; i<nbOriginalPointsContour ; ++i ) contourPolygon << QPoint(_datasOriginalContourPoints[i].x,_datasOriginalContourPoints[i].y);
		contourPolygon << QPoint(_datasOriginalContourPoints[0].x,_datasOriginalContourPoints[0].y);

		for ( j = minYIndex ; j<maxYIndex ; j++ )
		{
			for ( i = minXIndex ; i<maxXIndex ; i++ )
			{
				if ( slice.at(j,i) && contourPolygon.containsPoint(QPoint(i,j),Qt::OddEvenFill) )
				{
					painter.drawPoint(i,j);
				}
			}
		}
	}
	// Sinon on ajoute la composante en entier
	else
	{
		const int nbOriginalPointsContour = _datasOriginalContourPoints.size();

		QPolygon contourPolygon;
		int i, j, minXIndex, maxXIndex, minYIndex, maxYIndex;
		minXIndex = maxXIndex = _datasOriginalContourPoints[0].x;
		minYIndex = maxYIndex = _datasOriginalContourPoints[0].y;
		for ( i=0 ; i<nbOriginalPointsContour ; ++i )
		{
			contourPolygon << QPoint(_datasOriginalContourPoints[i].x,_datasOriginalContourPoints[i].y);
			minXIndex = qMin(minXIndex,_datasOriginalContourPoints[i].x);
			maxXIndex = qMax(maxXIndex,_datasOriginalContourPoints[i].x);
			minYIndex = qMin(minYIndex,_datasOriginalContourPoints[i].y);
			maxYIndex = qMax(maxYIndex,_datasOriginalContourPoints[i].y);
		}
		contourPolygon << QPoint(_datasOriginalContourPoints[0].x,_datasOriginalContourPoints[0].y);

		for ( j = minYIndex ; j<maxYIndex ; j++ )
		{
			for ( i = minXIndex ; i<maxXIndex ; i++ )
			{
				if ( slice.at(j,i) && contourPolygon.containsPoint(QPoint(i,j),Qt::OddEvenFill) )
				{
					painter.drawPoint(i,j);
				}
			}
		}
	}
}

void ContourCurve::writeContourContentInPgm3D( QDataStream &stream, const arma::Slice &slice, const iCoord2D &marrow ) const
{
	const iCoord2D &mainPoint1 = _datasMainDominantPoints[0];
	const iCoord2D &mainPoint2 = _datasMainDominantPoints[1];

	const int width = slice.n_cols;
	const int height = slice.n_rows;
	arma::Slice component(slice.n_rows,slice.n_cols);
	component.fill(0);

	int i,j,index;

	// S'il y a deux points dominants principaux
	if ( (mainPoint1.x != -1 || mainPoint1.y != -1) && (mainPoint2.x != -1 || mainPoint2.y != -1) )
	{
		const iCoord2D &support1 = _datasMainSupportPoints[0];
		const iCoord2D &support2 = _datasMainSupportPoints[1];


		const qreal daMain1Main2 = mainPoint1.y - mainPoint2.y;
		const qreal dbMain1Main2 = mainPoint2.x - mainPoint1.x;
		const qreal dcMain1Main2 = daMain1Main2*mainPoint1.x + dbMain1Main2*mainPoint1.y;
		const bool supToMain1Main2 = ( daMain1Main2*marrow.x + dbMain1Main2*marrow.y ) > dcMain1Main2;

		const qreal daMain1Support1 = mainPoint1.y - support1.y;
		const qreal dbMain1Support1 = support1.x - mainPoint1.x;
		const qreal dcMain1Support1 = daMain1Support1*mainPoint1.x + dbMain1Support1*mainPoint1.y;
		const bool supToMain1Support1 = ( daMain1Support1*mainPoint2.x + dbMain1Support1*mainPoint2.y ) > dcMain1Support1;

		const qreal daMain2Support2 = mainPoint2.y - support2.y;
		const qreal dbMain2Support2 = support2.x - mainPoint2.x;
		const qreal dcMain2Support2 = daMain2Support2*mainPoint2.x + dbMain2Support2*mainPoint2.y;
		const bool supToMain2Support2 = ( daMain2Support2*mainPoint1.x + dbMain2Support2*mainPoint1.y ) > dcMain2Support2;

		// Ajout des pixels du noeud dans l'aubier mais au-dessus de la droite passant par les deux points dominants
		// et du bon côté de chaque droite de prolongement
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				if ( slice.at(j,i)
					 && ((daMain1Main2*i+dbMain1Main2*j < dcMain1Main2) == supToMain1Main2)
					 && ((daMain1Support1*i+dbMain1Support1*j > dcMain1Support1) == supToMain1Support1)
					 && ((daMain2Support2*i+dbMain2Support2*j > dcMain2Support2) == supToMain2Support2)
				   )
				{
					component(i,j) = 1;
				}
			}
		}

		// Ajout des pixels du noeud en dehors de l'aubier.
		const int nbOriginalPointsContour = _datasOriginalContourPoints.size();
		int originalMain1Index, originalMain2Index, currentDistanceMain1, currentDistanceMain2, minXIndex, maxXIndex, minYIndex, maxYIndex;
		currentDistanceMain1 = currentDistanceMain2 = width+height;
		originalMain1Index = originalMain2Index = 0;
		minXIndex = maxXIndex = _datasOriginalContourPoints[0].x;
		minYIndex = maxYIndex = _datasOriginalContourPoints[0].y;
		for ( index=0 ; index<nbOriginalPointsContour ; index++ )
		{
			if ( _datasMainDominantPoints[0].distance(_datasOriginalContourPoints[index]) < currentDistanceMain1 )
			{
				currentDistanceMain1 = _datasMainDominantPoints[0].distance(_datasOriginalContourPoints[index]);
				originalMain1Index = index;
			}
			if ( _datasMainDominantPoints[1].distance(_datasOriginalContourPoints[index]) < currentDistanceMain2 )
			{
				currentDistanceMain2 = _datasMainDominantPoints[1].distance(_datasOriginalContourPoints[index]);
				originalMain2Index = index;
			}
			minXIndex = qMin(minXIndex,_datasOriginalContourPoints[index].x);
			maxXIndex = qMax(maxXIndex,_datasOriginalContourPoints[index].x);
			minYIndex = qMin(minYIndex,_datasOriginalContourPoints[index].y);
			maxYIndex = qMax(maxYIndex,_datasOriginalContourPoints[index].y);
		}

		// Création du polygone qui servira à tester l'appartenance d'un pixel au noeud.
		// Ce polygone est constitué :
		//     - du premier pixel du contour initial (non lissé) au pixel le plus proche du premier point dominant
		//     - du premier point dominant
		//     - du second point dominant
		//     - du pixel du contour initial le plus proche du second point dominant au dernier pixel du contour initial.

		QPolygon contourPolygon;
		for ( i=0 ; i<originalMain1Index ; ++i ) contourPolygon << QPoint(_datasOriginalContourPoints[i].x,_datasOriginalContourPoints[i].y);
		contourPolygon << QPoint(_datasMainDominantPoints[0].x,_datasMainDominantPoints[0].y);
		contourPolygon << QPoint(_datasMainDominantPoints[1].x,_datasMainDominantPoints[1].y);
		for ( i=originalMain2Index ; i<nbOriginalPointsContour ; ++i ) contourPolygon << QPoint(_datasOriginalContourPoints[i].x,_datasOriginalContourPoints[i].y);
		contourPolygon << QPoint(_datasOriginalContourPoints[0].x,_datasOriginalContourPoints[0].y);

		for ( j = minYIndex ; j<maxYIndex ; j++ )
		{
			for ( i = minXIndex ; i<maxXIndex ; i++ )
			{
				if ( slice.at(j,i) && contourPolygon.containsPoint(QPoint(i,j),Qt::OddEvenFill) )
				{
					component(i,j) = 1;
				}
			}
		}
	}
	// Sinon on ajoute la composante en entier
	else
	{
		const int nbOriginalPointsContour = _datasOriginalContourPoints.size();

		QPolygon contourPolygon;
		int i, j, minXIndex, maxXIndex, minYIndex, maxYIndex;
		minXIndex = maxXIndex = _datasOriginalContourPoints[0].x;
		minYIndex = maxYIndex = _datasOriginalContourPoints[0].y;
		for ( i=0 ; i<nbOriginalPointsContour ; ++i )
		{
			contourPolygon << QPoint(_datasOriginalContourPoints[i].x,_datasOriginalContourPoints[i].y);
			minXIndex = qMin(minXIndex,_datasOriginalContourPoints[i].x);
			maxXIndex = qMax(maxXIndex,_datasOriginalContourPoints[i].x);
			minYIndex = qMin(minYIndex,_datasOriginalContourPoints[i].y);
			maxYIndex = qMax(maxYIndex,_datasOriginalContourPoints[i].y);
		}
		contourPolygon << QPoint(_datasOriginalContourPoints[0].x,_datasOriginalContourPoints[0].y);

		for ( j = minYIndex ; j<maxYIndex ; j++ )
		{
			for ( i = minXIndex ; i<maxXIndex ; i++ )
			{
				if ( slice.at(j,i) && contourPolygon.containsPoint(QPoint(i,j),Qt::OddEvenFill) )
				{
					component(i,j) = 1;
				}
			}
		}
	}

	for ( j=0 ; j<height ; ++j )
	{
		for ( i=0 ; i<width ; ++i )
		{
			stream << static_cast<qint16>(component.at(j,i));
		}
	}
}
