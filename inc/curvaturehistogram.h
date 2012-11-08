#ifndef CURVATUREHISTOGRAM_H
#define CURVATUREHISTOGRAM_H

#include "inc/histogram.h"

class ContourCurveSlice;

class CurvatureHistogram  : public Histogram<qreal>
{
public:
	CurvatureHistogram();
	virtual ~CurvatureHistogram();

	const QVector<uint> &dominantPoints() const;

	void clear();
	void construct(const ContourCurveSlice &contour , const int &curvatureWidth );

private:
	QVector<uint> _dominantPointIndex;
};

#endif // CURVATUREHISTOGRAM_H
