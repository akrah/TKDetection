#ifndef CONTOURCURVE_H
#define CONTOURCURVE_H

#include "billon_def.h"
#include "global.h"
#include <qwt_plot_curve.h>

class Marrow;

class ContourCurve
{
public:
	ContourCurve();
	~ContourCurve();

	const QVector<iCoord2D> &contourPoints() const;
	const QVector<iCoord2D> &dominantPoints() const;
	const QVector<iCoord2D> &mainDominantPoints() const;
	int indexOfMainPoint( const int &number ) const;

	void constructCurve( const Billon &billon, const iCoord2D &billonCenter, const int &sliceNumber, const int &componentNumber, const int &blurredSegmentThickness, const iCoord2D &startPoint = iCoord2D(-1,-1) );

	void draw( QImage &image ) const;

private:
	void smoothCurve( QVector<iCoord2D> &contour, const int &smoothingRadius = 5 );

private:
	QVector<iCoord2D> _datasContourPoints;
	QVector<iCoord2D> _datasDominantPoints;
	QVector<iCoord2D> _datasMainDominantPoints;
	QVector<int> _datasIndexMainDominantPoints;
};

#endif // CONTOURCURVATURECURVE_H
