#ifndef DEFINE_H
#define DEFINE_H

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
#define DEFAULT_MASK_RADIUS 2
#define DEFAULT_MINIMUM_WIDTH_OF_NEIGHBORHOOD 10
#define DEFAULT_MINIMUM_WIDTH_OF_INTERVALS 10
#define DEFAULT_PERCENTAGE_FOR_MAXIMUM_CANDIDATE 30

// Paramètres d'intensité
#define MINIMUM_INTENSITY -900
#define MAXIMUM_INTENSITY 530

// Paramètres de z-mouvement
#define MINIMUM_Z_MOTION 200
#define MAXIMUM_Z_MOTION 500

// Paramètres de détection de la moelle
#define FALSE_CUT_PERCENT 20
#define NEIGHBORHOOD_WINDOW_WIDTH 59
#define NEIGHBORHOOD_WINDOW_HEIGHT NEIGHBORHOOD_WINDOW_WIDTH
#define BINARIZATION_THRESHOLD 90
#define PITH_LAG NEIGHBORHOOD_WINDOW_WIDTH/2

// Paramètres de flots optiques
#define FLOW_ALPHA_DEFAULT 7
#define FLOW_EPSILON_DEFAULT 0.001f
#define FLOW_MAXIMUM_ITERATIONS 100

/*************************************
 * Enumerations
 *************************************/

	// Types de coupe possibles
	enum SliceType
	{
		_SLICE_TYPE_MIN_ = -1,
		CURRENT,
		MOVEMENT,
		EDGE_DETECTION,
		FLOW,
		RESTRICTED_AREA,
		_SLICE_TYPE_MAX_
	};

	// Type de detection de contours
	enum EdgeDetectionType
	{
		_EDGE_DETECTION_MIN_ = -1,
		SOBEL,
		LAPLACIAN,
		CANNY,
		_EDGE_DETECTION_MAX_
	};

	// Type d'export OFS
	enum OfsExportType
	{
		_OFS_EXPORT_TYPE_MIN_ = -1,
		PITH,
		BILLON_RESTRICTED_AREA,
		CURENT_ANGULAR_SECTOR_LARGE_AREA,
		ALL_ANGULAR_SECTORS_ALL_SLICE_INTERVALS_LARGE_AREA,
		_OFS_EXPORT_TYPE_MAX_
	};

	// Type d'histogram
	enum HistogramType
	{
		_HISTOGRAM_TYPE_MIN_ = -1,
		SLICE_HISTOGRAM,
		SECTOR_HISTOGRAM,
		PITH_KNOT_DISTANCE_HISTOGRAM,
		_HISTOGRAM_TYPE_MAX_
	};
}

#endif // DEFINE_H
