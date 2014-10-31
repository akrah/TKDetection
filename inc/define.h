#ifndef DEFINE_H
#define DEFINE_H

#include <QtGlobal>

namespace TKD
{
/*************************************
 * Constantes d'angles
 *************************************/
#define PI					(4.*atan(1.0))
#define TWO_PI				(2.*PI)
#define PI_ON_TWO			(PI/2.)
#define PI_ON_THREE			(PI/3.)
#define PI_ON_FOUR			(PI/4.)
#define PI_ON_EIGHT			(PI/8.)
#define THREE_PI_ON_TWO		(3.*PI_ON_TWO)
#define THREE_PI_ON_FOUR	(3.*PI_ON_FOUR)
#define TWO_PI_ON_THREE		(2.*PI_ON_THREE)
#define SEVEN_PI_ON_EIGHT	(7.*PI_ON_EIGHT)
#define DEG_TO_RAD_FACT		(PI/180.)
#define RAD_TO_DEG_FACT		(180./PI)

/*************************************
 * Paramètres par défaut
 *************************************/

// Paramètres de détection des maximums et intervalles dans un histogramme
#define HISTOGRAM_SMOOTHING_RADIUS 8
#define HISTOGRAM_PERCENTAGE_OF_MINIMUM_HEIGHT_OF_MAXIMUM 10
#define HISTOGRAM_MINIMUM_WIDTH_OF_INTERVALS 10
#define HISTOGRAM_DERIVATIVE_SEARCH_PERCENTAGE 30
#define HISTOGRAM_PERCENTAGE_OF_SLICES_TO_IGNORE 5

#define HISTOGRAM_ANGULAR_SMOOTHING_RADIUS 8
#define HISTOGRAM_ANGULAR_PERCENTAGE_OF_MINIMUM_HEIGHT_OF_MAXIMUM 15
#define HISTOGRAM_ANGULAR_MINIMUM_WIDTH_OF_INTERVALS 10
#define HISTOGRAM_ANGULAR_DERIVATIVE_SEARCH_PERCENTAGE 50
#define HISTOGRAM_ANGULAR_INTERVAL_GAP 0

#define HISTOGRAM_DISTANCE_SMOOTHING_RADIUS 5
#define HISTOGRAM_DISTANCE_PERCENTAGE_OF_MINIMUM_HEIGHT_OF_MAXIMUM 75
#define HISTOGRAM_DISTANCE_MINIMUM_WIDTH_OF_INTERVALS 5
#define HISTOGRAM_DISTANCE_DERIVATIVE_SEARCH_PERCENTAGE 75

// Paramètres d'intensité
#define MINIMUM_INTENSITY -900
#define MAXIMUM_INTENSITY 530

// Paramètres de z-mouvement
#define MINIMUM_Z_MOTION 200
#define MAXIMUM_Z_MOTION 500

// Paramètres de détection de la moelle
#define FALSE_CUT_PERCENT_BILLON 25
#define BINARIZATION_THRESHOLD_BILLON 90
#define NEIGHBORHOOD_WINDOW_WIDTH_BILLON 59
#define NEIGHBORHOOD_WINDOW_HEIGHT_BILLON NEIGHBORHOOD_WINDOW_WIDTH_BILLON
#define PITH_LAG_BILLON 29
#define PITH_SMOOTHING_BILLON 10
#define MIN_WOOD_PERCENTAGE_BILLON 25
#define ASCENDING_ORDER_BILLON true
#define FIRST_VALID_SLICES_TO_EXTRAPOLATE_BILLON 0
#define LAST_VALID_SLICES_TO_EXTRAPOLATE_BILLON 0

#define NEIGHBORHOOD_WINDOW_WIDTH_KNOT 25
#define NEIGHBORHOOD_WINDOW_HEIGHT_KNOT NEIGHBORHOOD_WINDOW_WIDTH_KNOT
#define PITH_LAG_KNOT 10
#define PITH_SMOOTHING_KNOT 10
#define MIN_WOOD_PERCENTAGE_KNOT 90
#define ASCENDING_ORDER_KNOT false
#define FIRST_VALID_SLICES_TO_EXTRAPOLATE_KNOT 20
#define LAST_VALID_SLICES_TO_EXTRAPOLATE_KNOT 1

// Paramètres de la zone restreinte autour de la moelle
#define RESTRICTED_AREA_DEFAULT_RESOLUTION 100
#define RESTRICTED_AREA_DEFAULT_THRESHOLD -900
#define RESTRICTED_AREA_DEFAULT_RADIUS 133.33

/*************************************
 * Enumerations
 *************************************/

	// Types de coupe possibles
	enum ViewType
	{
		_VIEW_TYPE_MIN_ = -1,
		CLASSIC,
		Z_MOTION,
		HOUGH,
		_VIEW_TYPE_MAX_
	};

	// Type d'export OFS
	enum OfsExportType
	{
		_OFS_EXPORT_TYPE_MIN_ = -1,
		COMPLETE_PITH,
		PITH_ON_CURRENT_SLICE_INTERVAL,
		COMPLETE_BILLON,
		ALL_KNOT_AREAS,
		ALL_KNOT_AREAS_OF_CURRENT_SLICE_INTERVAL,
		CURRENT_KNOT_AREA,
		_OFS_EXPORT_TYPE_MAX_
	};

	// Type d'histogram
	enum HistogramType
	{
		_HISTOGRAM_TYPE_MIN_ = -1,
		SLICE_HISTOGRAM,
		SECTOR_HISTOGRAM,
		_HISTOGRAM_TYPE_MAX_
	};

	// Type de vue
	enum ProjectionType
	{
		_PROJECTION_TYPE_MIN_ = -1,
		Z_PROJECTION,
		Y_PROJECTION,
		POLAR_PROJECTION,
		ELLIPTIC_PROJECTION,
		CYLINDRIC_PROJECTION,
		_PROJECTION_TYPE_MAX_
	};

	// Type d'extrapolation
	enum ExtrapolationType
	{
		_EXTRAPOLATION_TYPE_MIN_ = -1,
		NO_EXTRAPOLATION,
		LINEAR,
		SLOPE_DIRECTION,
		_EXTRAPOLATION_TYPE_MAX_
	};


/*************************************
 * Enumerations
 *************************************/

	struct HistogramParameters
	{
		uint smoothingRadius;
		uint minimumHeightPercentageOfMaximum;
		uint neighborhoodOfMaximums;
		uint derivativesPercentage;
		uint minimumWidthOfIntervals;
		bool loop;
	};
}

#endif // DEFINE_H
