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
	_datasContourPoints.clear();
	_datasContourPoints = billon.extractContour( billonCenter, sliceNumber, componentNumber, startPoint );
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
				const int padding = qMin(billon.n_cols,billon.n_rows)/20;
				QVector<iCoord2D> dominantPoints;
				dominantPoints.reserve(nbPoints+1);
				dominantPoints << _datasDominantPoints << _datasDominantPoints[0];
				int index, oldIndex;
				qreal angle, oldAngle;
//				std::cout << "======================================================" <<  std::endl;
//				std::cout << "||                                                  ||" <<  std::endl;
//				std::cout << "Angles depuis le début : " <<  std::endl;
//				for ( int i = 1; i<nbPoints ; ++i )
//				{
//					dominantPoints[i].angle(dominantPoints[i-1],dominantPoints[i+1]);
//				}
//				std::cout << "Angles depuis la fin : " <<  std::endl;
//				for ( int i = nbPoints-1; i>0 ; --i )
//				{
//					dominantPoints[i].angle(dominantPoints[i+1],dominantPoints[i-1]);
//				}
//				std::cout << "||                                                  ||" <<  std::endl;
//				std::cout << "======================================================" <<  std::endl;
				index = 1;
				angle = dominantPoints[index].angle(dominantPoints[index-1],dominantPoints[index+1]);
				while ( index < nbPoints )
				{
					oldAngle = qAbs(angle);
					angle = dominantPoints[index].angle(dominantPoints[index-1],dominantPoints[index+1]);
					if ( ((angle < -PI_ON_FOUR || angle  > THREE_PI_ON_FOUR) && oldAngle+qAbs(angle)>PI) || dominantPoints[index].distance(dominantPoints[0]) < 20 ) index++;
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
					oldAngle = qAbs(angle);
					angle = dominantPoints[index].angle(dominantPoints[index+1],dominantPoints[index-1]);
					if ( ((angle > PI_ON_FOUR || angle < -THREE_PI_ON_FOUR) && oldAngle+qAbs(angle)>PI) || dominantPoints[index].distance(dominantPoints[0]) < 20 ) index--;
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

			painter.setPen(Qt::red);

			const iCoord2D &mainPoint1 = _datasMainDominantPoints[0];
			const iCoord2D &mainPoint2 = _datasMainDominantPoints[1];

			if ( _datasIndexMainDominantPoints[0] != -1 ) painter.drawEllipse(mainPoint1.x-3,mainPoint1.y-3,6,6);
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


void ContourCurve::drawRestrictedComponent( QImage &image, const arma::Slice &slice ) const
{
	const iCoord2D &mainPoint1 = _datasMainDominantPoints[0];
	const iCoord2D &mainPoint2 = _datasMainDominantPoints[1];

	if ( (mainPoint1.x != -1 || mainPoint1.y != -1) && (mainPoint2.x != -1 || mainPoint2.y != -1) )
	{
		QPainter painter(&image);

		const int width = slice.n_cols;
		const int height = slice.n_rows;

		int i,j;

		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				color = slice.at(j,i);
				if ( color )
				{
					painter.setPen(colors[color%nbColors]);
					painter.drawPoint(i,j);
				}
			}
		}
	}
}
