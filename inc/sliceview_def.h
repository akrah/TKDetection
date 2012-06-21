#ifndef SLICEVIEW_DEF_H
#define SLICEVIEW_DEF_H

// Types de coupe possibles
namespace SliceType {
	enum SliceType {
		_SLICE_TYPE_MIN_ = -1,
		CURRENT,
		MOVEMENT,
		EDGE_DETECTION,
		FLOW,
		RESTRICTED_AREA,
		AVERAGE,
		MEDIAN,
		_SLICE_TYPE_MAX_
	};
}

namespace EdgeDetectionType {
	enum EdgeDetectionType {
		_EDGE_DETECTION_MIN_ = -1,
		SOBEL,
		LAPLACIAN,
		_EDGE_DETECTION_MAX_
	};
}

#endif // SLICEVIEW_DEF_H
