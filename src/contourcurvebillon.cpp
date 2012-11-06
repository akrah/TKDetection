#include "inc/contourcurvebillon.h"

#include "inc/contourcurveslice.h"

ContourCurveBillon::ContourCurveBillon(const Billon &billon) : _initialBillon(billon), _resultBillon(billon)
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

void ContourCurveBillon::compute( const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius )
{
	_contourCurves.resize(_initialBillon.n_slices);
	for ( uint k=0 ; k<_initialBillon.n_slices ; ++k )
	{
		_contourCurves[k].compute(_initialBillon.slice(k), _resultBillon.slice(k), _resultBillon.pithCoord(k), intensityThreshold, blurredSegmentThickness, smoothingRadius );
	}
}
