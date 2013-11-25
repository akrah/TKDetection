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
	const QVector<qreal> &lowessData() const;

	void construct( const Billon &tangentialBillon, const KnotPithProfile &knotPithProfile , const qreal & lowessBandWidth );

private:
	QVector<EllipticalAccumulationHistogram> _ellipticalHistograms;
	QVector<qreal> _lowessData;
};

#endif // KNOTELLIPSERADIIHISTOGRAM_H
