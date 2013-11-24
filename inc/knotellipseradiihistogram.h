#ifndef KNOTELLIPSERADIIHISTOGRAM_H
#define KNOTELLIPSERADIIHISTOGRAM_H

#include "def/def_billon.h"
#include "def/def_coordinate.h"

#include "inc/histogram.h"
#include "inc/ellipticalaccumulationhistogram.h"

class KnotPithProfile;

class KnotEllipseRadiiHistogram : public Histogram<qreal>
{
public:
	KnotEllipseRadiiHistogram();
	virtual ~KnotEllipseRadiiHistogram();

	const EllipticalAccumulationHistogram & ellipticalHistogram( const int & sliceIndex ) const;

	void construct( const Billon &tangentialBillon, const KnotPithProfile &knotPithProfile );

private:
	QVector<EllipticalAccumulationHistogram> _ellipticalHistograms;
};

#endif // KNOTELLIPSERADIIHISTOGRAM_H
