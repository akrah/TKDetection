#include "inc/contourcurvebillon.h"

#include "inc/contourcurveslice.h"

ContourCurveBillon::ContourCurveBillon(const Billon &billon) : _initialBillon(billon)
{
}

void ContourCurveBillon::intialize( const int &intensityThreshold, const int &blurredSegmentThickness, const int &smoothingRadius )
{
	_contourCurves.clear();
	_resultBillon = _initialBillon;

	for ( uint k=0 ; k<_initialBillon.n_slices ; ++k )
	{
		ContourCurveSlice contourSlice(_initialBillon.slice(k));
		contourSlice.compute( _resultBillon.slice(k), _resultBillon.pithCoord(k), intensityThreshold, blurredSegmentThickness, smoothingRadius );
		_contourCurves.append(contourSlice);
	}
}
