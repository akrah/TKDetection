#ifndef OPTIONS_H
#define OPTIONS_H

#include "define.h"

/*********************************************************
 * Déclaration des strctures pour les paramètres des algos
 * *******************************************************/
struct Params_Global {
	int minIntensity = MINIMUM_INTENSITY;
	int maxIntensity = MAXIMUM_INTENSITY;
	int minZMotion = MINIMUM_Z_MOTION;
} p_global;

struct Params_PithDetectionBillon {
	int subWindowWidth = NEIGHBORHOOD_WINDOW_WIDTH_BILLON;
	int subWindowHeight = NEIGHBORHOOD_WINDOW_HEIGHT_BILLON;
	int pithShift = PITH_LAG_BILLON;
	int smoothingRadius = PITH_SMOOTHING_BILLON;
	int minimumWoodPercentage = MIN_WOOD_PERCENTAGE_BILLON;
	bool useAscendingOrder = ASCENDING_ORDER_BILLON;
	int extrapolationType = TKD::LINEAR;
	int percentageOfFirstValidSlicesToExtrapolate = 0;
	int percentageOfLastValidSlicesToExtrapolate = 0;
} p_pithDetectionBillon;

struct Params_RestrictedArea {
	int nbDirections = RESTRICTED_AREA_DEFAULT_RESOLUTION;
	int barkValue = MINIMUM_INTENSITY;
	int minRadius = 5;
} p_restrictedArea;

struct Params_SliceHistogram {
	int smoothingRadius = HISTOGRAM_SMOOTHING_RADIUS;
	int minimumHeightOfMaximums = HISTOGRAM_PERCENTAGE_OF_MINIMUM_HEIGHT_OF_MAXIMUM;
	int derivativeSearchPercentage = HISTOGRAM_DERIVATIVE_SEARCH_PERCENTAGE;
	int minimumWidthOfIntervals = HISTOGRAM_MINIMUM_WIDTH_OF_INTERVALS;
} p_sliceHistogram;

struct Params_SectorHistogram {
	int smoothingRadius = HISTOGRAM_ANGULAR_SMOOTHING_RADIUS;
	int minimumHeightOfMaximums = HISTOGRAM_ANGULAR_PERCENTAGE_OF_MINIMUM_HEIGHT_OF_MAXIMUM;
	int derivativeSearchPercentage = HISTOGRAM_ANGULAR_DERIVATIVE_SEARCH_PERCENTAGE;
	int minimumWidthOfIntervals = HISTOGRAM_ANGULAR_MINIMUM_WIDTH_OF_INTERVALS;
} p_sectorHistogram;

struct Params_PithDetectionKnot {
	int subWindowWidth = NEIGHBORHOOD_WINDOW_WIDTH_KNOT;
	int subWindowHeight = NEIGHBORHOOD_WINDOW_HEIGHT_KNOT;
	int pithShift = PITH_LAG_KNOT;
	int smoothingRadius = PITH_SMOOTHING_KNOT;
	int minimumWoodPercentage = MIN_WOOD_PERCENTAGE_KNOT;
	bool useAscendingOrder = ASCENDING_ORDER_KNOT;
	int extrapolationType = TKD::SLOPE_DIRECTION;
	int percentageOfFirstValidSlicesToExtrapolate = FIRST_VALID_SLICES_TO_EXTRAPOLATE_KNOT;
	int percentageOfLastValidSlicesToExtrapolate = LAST_VALID_SLICES_TO_EXTRAPOLATE_KNOT;
} p_pithDetectionKnot;

struct Params_TangentialGenerator {
	bool useTrilinearInterpolation = true;
} p_tangentialGenerator;

struct Params_PithProfile {
	int smoothingRadius = 2;
} p_pithProfile;

struct Params_EllipticalHistograms {
	int lowessBandWidth = 33;
	int lowessIqrCoefficient  = 1;
	int lowessPercentageOfFirstValidSlicesToExtrapolate = 5;
	int lowessPercentageOfLastValidSlicesToExtrapolate = 0;
	int smoothingRadius = 0;
} p_ellipticalHistograms;

#endif // OPTIONS_H
