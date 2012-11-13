#include "inc/contourbillon.h"

#include "inc/contourslice.h"

ContourBillon::ContourBillon()
{
}

ContourBillon::~ContourBillon()
{
}

const Billon &ContourBillon::knotBillon() const
{
	return _resultBillon;
}

const QVector<ContourSlice> &ContourBillon::contours() const
{
	return _contourCurves;
}

const ContourSlice &ContourBillon::contour( const uint &sliceIndex ) const
{
	Q_ASSERT_X( sliceIndex<static_cast<uint>(_contourCurves.size()) , "ContourCurveBillon::contour", "sliceIndex out of bounds" );
	return _contourCurves[sliceIndex];
}

void ContourBillon::clear()
{
	_contourCurves.clear();
}

bool ContourBillon::isEmpty()
{
	return _contourCurves.isEmpty();
}

void ContourBillon::compute( const Billon &billon, const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius )
{
	_resultBillon = billon;
	_contourCurves.resize(billon.n_slices);
	for ( uint k=0 ; k<billon.n_slices ; ++k )
	{
		_contourCurves[k].compute( _resultBillon.slice(k), billon.slice(k), billon.pithCoord(k), intensityThreshold, blurredSegmentThickness, smoothingRadius );
	}
}
