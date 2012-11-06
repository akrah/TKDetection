#ifndef CONTOURCURVESLICE_H
#define CONTOURCURVESLICE_H

#include "def/def_coordinate.h"
#include "def/def_billon.h"
#include "inc/coordinate.h"

#include <QVector>
#include <QPolygon>

class QImage;

class ContourCurveSlice
{
public:
	ContourCurveSlice();
	~ContourCurveSlice();

	const QVector<iCoord2D> &contourPoints() const;
	const QVector<iCoord2D> &dominantPoints() const;
	const QVector<iCoord2D> &mainDominantPoints() const;
	int indexOfMainPoint( const uint &number ) const;
	const QVector<iCoord2D> &mainSupportPoints() const;

	void compute( const Slice &initialSlice, Slice &resultSlice, const iCoord2D &sliceCenter, const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius, const iCoord2D &startPoint = iCoord2D(-1,-1) );
	void computeOldMethod( const Slice &initialSlice, Slice &resultSlice, const iCoord2D &sliceCenter, const int &intensityThreshold, const int &smoothingRadius, const iCoord2D &startPoint = iCoord2D(-1,-1) );
	void draw( QImage &image ) const;

private:
	void clear();
	void smoothCurve( int smoothingRadius );
	void extractContourPointsAndDominantPoints( const Slice &initialSlice, const iCoord2D &sliceCenter, const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius, const iCoord2D &startPoint );
	void computeMainDominantPoints( const iCoord2D &sliceCenter );
	void computeContourPolygons();
	void updateSlice( const Slice &initialSlice, Slice &resultSlice, const iCoord2D &sliceCenter, const int &intensityThreshold );

private:
	QVector<iCoord2D> _datasContourPoints;
	QVector<iCoord2D> _datasOriginalContourPoints;
	QVector<iCoord2D> _datasDominantPoints;
	QVector<iCoord2D> _datasMainDominantPoints;
	QVector<int> _datasIndexMainDominantPoints;
	QVector<iCoord2D> _datasMainSupportPoints;

	QPolygon _contourPolygonBottom;
	QPolygon _contourPolygonTop;
};

#endif // CONTOURCURVESLICE_H
