#ifndef SLICEHISTOGRAM_DEF_H
#define SLICEHISTOGRAM_DEF_H

// Types de coupe possibles
namespace HistogramIntervalType {
	enum HistogramIntervalType {
		_HIST_INTERVAL_TYPE_MIN_ = -1,
		FROM_EDGE,
		FROM_MEANS,
		FROM_MEDIAN,
		FROM_MIDDLE_OF_MEANS_AND_MEDIAN,
		_HIST_INTERVAL_TYPE_MAX__
	};
}

#endif // SLICEHISTOGRAM_DEF_H
