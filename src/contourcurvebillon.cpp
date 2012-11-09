#include "inc/contourcurvebillon.h"

#include "inc/contourcurveslice.h"

ContourCurveBillon::ContourCurveBillon()
{
}

ContourCurveBillon::~ContourCurveBillon()
{
}

const Billon &ContourCurveBillon::knotBillon() const
{
	return _resultBillon;
}

const QVector<ContourCurveSlice> &ContourCurveBillon::contours() const
{
	return _contourCurves;
}

const ContourCurveSlice &ContourCurveBillon::contour( const uint &sliceIndex ) const
{
	Q_ASSERT_X( sliceIndex<static_cast<uint>(_contourCurves.size()) , "ContourCurveBillon::contour", "sliceIndex out of bounds" );
	return _contourCurves[sliceIndex];
}

void ContourCurveBillon::clear()
{
	_contourCurves.clear();
}

bool ContourCurveBillon::isEmpty()
{
	return _contourCurves.isEmpty();
}

void ContourCurveBillon::compute( const Billon &billon, const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius )
{
	_resultBillon = billon;
	_contourCurves.resize(billon.n_slices);
	for ( uint k=0 ; k<billon.n_slices ; ++k )
	{
		_contourCurves[k].compute( _resultBillon.slice(k), billon.slice(k), billon.pithCoord(k), intensityThreshold, blurredSegmentThickness, smoothingRadius );
	}
}
