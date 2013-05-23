#ifndef CURVATUREHISTOGRAM_H
#define CURVATUREHISTOGRAM_H

#include "inc/histogram.h"

class Contour;

class CurvatureHistogram  : public Histogram<qreal>
{
public:
	CurvatureHistogram();
	virtual ~CurvatureHistogram();

	void construct( const Contour &contour , const int &curvatureWidth, const bool &reverse = false );
};

#endif // CURVATUREHISTOGRAM_H
