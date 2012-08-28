#ifndef CONTOURCURVE_H
#define CONTOURCURVE_H

#include "billon_def.h"
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

	void setSmoothingRadius( const int &radius );
	void constructCurve( const Billon &billon, const iCoord2D &billonCenter, const int &sliceNumber, const int &componentNumber, const int &blurredSegmentThickness, const iCoord2D &startPoint = iCoord2D(-1,-1) );
	void draw( QImage &image ) const;
	void drawRestrictedComponent( QImage &image, const arma::Slice &slice, const iCoord2D &marrow ) const;

	void writeContourContentInPgm3D( QDataStream &stream, const arma::Slice &slice, const iCoord2D &marrow ) const;

private:
	void smoothCurve( QVector<iCoord2D> &contour, int smoothingRadius = 5 );

private:
	QVector<iCoord2D> _datasContourPoints;
	QVector<iCoord2D> _datasOriginalContourPoints;
	QVector<iCoord2D> _datasDominantPoints;
	QVector<iCoord2D> _datasMainDominantPoints;
	QVector<int> _datasIndexMainDominantPoints;
	QVector<iCoord2D> _datasMainSupportPoints;

	int _smoothingRadius;
};

#endif // CONTOURCURVATURECURVE_H
