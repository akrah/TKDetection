#include "inc/knotellipseradiihistogram.h"

#include "inc/billon.h"
#include "inc/knotpithprofile.h"
#include "inc/lowess.h"

#include <QtGlobal>

KnotEllipseRadiiHistogram::KnotEllipseRadiiHistogram()
{
}

KnotEllipseRadiiHistogram::~KnotEllipseRadiiHistogram()
{
}

const EllipticalAccumulationHistogram & KnotEllipseRadiiHistogram::ellipticalHistogram( const int &sliceIndex ) const
{
	Q_ASSERT_X( sliceIndex<_ellipticalHistograms.size(), QObject::tr("ellipticalHistogram( uint sliceIndex )").toStdString().c_str(), QObject::tr("sliceIndex trop grand").toStdString().c_str() );
	return _ellipticalHistograms[sliceIndex];
}

const QVector<qreal> &KnotEllipseRadiiHistogram::lowessData() const
{
	return _lowessData;
}


void KnotEllipseRadiiHistogram::construct( const Billon &tangentialBillon, const KnotPithProfile &knotPithProfile, const qreal &lowessBandWidth,
										   const uint &ellipticalAccumulationSmoothingRadius, const uint &ellipticalAccumulationMinimumGap )
{
	const uint &nbSlices = tangentialBillon.n_slices;

	this->clear();
	_ellipticalHistograms.clear();

	if ( !nbSlices ) return;

	this->resize(nbSlices);
	_lowessData.resize(nbSlices);
	_ellipticalHistograms.resize(nbSlices);

	qreal ellipticityRate;
	for ( uint k=0 ; k<nbSlices ; ++k )
	{
		EllipticalAccumulationHistogram &ellipticalHistogram = _ellipticalHistograms[k];
		ellipticityRate = (tangentialBillon.voxelWidth()/tangentialBillon.voxelHeight()) / knotPithProfile[k];
		ellipticalHistogram.construct( tangentialBillon.slice(k), tangentialBillon.pithCoord(k), ellipticityRate,
									   ellipticalAccumulationSmoothingRadius, ellipticalAccumulationMinimumGap, (k/(nbSlices*1.0)) );
		(*this)[k] = ellipticalHistogram.detectedRadius();
	}
	(*this)[0] = 0;

	// LOWESS
	Lowess lowess(lowessBandWidth);
	lowess.compute( *this, _lowessData );
}
