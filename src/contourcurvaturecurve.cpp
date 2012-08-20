#include "inc/contourcurvaturecurve.h"

#include "inc/billon.h"
#include "inc/marrow.h"

#include <QProcess>
<<<<<<< HEAD
#include <QTemporaryFile>

ContourCurvatureCurve::ContourCurvatureCurve()
{
	_curveCurvature.setPen(QPen(Qt::blue));
	_curveCurrentPosition.setPen(QPen(Qt::red));
=======
#include <QFile>

ContourCurvatureCurve::ContourCurvatureCurve()
{
	_curveCurvature.setPen(QPen(Qt::red));
	_curveCurrentPosition.setPen(QPen(Qt::yellow));
>>>>>>> d51d019e6f6e4c1b23ddfd74c670e687d428b1ff
}

ContourCurvatureCurve::~ContourCurvatureCurve()
{
	detach();
}

void ContourCurvatureCurve::attach( QwtPlot * const plot )
{
	if ( plot != 0 )
	{
		_curveCurvature.attach(plot);
		_curveCurrentPosition.attach(plot);
	}
}

void ContourCurvatureCurve::detach()
{
	_curveCurvature.detach();
	_curveCurrentPosition.detach();
}

void ContourCurvatureCurve::clear()
{
	_curveCurvature.setSamples(QVector<QPointF>(0));
	_curveCurrentPosition.setSamples(QVector<QPointF>(0));
}

void ContourCurvatureCurve::setCurvePosition( const int &position )
{
<<<<<<< HEAD
	QVector<QPointF> curvePosition(2);
	curvePosition[0] = QPointF(position, 0.);
	curvePosition[1] = QPointF(position, _datasCurvature[position]);
	_curveCurrentPosition.setSamples(curvePosition);
}

const QVector<iCoord2D> &ContourCurvatureCurve::contourPoints() const
{
	return _datasContourPoints;
}

const QVector<iCoord2D> &ContourCurvatureCurve::dominantPoints() const
{
	return _datasDominantPoints;
}

void ContourCurvatureCurve::constructCurve( const Billon &billon, const Marrow *marrow, const int &sliceNumber, const int &componentNumber, const int &blurredSegmentThickness, const iCoord2D &startPoint )
{
	_datasCurvature.clear();
	_datasDominantPoints.clear();
	_datasContourPoints.clear();
	_datasContourPoints = billon.extractContour( marrow, sliceNumber, componentNumber, startPoint );

	int nbPoints = _datasContourPoints.size();
	QVector<QPointF> curveDatas(nbPoints), curvePosition(1);

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

		// Extraction de la courbure à partir des points de contour
//		QTemporaryFile fileCurvature("TKDetection_XXXXXX.crv");
//		if( !fileCurvature.open() )
//		{
//			qDebug() << QObject::tr("ERREUR : Impossible de créer le ficher de courbures %1.").arg(fileCurvature.fileName());
//			return;
//		}
//		fileCurvature.close();

//		QProcess curvatureExtraction;
//		curvatureExtraction.setStandardInputFile(fileContours.fileName());
//		curvatureExtraction.setStandardOutputFile(fileCurvature.fileName());
//		curvatureExtraction.start(QString("curvature_gmcb -setWidth %1").arg(blurredSegmentThickness));

//		if ( curvatureExtraction.waitForFinished(3000) )
//		{
//			if( !fileCurvature.open() )
//			{
//				qDebug() << QObject::tr("ERREUR : Impossible de lire le ficher de courbures %1.").arg(fileCurvature.fileName());
//				return;
//			}
//			QTextStream streamCurvature(&fileCurvature);
//			streamCurvature.readLine();
//			int temp;
//			qreal curvature;
//			for ( int i=0 ; i<nbPoints ; ++i )
//			{
//				streamCurvature >> temp >> temp >> curvature;
//				_datasCurvature << curvature;
//				curveDatas[i] = QPointF(i,curvature);
//			}
//			curvePosition[0] = curveDatas[0];
//			fileCurvature.close();
//		}

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
		}

		mainDominantPoints();
=======
	QVector<QPointF> curvePosition(1);
	curvePosition[0] = QPointF(position, _datasCurvature[position]);
	_curveCurrentPosition.setSamples(curvePosition);
}

const QVector<iCoord2D> &ContourCurvatureCurve::curvatureEdges() const
{
	return _datasEdges;
}

void ContourCurvatureCurve::constructCurve( const Billon &billon, const Marrow *marrow, const int &sliceNumber, const int &componentNumber )
{
	_datasCurvature.clear();
	_datasEdges.clear();
	_datasEdges = billon.extractEdges( marrow, sliceNumber, componentNumber );

	int nbPoints = _datasEdges.size();
	QVector<QPointF> curveDatas(nbPoints), curvePosition(1);

	if ( nbPoints )
	{
		QFile fileContours("01234567890123456789.ctr");
		if( !fileContours.open(QIODevice::WriteOnly) )
		{
			qDebug() << QObject::tr("ERREUR : Impossible de créer le ficher de contours 01234567890123456789.ctr.");
			return;
		}
		QTextStream streamContours(&fileContours);
		for ( int i=0 ; i<nbPoints ; ++i )
		{
			streamContours << _datasEdges.at(i).x << " " << _datasEdges.at(i).y << endl;
		}
		fileContours.close();

		QProcess curvatureExtraction;
		curvatureExtraction.setStandardInputFile("01234567890123456789.ctr");
		curvatureExtraction.setStandardOutputFile("01234567890123456789.crv");
		curvatureExtraction.start("curvature_gmcb -setWidth 1");

		if ( curvatureExtraction.waitForFinished(5000) )
		{
			QFile fileCurvature("01234567890123456789.crv");
			if( !fileCurvature.open(QIODevice::ReadOnly) )
			{
				qDebug() << QObject::tr("ERREUR : Impossible de lire le ficher de contours 01234567890123456789.crv.");
				return;
			}
			QTextStream streamCurvature(&fileCurvature);
			streamCurvature.readLine();
			int temp;
			qreal curvature;
			for ( int i=0 ; i<nbPoints ; ++i )
			{
				streamCurvature >> temp >> temp >> curvature;
				_datasCurvature << curvature;
				curveDatas[i] = QPointF(i,curvature);
				qDebug() << curvature;
			}
			curvePosition[0] = curveDatas[0];
			fileCurvature.close();
		}
>>>>>>> d51d019e6f6e4c1b23ddfd74c670e687d428b1ff
	}

	_curveCurvature.setSamples(curveDatas);
	_curveCurrentPosition.setSamples(curvePosition);
}
<<<<<<< HEAD

QVector<iCoord2D> ContourCurvatureCurve::mainDominantPoints() const
{
	QVector<iCoord2D> mainPoints(0);
	const int nbPoints = _datasDominantPoints.size();
	if ( nbPoints > 2 )
	{
		mainPoints.reserve(2);
		QVector<iCoord2D> dominantPoints;
		dominantPoints.reserve(nbPoints+1);
		dominantPoints << _datasDominantPoints << _datasDominantPoints[0];
		int index = 1;
		while ( index < nbPoints && dominantPoints[index].angle(dominantPoints[index-1],dominantPoints[index+1]) > PI_ON_TWO ) index++;
		if (index < nbPoints ) mainPoints << dominantPoints[index];
		index = nbPoints-1;
		while ( index > 0 && dominantPoints[index].angle(dominantPoints[index-1],dominantPoints[index+1]) > PI_ON_TWO ) index--;
		if ( index > 0 ) mainPoints << dominantPoints[index];
	}
	return mainPoints;
}
=======
>>>>>>> d51d019e6f6e4c1b23ddfd74c670e687d428b1ff
