#include "inc/contourcurvaturecurve.h"

#include "inc/billon.h"
#include "inc/marrow.h"

#include <QProcess>
#include <QFile>

ContourCurvatureCurve::ContourCurvatureCurve()
{
	_curveCurvature.setPen(QPen(Qt::red));
	_curveCurrentPosition.setPen(QPen(Qt::yellow));
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
	}

	_curveCurvature.setSamples(curveDatas);
	_curveCurrentPosition.setSamples(curvePosition);
}
