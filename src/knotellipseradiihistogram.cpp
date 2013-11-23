#include "inc/knotellipseradiihistogram.h"

#include "inc/billon.h"
#include "inc/knotpithprofile.h"

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

void KnotEllipseRadiiHistogram::construct( const Billon &tangentialBillon, const KnotPithProfile &knotPithProfile )
{
	const uint &nbSlices = tangentialBillon.n_slices;

	this->clear();
	_ellipticalHistograms.clear();

	if ( !nbSlices ) return;

	this->resize(nbSlices);
	_ellipticalHistograms.resize(nbSlices);

	qreal ellipticityRate;
	for ( uint k=0 ; k<nbSlices ; ++k )
	{
		EllipticalAccumulationHistogram &ellipticalHistogram = _ellipticalHistograms[k];
		ellipticityRate = (tangentialBillon.voxelWidth()/tangentialBillon.voxelHeight())
						  / qCos(knotPithProfile[k]);
		ellipticalHistogram.construct( tangentialBillon.slice(k), tangentialBillon.pithCoord(k), ellipticityRate );
		(*this)[k] = ellipticalHistogram.detectedRadius();
	}
}
