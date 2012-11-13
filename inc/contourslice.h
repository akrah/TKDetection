#ifndef CONTOURSLICE_H
#define CONTOURSLICE_H

#include "def/def_coordinate.h"
#include "def/def_billon.h"
#include "inc/coordinate.h"

#include <QVector>
#include <QPolygon>

class QImage;

class ContourSlice
{
public:
	ContourSlice();
	~ContourSlice();

	const QVector<iCoord2D> &contourPoints() const;
	const iCoord2D &dominantPoint( const uint &index ) const;
	const QVector<uint> &dominantPointIndex() const;
	const iCoord2D &leftMainDominantPoint() const;
	const iCoord2D &rightMainDominantPoint() const;
	const int &leftMainDominantPointIndex() const;
	const int &rightMainDominantPointIndex() const;
	const rCoord2D &leftMainSupportPoint() const;
	const rCoord2D &rightMainSupportPoint() const;

	void compute( Slice &resultSlice, const Slice &initialSlice, const iCoord2D &sliceCenter, const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius, const iCoord2D &startPoint = iCoord2D(-1,-1) );
	void computeOldMethod( Slice &resultSlice, const Slice &initialSlice, const iCoord2D &sliceCenter, const int &intensityThreshold, const int &smoothingRadius, const iCoord2D &startPoint = iCoord2D(-1,-1) );
	void draw(QImage &image , const int &cursorPosition = -1 ) const;

private:
	void clear();
	void smoothCurve(QVector<iCoord2D> &curve, int smoothingRadius );
	void extractContourPointsAndDominantPoints( const Slice &initialSlice, const iCoord2D &sliceCenter, const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius, const iCoord2D &startPoint );
	void computeMainDominantPoints( const iCoord2D &sliceCenter );
	void computeContourPolygons();
	void updateSlice( const Slice &initialSlice, Slice &resultSlice, const iCoord2D &sliceCenter, const int &intensityThreshold );

private:
	QVector<iCoord2D> _datasContourPoints;
	QVector<iCoord2D> _datasOriginalContourPoints;
	QVector<uint> _datasDominantPointsIndex;
	int _datasLeftMainDominantPointsIndex;
	int _datasRightMainDominantPointsIndex;
	rCoord2D _datasLeftMainSupportPoint;
	rCoord2D _datasRightMainSupportPoint;

	QPolygon _contourPolygonBottom;
	QPolygon _contourPolygonTop;

	static iCoord2D invalidICoord2D;
};

#endif // CONTOURSLICE_H
