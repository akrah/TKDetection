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


void KnotEllipseRadiiHistogram::construct( const Billon &tangentialBillon, const KnotPithProfile &knotPithProfile, const Interval<uint> &validSlices, const qreal &lowessBandWidth,
										   const uint &smoothingRadius )
{
	const uint &nbSlices = tangentialBillon.n_slices;

	this->clear();
	_ellipticalHistograms.clear();

	if ( !nbSlices ) return;

	this->resize(nbSlices);
	_lowessData.resize(nbSlices);
	_ellipticalHistograms.resize(nbSlices);

	const int &firstValidSliceIndex = validSlices.min();
	const int &lastValidSliceIndex = validSlices.max();

	qreal ellipticityRate;
	for ( int k=firstValidSliceIndex ; k<=lastValidSliceIndex ; ++k )
	{
		EllipticalAccumulationHistogram &ellipticalHistogram = _ellipticalHistograms[k];
		ellipticityRate = (tangentialBillon.voxelWidth()/tangentialBillon.voxelHeight()) / knotPithProfile[k];
		ellipticalHistogram.construct( tangentialBillon.slice(k), tangentialBillon.pithCoord(k), ellipticityRate,
									   smoothingRadius, k/static_cast<qreal>(nbSlices) );
		(*this)[k] = ellipticalHistogram.detectedRadius();
	}

	extrapolation(validSlices);

	// LOWESS
	Lowess lowess(lowessBandWidth);
	lowess.compute( *this, _lowessData );
}

void KnotEllipseRadiiHistogram::extrapolation( const Interval<uint> &validSlices )
{
	const int size = this->size();

	if ( !size ) return;

	const int firstValidSliceIndex = validSlices.min();
	const int lastValidSliceIndex = validSlices.max();
	const qreal firstValidValueIncrement = (*this)[firstValidSliceIndex]/static_cast<qreal>(firstValidSliceIndex);
	const qreal lastValidIncrement = qMin(qAbs(((*this)[lastValidSliceIndex] - (*this)[lastValidSliceIndex-2])),
										  qAbs(((*this)[lastValidSliceIndex-1] - (*this)[lastValidSliceIndex-3])));

	int k;
	for ( k=0 ; k<firstValidSliceIndex; ++k )
	{
		(*this)[k] = firstValidValueIncrement*static_cast<qreal>(k);
	}

	for ( k=lastValidSliceIndex+1 ; k<size; ++k )
	{
		(*this)[k] = (*this)[k-1] + lastValidIncrement;
	}
}
