#ifndef CONTOURSLICE_H
#define CONTOURSLICE_H

#include "def/def_coordinate.h"
#include "def/def_billon.h"
#include "inc/coordinate.h"
#include "inc/contour.h"
#include "inc/curvaturehistogram.h"
#include "inc/contourdistanceshistogram.h"

#include <QVector>
#include <QPolygon>

class QImage;

class ContourSlice
{
public:
	ContourSlice();
	~ContourSlice();

	const Contour &contour() const;
	const ContourDistancesHistogram &contourDistancesHistogram() const;
	const CurvatureHistogram &curvatureHistogram() const;
	const iCoord2D &leftMainDominantPoint() const;
	const iCoord2D &rightMainDominantPoint() const;
	const int &leftMainDominantPointIndex() const;
	const int &rightMainDominantPointIndex() const;
	const rCoord2D &leftMainSupportPoint() const;
	const rCoord2D &rightMainSupportPoint() const;

	void compute( Slice &resultSlice, const Slice &initialSlice, const uiCoord2D &sliceCenter, const int &intensityThreshold,
				  const int &smoothingRadius, const int &curvatureWidth, const qreal &curvatureThreshold, const iCoord2D &startPoint = iCoord2D(-1,-1) );

	void draw( QPainter &painter, const int &cursorPosition, const TKD::ViewType &viewType ) const;

private:
	void computeMainDominantPoints( const qreal &curvatureThreshold );
	void computeSupportsOfMainDominantPoints( const int &meansMaskSize );
	void computeContourPolygons();
	void updateSlice(const Slice &initialSlice, Slice &resultSlice, const int &intensityThreshold );

private:
	Contour _contour;
	Contour _originalContour;
	uiCoord2D _sliceCenter;

	ContourDistancesHistogram _contourDistancesHistogram;
	CurvatureHistogram _curvatureHistogram;

	int _leftMainDominantPointsIndex;
	int _rightMainDominantPointsIndex;
	rCoord2D _leftMainSupportPoint;
	rCoord2D _rightMainSupportPoint;

	QPolygon _contourPolygonBottom;
	QPolygon _contourPolygonTop;

	static iCoord2D invalidICoord2D;
};

#endif // CONTOURSLICE_H
