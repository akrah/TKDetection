#ifndef CONTOURSLICE_H
#define CONTOURSLICE_H

#include "def/def_coordinate.h"
#include "def/def_billon.h"
#include "inc/coordinate.h"
#include "inc/contour.h"
#include "inc/curvaturehistogram.h"

#include <QVector>
#include <QPolygon>

class QImage;

class ContourSlice
{
public:
	ContourSlice();
	~ContourSlice();

	const Contour &contour() const;
	const CurvatureHistogram &curvatureHistogram() const;
	const iCoord2D &dominantPoint( const uint &index ) const;
	const QVector<uint> &dominantPointIndex() const;
	const iCoord2D &leftMainDominantPoint() const;
	const iCoord2D &rightMainDominantPoint() const;
	const int &leftMainDominantPointIndex() const;
	const int &rightMainDominantPointIndex() const;
	const rCoord2D &leftMainSupportPoint() const;
	const rCoord2D &rightMainSupportPoint() const;

	void compute( Slice &resultSlice, const Slice &initialSlice, const iCoord2D &sliceCenter, const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius, const int &curvatureWidth, const iCoord2D &startPoint = iCoord2D(-1,-1) );
	void computeOldMethod( Slice &resultSlice, const Slice &initialSlice, const iCoord2D &sliceCenter, const int &intensityThreshold, const int &smoothingRadius, const int &curvatureWidth, const iCoord2D &startPoint = iCoord2D(-1,-1) );
	void draw( QImage &image , const int &cursorPosition = -1 ) const;

private:
	void computeDominantPoints(const int &blurredSegmentThickness);
	void computeMainDominantPoints();
	void computeSupportsOfMainDominantPoints();
	void computeContourPolygons();
	void updateSlice( const Slice &initialSlice, Slice &resultSlice, const iCoord2D &sliceCenter, const int &intensityThreshold );

private:
	Contour _contour;
	Contour _originalContour;

	CurvatureHistogram _curvatureHistogram;

	QVector<uint> _dominantPointsIndex;
	int _leftMainDominantPointsIndex;
	int _rightMainDominantPointsIndex;
	rCoord2D _leftMainSupportPoint;
	rCoord2D _rightMainSupportPoint;

	QPolygon _contourPolygonBottom;
	QPolygon _contourPolygonTop;

	static iCoord2D invalidICoord2D;
};

#endif // CONTOURSLICE_H
