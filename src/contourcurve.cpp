#include "inc/contourcurve.h"

#include "inc/billon.h"

#include <QProcess>
#include <QTemporaryFile>
#include <QPainter>

ContourCurve::ContourCurve()
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

int ContourCurve::volumeContourContent() const
{
	uint i, j, volume;
	volume = 0;
	for ( j=0 ; j<_component.n_rows ; ++j )
	{
		for ( i=0 ; i<_component.n_cols ; ++i )
		{
			if (_component.at(j,i)) volume++;
		}
	}
	return volume;
}

void ContourCurve::constructCurve( const Billon &billon, const iCoord2D &billonCenter, const int &sliceNumber, const int &componentNumber, const int &blurredSegmentThickness, const int &smoothingRadius, const iCoord2D &startPoint )
{
	_datasOriginalContourPoints.clear();
	_datasOriginalContourPoints = billon.extractContour( billonCenter, sliceNumber, componentNumber, startPoint );
	_datasContourPoints.clear();
	_datasContourPoints = _datasOriginalContourPoints;
	smoothCurve(_datasContourPoints,smoothingRadius);

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
				int index, oldIndex;
				qreal angle, oldAngle;

				// Point dominant dans le sens du contour
				index = 1;
				angle = -THREE_PI_ON_FOUR;
				while ( index < nbPoints )
				{
					oldAngle = qFuzzyCompare(angle+1,1.)?180.:angle;
					angle = _datasDominantPoints[index].angle(_datasDominantPoints[index-1],_datasDominantPoints[(index+1)%nbPoints]);
					if ( ((angle < -PI_ON_FOUR || angle  > THREE_PI_ON_FOUR) && (angle/oldAngle>0 || qAbs(oldAngle)+qAbs(angle)>PI)) || _datasDominantPoints[index].distance(_datasDominantPoints[0]) < 20 ) index++;
					else break;
				}
				oldIndex = index;
				// Si le point dominant trouvé est correct
				if ( index < nbPoints )
				{
					_datasMainDominantPoints[0] = _datasDominantPoints[index];
					_datasIndexMainDominantPoints[0] = index;

					// Calcul du point dominant support du point dominant principal
//					while ( index > 0 && _datasDominantPoints[index].distance(_datasMainDominantPoints[0]) < padding ) index--;
//					_datasMainSupportPoints[0] = _datasDominantPoints[index];
					_datasMainSupportPoints[0].x = _datasMainSupportPoints[0].y = 0;
					while ( index >= 0 )
					{
						_datasMainSupportPoints[0].x += _datasDominantPoints[index].x;
						_datasMainSupportPoints[0].y += _datasDominantPoints[index].y;
						index--;
					}
					_datasMainSupportPoints[0].x /= (_datasIndexMainDominantPoints[0] + 1);
					_datasMainSupportPoints[0].y /= (_datasIndexMainDominantPoints[0] + 1);

					// Si le point de support n'est pas correct
					qreal angle = _datasMainSupportPoints[0].angle(billonCenter,_datasMainDominantPoints[0]);
					if ( angle > -SEVEN_PI_ON_EIGHT && angle < THREE_PI_ON_FOUR )
					{
						_datasMainDominantPoints[0] = _datasMainSupportPoints[0] = iCoord2D(-1,-1);
						_datasIndexMainDominantPoints[0] = -1;
					}
				}

				// Point dominant dans le sens contraire du contour
				index = nbPoints-1;
				angle = THREE_PI_ON_FOUR;
				while ( index > oldIndex )
				{
					oldAngle = qFuzzyCompare(angle+1,1.)?180.:angle;
					angle = _datasDominantPoints[index].angle(_datasDominantPoints[(index+1)%nbPoints],_datasDominantPoints[index-1]);
					if ( ((angle > PI_ON_FOUR || angle < -THREE_PI_ON_FOUR) && (angle/oldAngle>0 || qAbs(oldAngle)+qAbs(angle)>PI)) || _datasDominantPoints[index].distance(_datasDominantPoints[0]) < 20 ) index--;
					else break;
				}
				// Si le point dominant trouvé est correct
				if ( index > oldIndex )
				{
					_datasMainDominantPoints[1] = _datasDominantPoints[index];
					_datasIndexMainDominantPoints[1] = index;

					// Calcul du point dominant support du point dominant principal
//					index = (index+1)%nbPoints;
//					while ( index>0 && index<nbPoints-1 && _datasDominantPoints[index].distance(_datasMainDominantPoints[1]) < padding ) index++;
//					_datasMainSupportPoints[1] = _datasDominantPoints[index%nbPoints];
					_datasMainSupportPoints[1].x = _datasMainSupportPoints[1].y = 0;
					while ( index != 1 )
					{
						_datasMainSupportPoints[1].x += _datasDominantPoints[index].x;
						_datasMainSupportPoints[1].y += _datasDominantPoints[index].y;
						index = (index+1)%nbPoints;
					}
					_datasMainSupportPoints[1].x /= (nbPoints - _datasIndexMainDominantPoints[1] + 1);
					_datasMainSupportPoints[1].y /= (nbPoints - _datasIndexMainDominantPoints[1] + 1);

					// Si le point de support n'est pas correct
					qreal angle = _datasMainSupportPoints[1].angle(billonCenter,_datasMainDominantPoints[1]);
					if ( angle > -THREE_PI_ON_FOUR && angle < SEVEN_PI_ON_EIGHT )
					{
						_datasMainDominantPoints[1] = _datasMainSupportPoints[1] = iCoord2D(-1,-1);
						_datasIndexMainDominantPoints[1] = -1;
					}
				}
			}
		}
	}

	const arma::Slice &slice = billon.slice(sliceNumber);
	const int width = slice.n_cols;
	const int height = slice.n_rows;
	_component.resize(height,width);
	_component.fill(0);

	const iCoord2D &mainPoint1 = _datasMainDominantPoints[0];
	const iCoord2D &mainPoint2 = _datasMainDominantPoints[1];
	const iCoord2D &support1 = _datasMainSupportPoints[0];
	const iCoord2D &support2 = _datasMainSupportPoints[1];

	const qreal daMain1Main2 = mainPoint1.y - mainPoint2.y;
	const qreal dbMain1Main2 = mainPoint2.x - mainPoint1.x;
	const qreal dcMain1Main2 = daMain1Main2*mainPoint1.x + dbMain1Main2*mainPoint1.y;
	const bool supToMain1Main2 = ( daMain1Main2*billonCenter.x + dbMain1Main2*billonCenter.y ) > dcMain1Main2;

	const qreal daMain1Support1 = mainPoint1.y - support1.y;
	const qreal dbMain1Support1 = support1.x - mainPoint1.x;
	const qreal dcMain1Support1 = daMain1Support1*mainPoint1.x + dbMain1Support1*mainPoint1.y;
	const bool supToMain1Support1 = ( daMain1Support1*mainPoint2.x + dbMain1Support1*mainPoint2.y ) > dcMain1Support1;

	const qreal daMain2Support2 = mainPoint2.y - support2.y;
	const qreal dbMain2Support2 = support2.x - mainPoint2.x;
	const qreal dcMain2Support2 = daMain2Support2*mainPoint2.x + dbMain2Support2*mainPoint2.y;
	const bool supToMain2Support2 = ( daMain2Support2*mainPoint1.x + dbMain2Support2*mainPoint1.y ) > dcMain2Support2;

	const bool hasMain1 = (mainPoint1.x != -1 || mainPoint1.y != -1);
	const bool hasMain2 = (mainPoint2.x != -1 || mainPoint2.y != -1);
	const int nbOriginalPointsContour = _datasOriginalContourPoints.size();

	QPolygon contourPolygonBottom;
	QPolygon contourPolygonTop;
	int i,j, index, originalMain1Index, originalMain2Index, currentDistanceMain1, currentDistanceMain2, minXIndex, maxXIndex, minYIndex, maxYIndex;

	// S'il y a deux points dominants principaux
	if ( hasMain1 && hasMain2 )
	{
		// Calcul des point du contour original les plus proches des points dominants principaux
		// et de la boite englobante du contour original
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

		// Création du polygone qui servira à tester l'appartenance d'un pixel en dessous de la droite des deux points dominants principaux au noeud.
		// Ce polygone est constitué :
		//     - des points compris entre le premier point du contour initial (non lissé) et le plus proche du premier point dominant
		//     - du point le plus proche du premier point dominant
		//     - du point le plus proche du second point dominant
		//     - des points compris entre le point du contour initial le plus proche du second point dominant et le dernier points du contour initial.
		for ( i=0 ; i<=originalMain1Index ; ++i ) contourPolygonBottom << QPoint(_datasOriginalContourPoints[i].x,_datasOriginalContourPoints[i].y);
		contourPolygonBottom << QPoint(mainPoint1.x,mainPoint1.y) << QPoint(mainPoint2.x,mainPoint2.y);
		for ( i=originalMain2Index ; i<nbOriginalPointsContour ; ++i ) contourPolygonBottom << QPoint(_datasOriginalContourPoints[i].x,_datasOriginalContourPoints[i].y);
		contourPolygonBottom << QPoint(_datasOriginalContourPoints[0].x,_datasOriginalContourPoints[0].y);

		// Création du polygone qui servira à tester l'appartenance d'un pixel au dessus de la droite des deux points dominants principaux au noeud.
		// Ce polygone est constitué :
		//     - du point le plus proche du premier point dominant
		//     - des points du contour initial (non lissé) situés entre le point le plus proche du premier point dominant et le point le plus proche du second point dominant
		//     - du point le plus proche du second point dominant
		contourPolygonTop << QPoint(mainPoint1.x,mainPoint1.y);
		for ( i=originalMain1Index ; i<=originalMain2Index ; ++i ) contourPolygonTop << QPoint(_datasOriginalContourPoints[i].x,_datasOriginalContourPoints[i].y);
		contourPolygonTop << QPoint(mainPoint2.x,mainPoint2.y);
		contourPolygonTop << QPoint(_datasOriginalContourPoints[originalMain1Index].x,_datasOriginalContourPoints[originalMain1Index].y);

		// Ajout des pixel
		for ( j = minYIndex ; j<maxYIndex ; j++ )
		{
			for ( i = minXIndex ; i<maxXIndex ; i++ )
			{
				if ( slice.at(j,i) &&
						( contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill)
									|| ( ((daMain1Main2*i+dbMain1Main2*j <= dcMain1Main2) == supToMain1Main2)
										 && ((daMain1Support1*i+dbMain1Support1*j >= dcMain1Support1) == supToMain1Support1)
										 && ((daMain2Support2*i+dbMain2Support2*j >= dcMain2Support2) == supToMain2Support2)
										 && contourPolygonTop.containsPoint(QPoint(i,j),Qt::OddEvenFill)
									   )
						)
					 )
				{
					_component.at(j,i) = 1;
				}
			}
		}
	}
	else if ( hasMain1 )
	{
		// Création du polygone qui servira à tester l'appartenance d'un pixel au noeud.
		// Ce polygone est constitué des pixels du contour initial (non lissé)
		minXIndex = maxXIndex = _datasOriginalContourPoints[0].x;
		minYIndex = maxYIndex = _datasOriginalContourPoints[0].y;
		for ( i=0 ; i<nbOriginalPointsContour ; i++ )
		{
			contourPolygonBottom << QPoint(_datasOriginalContourPoints[i].x,_datasOriginalContourPoints[i].y);
			minXIndex = qMin(minXIndex,_datasOriginalContourPoints[i].x);
			maxXIndex = qMax(maxXIndex,_datasOriginalContourPoints[i].x);
			minYIndex = qMin(minYIndex,_datasOriginalContourPoints[i].y);
			maxYIndex = qMax(maxYIndex,_datasOriginalContourPoints[i].y);
		}
		contourPolygonBottom << QPoint(_datasOriginalContourPoints[0].x,_datasOriginalContourPoints[0].y);

		// Ajout des pixels du noeud du bon côté de chaque la droite de prolongement
		// et à l'intérieur du contour original
		const iCoord2D nextMain1( mainPoint1.x - daMain1Support1, mainPoint1.y - dbMain1Support1 );
		const bool rightToMain1Support1 = ( daMain1Support1*nextMain1.x + dbMain1Support1*nextMain1.y ) > dcMain1Support1;
		for ( j = minYIndex ; j<maxYIndex ; j++ )
		{
			for ( i = minXIndex ; i<maxXIndex ; i++ )
			{
				if ( slice.at(j,i)
					 && ((daMain1Support1*i+dbMain1Support1*j > dcMain1Support1) == rightToMain1Support1)
					 && contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill) )
				{
					_component.at(j,i) = 1;
				}
			}
		}
	}
	else if ( hasMain2 )
	{
		// Création du polygone qui servira à tester l'appartenance d'un pixel au noeud.
		// Ce polygone est constitué des pixels du contour initial (non lissé)
		QPolygon contourPolygon;
		int minXIndex, maxXIndex, minYIndex, maxYIndex;
		minXIndex = maxXIndex = _datasOriginalContourPoints[0].x;
		minYIndex = maxYIndex = _datasOriginalContourPoints[0].y;
		for ( i=0 ; i<nbOriginalPointsContour ; i++ )
		{
			contourPolygon << QPoint(_datasOriginalContourPoints[i].x,_datasOriginalContourPoints[i].y);
			minXIndex = qMin(minXIndex,_datasOriginalContourPoints[i].x);
			maxXIndex = qMax(maxXIndex,_datasOriginalContourPoints[i].x);
			minYIndex = qMin(minYIndex,_datasOriginalContourPoints[i].y);
			maxYIndex = qMax(maxYIndex,_datasOriginalContourPoints[i].y);
		}
		contourPolygon << QPoint(_datasOriginalContourPoints[0].x,_datasOriginalContourPoints[0].y);

		// Ajout des pixels du noeud du bon côté de chaque la droite de prolongement
		// et à l'intérieur du contour original
		const iCoord2D nextMain2( mainPoint2.x + daMain2Support2, mainPoint2.y + dbMain2Support2 );
		const bool leftToMain2Support2 = ( daMain2Support2*nextMain2.x + dbMain2Support2*nextMain2.y ) > dcMain2Support2;
		for ( j = minYIndex ; j<maxYIndex ; j++ )
		{
			for ( i = minXIndex ; i<maxXIndex ; i++ )
			{
				if ( slice.at(j,i)
					 && ((daMain2Support2*i+dbMain2Support2*j > dcMain2Support2) == leftToMain2Support2)
					 && contourPolygon.containsPoint(QPoint(i,j),Qt::OddEvenFill) )
				{
					_component.at(j,i) = 1;
				}
			}
		}
	}
	// Sinon on ajoute la composante en entier
	else if ( nbOriginalPointsContour > 3 )
	{
		minXIndex = maxXIndex = _datasOriginalContourPoints[0].x;
		minYIndex = maxYIndex = _datasOriginalContourPoints[0].y;
		for ( i=0 ; i<nbOriginalPointsContour ; ++i )
		{
			contourPolygonBottom << QPoint(_datasOriginalContourPoints[i].x,_datasOriginalContourPoints[i].y);
			minXIndex = qMin(minXIndex,_datasOriginalContourPoints[i].x);
			maxXIndex = qMax(maxXIndex,_datasOriginalContourPoints[i].x);
			minYIndex = qMin(minYIndex,_datasOriginalContourPoints[i].y);
			maxYIndex = qMax(maxYIndex,_datasOriginalContourPoints[i].y);
		}
		contourPolygonBottom << QPoint(_datasOriginalContourPoints[0].x,_datasOriginalContourPoints[0].y);

		for ( j = minYIndex ; j<maxYIndex ; j++ )
		{
			for ( i = minXIndex ; i<maxXIndex ; i++ )
			{
				if ( slice.at(j,i) && contourPolygonBottom.containsPoint(QPoint(i,j),Qt::OddEvenFill) )
				{
					_component.at(j,i) = 1;
				}
			}
		}
	}


}

void ContourCurve::smoothCurve( QVector<iCoord2D> &contour, int smoothingRadius )
{
	if ( smoothingRadius > 0 )
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
}

void ContourCurve::draw( QImage &image ) const
{
	QPainter painter(&image);
	int i;

	const int nbContourPoints = _datasContourPoints.size();
	if ( nbContourPoints > 0 )
	{
		painter.setPen(Qt::blue);
		for ( i=0 ; i<nbContourPoints ; ++i )
		{
			painter.drawPoint(_datasContourPoints[i].x,_datasContourPoints[i].y);
		}

		const int nbDominantPoints = _datasDominantPoints.size();
		if ( nbDominantPoints > 0 )
		{
			painter.setPen(Qt::green);
			for ( i=0 ; i<nbDominantPoints ; ++i )
			{
				painter.drawEllipse(_datasDominantPoints[i].x-2,_datasDominantPoints[i].y-2,4,4);
			}


			const iCoord2D &mainPoint1 = _datasMainDominantPoints[0];
			const iCoord2D &mainPoint2 = _datasMainDominantPoints[1];

			painter.setPen(Qt::red);
			if ( _datasIndexMainDominantPoints[0] != -1 ) painter.drawEllipse(mainPoint1.x-3,mainPoint1.y-3,6,6);
			if ( _datasIndexMainDominantPoints[1] != -1 ) painter.drawEllipse(mainPoint2.x-3,mainPoint2.y-3,6,6);

			painter.setPen(Qt::gray);

			const iCoord2D &supportMain1 = _datasMainSupportPoints[0];
			qreal a, b;
			if ( supportMain1.x != -1 || supportMain1.y != -1 )
			{
				a = ( mainPoint1.y - supportMain1.y ) / static_cast<qreal>( mainPoint1.x - supportMain1.x );
				b = ( mainPoint1.y * supportMain1.x - mainPoint1.x * supportMain1.y ) / static_cast<qreal>( supportMain1.x - mainPoint1.x );
//				painter.drawLine(0, b, image.width(), a * image.width() + b );
				if ( supportMain1.x < mainPoint1.x )
				{
					painter.drawLine(mainPoint1.x, mainPoint1.y, image.width(), a * image.width() + b );
				}
				else
				{
					painter.drawLine(mainPoint1.x, mainPoint1.y, 0., b );
				}
			}
			const iCoord2D &supportMain2 = _datasMainSupportPoints[1];
			if ( supportMain2.x != -1 || supportMain2.y != -1 )
			{
				a = ( mainPoint2.y - supportMain2.y ) / static_cast<qreal>( mainPoint2.x - supportMain2.x );
				b = ( mainPoint2.y * supportMain2.x - mainPoint2.x * supportMain2.y ) / static_cast<qreal>( supportMain2.x - mainPoint2.x );
//				painter.drawLine(0, b, image.width(), a * image.width() + b );
				if ( supportMain2.x < mainPoint2.x )
				{
					painter.drawLine(mainPoint2.x, mainPoint2.y, image.width(), a * image.width() + b );
				}
				else
				{
					painter.drawLine(mainPoint2.x, mainPoint2.y, 0., b );
				}
			}
		}

		painter.setPen(Qt::red);
		painter.drawEllipse(_datasContourPoints[0].x-1,_datasContourPoints[0].y-1,2,2);
	}
}


void ContourCurve::drawRestrictedComponent( QImage &image ) const
{
	QPainter painter(&image);
	painter.setPen(QColor(255,255,255,127));

	uint i, j;
	for ( j=0 ; j<_component.n_rows ; j++ )
	{
		for ( i=0 ; i<_component.n_cols ; i++ )
		{
			if ( _component.at(j,i) ) painter.drawPoint(i,j);
		}
	}
}

void ContourCurve::writeContourContentInPgm3D( QDataStream &stream ) const
{
	uint i, j;
	for ( j=0 ; j<_component.n_rows ; ++j )
	{
		for ( i=0 ; i<_component.n_cols ; ++i )
		{
			stream << static_cast<qint16>(_component.at(j,i));
		}
	}
}
