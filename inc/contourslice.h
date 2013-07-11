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
	const iCoord2D &maxConcavityPoint() const;
	const iCoord2D &minConcavityPoint() const;
	const int &maxConcavityPointIndex() const;
	void setMaxConcavityPointIndex( int maxConcavityIndex );
	const int &minConcavityPointIndex() const;
	void setMinConcavityPointIndex( int maxConcavityIndex );
	const rCoord2D &maxSupportPoint() const;
	void setMaxSupportPoint( const rCoord2D &maxSupportPoint );
	const rCoord2D &minSupportPoint() const;
	void setMinSupportPoint( const rCoord2D &minSupportPoint );
	const uiCoord2D &sliceCenter() const;

	void compute( Slice &resultSlice, const Slice &initialSlice, const uiCoord2D &sliceCenter, const int &intensityThreshold,
				  const int &smoothingRadius, const int &curvatureWidth, const qreal &curvatureThreshold, const iCoord2D &startPoint = iCoord2D(-1,-1) );

	void draw( QPainter &painter, const int &cursorPosition, const TKD::ViewType &viewType ) const;

private:
	void computeConcavityPoints( const qreal &curvatureThreshold );
	void computeSupportPoints( const int &meansMaskSize );
	void computeContourPolygons();
	void updateSlice( const Slice &initialSlice, Slice &resultSlice, const int &intensityThreshold );

private:
	Contour _contour;
	Contour _originalContour;
	uiCoord2D _sliceCenter;

	ContourDistancesHistogram _contourDistancesHistogram;
	CurvatureHistogram _curvatureHistogram;

	int _maxConcavityPointsIndex;
	int _minConcavityPointsIndex;
	rCoord2D _maxSupportPoint;
	rCoord2D _minSupportPoint;

	QPolygon _contourPolygonBottom;
	QPolygon _contourPolygonTop;
};

#endif // CONTOURSLICE_H
