#ifndef CONTOURCURVE_H
#define CONTOURCURVE_H

#include "billon_def.h"
#include "billon.h"
#include "global.h"
#include <qwt_plot_curve.h>

class ContourCurve
{
public:
	ContourCurve();
	~ContourCurve();

	const QVector<iCoord2D> &contourPoints() const;
	const QVector<iCoord2D> &dominantPoints() const;
	const QVector<iCoord2D> &mainDominantPoints() const;
	int indexOfMainPoint( const int &number ) const;
	const QVector<iCoord2D> &mainSupportPoints() const;
	int volumeContourContent() const;

	void constructCurve( const Billon &billon, const iCoord2D &billonCenter, const int &sliceNumber, const int &componentNumber, const int &blurredSegmentThickness, const int &smoothingRadius, const iCoord2D &startPoint = iCoord2D(-1,-1) );
	void draw( QImage &image ) const;
	void drawRestrictedComponent( QImage &image ) const;

	void writeContourContentInPgm3D( QDataStream &stream ) const;

private:
	void smoothCurve( QVector<iCoord2D> &contour, int smoothingRadius = 5 );

private:
	QVector<iCoord2D> _datasContourPoints;
	QVector<iCoord2D> _datasOriginalContourPoints;
	QVector<iCoord2D> _datasDominantPoints;
	QVector<iCoord2D> _datasMainDominantPoints;
	QVector<int> _datasIndexMainDominantPoints;
	QVector<iCoord2D> _datasMainSupportPoints;

	arma::Slice _component;
};

#endif // CONTOURCURVATURECURVE_H
