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


void KnotEllipseRadiiHistogram::construct( const Billon &tangentialBillon, const KnotPithProfile &knotPithProfile, const qreal &lowessBandWidth )
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
		ellipticityRate = (tangentialBillon.voxelWidth()/tangentialBillon.voxelHeight()) / qCos(knotPithProfile[k]);
		ellipticalHistogram.construct( tangentialBillon.slice(k), tangentialBillon.pithCoord(k), ellipticityRate );
		(*this)[k] = ellipticalHistogram.detectedRadius();
	}

	// LOWESS
	QVector<qreal> residus;
	Lowess lowess(lowessBandWidth);
	lowess.compute( *this, _lowessData, residus );
	outlierInterpolation( residus, 1.0 );
	//lowess.compute( *this, _lowessData, residus );
}

void KnotEllipseRadiiHistogram::outlierInterpolation( const QVector<qreal> &residus, const qreal &iqrCoeff )
{
	const int &size = this->size();

	QVector<qreal> sortedResidus(residus);
	qSort(sortedResidus);
	const qreal &q1 = sortedResidus[size/4];
	const qreal &q3 = sortedResidus[3*size/4];

	const Interval<qreal> outlierInterval( q1-iqrCoeff*(q3-q1), q3+iqrCoeff*(q3-q1) );

//	int startSliceIndex, newK, startSliceIndexMinusOne;
//	qreal interpolationStep, currentInterpolatePithCoord;

	for ( int k=0 ; k<size ; ++k )
	{
		if ( !outlierInterval.containsOpen(residus[k]) )
		{
			(*this)[k] = _lowessData[k];
//			startSliceIndex = k++;
//			startSliceIndexMinusOne = startSliceIndex?startSliceIndex-1:0;

//			while ( k<size && !outlierInterval.containsOpen(residus[k]) ) ++k;
//			if ( k<size ) --k;
//			k = qMin(k,size-1);

//			std::cout << "Outlier interpolation [" << startSliceIndex << ", " << k << "]" << std::endl;

//			interpolationStep = startSliceIndex && k<size-1 ? ((*this)[k+1] - (*this)[startSliceIndexMinusOne]) / static_cast<qreal>( k+1-startSliceIndexMinusOne )
//															   : 0.;

//			currentInterpolatePithCoord = interpolationStep +
//										  (startSliceIndex || k==size ? (*this)[startSliceIndexMinusOne]
//																	  : (*this)[k+1]);

//			for ( newK = startSliceIndex ; newK <= k ; ++newK, currentInterpolatePithCoord += interpolationStep )
//			{
//				(*this)[newK] = currentInterpolatePithCoord;
//			}
		}
	}
}
