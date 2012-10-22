#ifndef CONTOURCURVE_H
#define CONTOURCURVE_H

#include "def/def_coordinate.h"
#include "def/def_billon.h"

#include "billon.h"

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

	void constructCurve( const Slice &slice, const iCoord2D &sliceCenter, const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius, const iCoord2D &startPoint = iCoord2D(-1,-1) );
	void constructCurveOldMethod( const Slice &slice, const iCoord2D &sliceCenter, const int &intensityThreshold, const int &smoothingRadius, const iCoord2D &startPoint = iCoord2D(-1,-1) );
	void draw( QImage &image ) const;
	void drawRestrictedComponent( QImage &image ) const;
	void writeContourContentInPgm3D( QDataStream &stream ) const;
	void writeContourContentInSDP( QTextStream &stream, const uint &sliceNum ) const;

private:
	void smoothCurve( QVector<iCoord2D> &contour, int smoothingRadius = 5 ) const;
	void extractContourPointsAndDominantPoints( const Slice &slice, const iCoord2D &sliceCenter, const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius, const iCoord2D &startPoint );
	void computeMainDominantPoints( const iCoord2D &sliceCenter );
	void computeContourPolygons( const Slice &slice );
	void updateSlice( const Slice &slice, const iCoord2D &sliceCenter, const int &intensityThreshold );

private:
	QVector<iCoord2D> _datasContourPoints;
	QVector<iCoord2D> _datasOriginalContourPoints;
	QVector<iCoord2D> _datasDominantPoints;
	QVector<iCoord2D> _datasMainDominantPoints;
	QVector<int> _datasIndexMainDominantPoints;
	QVector<iCoord2D> _datasMainSupportPoints;

	QPolygon _contourPolygonBottom;
	QPolygon _contourPolygonTop;

	Slice _component;
};

#endif // CONTOURCURVATURECURVE_H
