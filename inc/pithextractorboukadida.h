#ifndef PITHEXTRACTORBOUKADIDA_H
#define PITHEXTRACTORBOUKADIDA_H

#include "def/def_billon.h"
#include "def/def_coordinate.h"
#include "inc/define.h"
#include "inc/interval.h"

#include <armadillo>

template <typename T> class QVector;
class Pith;

class PithExtractorBoukadida
{

public:

	PithExtractorBoukadida(const int &subWindowWidth = NEIGHBORHOOD_WINDOW_WIDTH_BILLON, const int &subWindowHeight = NEIGHBORHOOD_WINDOW_HEIGHT_BILLON,
							const qreal &pithShift = PITH_LAG_BILLON, const uint &smoothingRadius = PITH_SMOOTHING_BILLON,
							const qreal &minWoodPercentage = MIN_WOOD_PERCENTAGE_BILLON,
							const Interval<int> &intensityInterval = Interval<int>(MINIMUM_INTENSITY,MAXIMUM_INTENSITY),
							const bool &ascendingOrder = ASCENDING_ORDER_BILLON, const TKD::ExtrapolationType &extrapolationType = TKD::LINEAR,
							const qreal &firstValidSliceToExtrapolate = FIRST_VALID_SLICES_TO_EXTRAPOLATE_BILLON, const qreal &lastValidSliceToExtrapolate = LAST_VALID_SLICES_TO_EXTRAPOLATE_BILLON );

	~PithExtractorBoukadida();

	/**
	 * \fn		Pith* process( const icube &image, const int &sliceMin, const int &sliceMax )
	 * \brief	Extrait la moelle d'une matrice sous forme d'une liste des coordonnées des voxels qui la compose.
	 *          Cette classe implémente l'algorithme proposé par Boukadida et al., 2012.
	 * \param	image Image 2D
	 * \param	sliceMin Première coupe à traiter
	 * \param	sliceMax Dernière coupe à traiter
	 */
	void process( Billon &billon , const bool &adaptativeWidth = false );

	/********************************************************
	 * Get functions
	 ********************************************************/
	/**
	 * \fn		int subWindowWidth()
	 * \return	la largeur de la fenêtre du voisinage de la moelle
	 */
	int subWindowWidth() const;

	/**
	 * \fn		int subWindowHeight()
	 * \return	la hauteur de la fenêtre du voisinage de la moelle
	 */
	int subWindowHeight() const;

	/**
	 * \fn		int pithShift()
	 * \return	Décalage autorisé de la position de la moelle entre deux coupes consécutives
	 */
	qreal pithShift() const;

	/**
	 * \fn		uint smoothingRadius()
	 * \return	Rayon de lissage de la moelle (en nombre de coupes)
	 */
	uint smoothingRadius() const;

	/**
	 * \fn		qreal minPercentageOfWood()
	 * \return	Pourcentage de bois minimum présent sur une coupe pour la considérer comme valide
	 */
	qreal minWoodPercentage() const;

	/**
	 * \fn		Interval<int> intensityInterval()
	 * \return	Interval d'intensité à prednre en compte pour la transformée de Hough
	 */
	Interval<int> intensityInterval() const;

	/**
	 * \fn		bool ascendingOrder();
	 * \return	True if pith extraction on slices taken in ascending order, false otherwise
	 */
	bool ascendingOrder() const;

	/**
	 * \fn		bool extrapolation()
	 * \return	True if slices outside the valid interval could be interpolated
	 */
	TKD::ExtrapolationType extrapolation() const;

	/**
	 * \fn		const Interval<uint> &validSlices()
	 * \return	The interval of valid slices
	 */
	const Interval<uint> &validSlices() const;

	/**
	 * \fn		const qreal &firstValidSlicesToExtrapolate()
	 * \return	The percentage of valid slices to Extrapolate at the pith beginning
	 */
	const uint &firstValidSlicesToExtrapolate() const;

	/**
	 * \fn		const qreal &lasttValidSlicesToExtrapolate()
	 * \return	The percentage of valid slices to Extrapolate at the pith ending
	 */
	const uint &lastValidSlicesToExtrapolate() const;

	/********************************************************
	 * Set functions
	 ********************************************************/

public:
	/**
	 * \fn		void setSubWindowWidth( int width )
	 * \param	width Largeur de la fenêtre du voisinage de la moelle
	 */
	void setSubWindowWidth( const int &width );

	/**
	 * \fn		void setSubWindowHeight( int height )
	 * \param	height Hauteur de la fenêtre du voisinage de la moelle
	 */
	void setSubWindowHeight( const int &height );

	/**
	 * \fn		void setPithShift( const uint &shift )
	 * \param	shift Décalage autorisé de la position de la moelle entre deux coupes consécutives
	 */
	void setPithShift( const qreal &shift );

	/**
	 * \fn		void setSmoothingRadius( const uint &radius )
	 * \param	radius Rayon de lissage de la moelle (en nombre de coupes)
	 */
	void setSmoothingRadius( const uint &radius );

	/**
	 * \fn		void setMinWoodPercentage( const qreal &percentage )
	 * \param	percentage Pourcentage de bois minimum présent sur une coupe pour la considérer comme valide
	 */
	void setMinWoodPercentage( const qreal &percentage );

	/**
	 * \fn		void setIntensityInterval( const Interval<int> &interval )
	 * \param	interval Interval d'intensité à prednre en compte pour la transformée de Hough
	 */
	void setIntensityInterval( const Interval<int> &interval );

	/**
	 * \fn		void setAscendingOrder( const bool &order );
	 * \param	order True if pith extraction on slices taken in ascending order, false otherwise
	 */
	void setAscendingOrder( const bool &order );

	/**
	 * \fn		void setExtrapolation( const TKD::ExtrapolationType &extrapolationType );
	 * \param	extrapolationType Extrapolation type of slices outside the valid slice interval
	 */
	void setExtrapolation( const TKD::ExtrapolationType &extrapolationType );

	/**
	 * \fn		void setFirstValidSlicesToExtrapolate( const qreal &percentOfSlices )
	 * \param	percentOfSlices	Percentage of valid slices to Extrapolate at the pith beginning
	 */
	void setFirstValidSlicesToExtrapolate( const uint &percentOfSlices );

	/**
	 * \fn		void setLastValidSlicesToExtrapolate( const qreal &percentOfSlices )
	 * \param	percentOfSlices	Percentage of valid slices to Extrapolate at the pith ending
	 */
	void setLastValidSlicesToExtrapolate( const uint &percentOfSlices );

private:
	/**
	 * \fn		Coord transHough(const Slice &slice, uint &nbContourPoints );
	 * \brief	Transformée de Hough sur une coupe
	 * \param	slice Coupe à traiter
	 * \param   nbContourPoints nombre de points utilisés pour tracer les droites
	 * \return	les coordonnées de la moelle pour la coupe traitée
	 */
	uiCoord2D transHough( const Slice &slice, qreal & lineOnMaxRatio, const rCoord2D &voxelDims, const qreal &adaptativeWidthCoeff = 1.0 ) const;

	/**
	 * \fn		uint contour( const Slice &slice, arma::Mat<qreal> &orientations )
	 * \brief	Calcul de l'orientation en chaque pixel de slice avec les filtres de Sobel
	 * \param	slice Coupe dont on veut les orientations
	 * \param	orientations Coupe contenant l'orientation en chaque pixel (pour les pixels d'orientation supérieure à l'orientation médiane)
	 * \param   hasContour Matrice de booleen qui précise pour chaque pixel de Slice s'il est un point de contour
	 * \return	le nombre d'orientations prises en compte
	 */
	uint accumulation( const Slice &slice, arma::Mat<qreal> &orientations,
					   arma::Mat<int> &accuSlice, const rCoord2D &voxelDims ) const;

	void drawLine(arma::Mat<int> &slice, const iCoord2D &origin, const qreal &orientationOrig ) const;

	void interpolation( Pith &pith, const QVector<qreal> &nbLineByMaxRatio , const Interval<uint> &sliceIntervalToInterpolate ) const;

	void fillBillonBackground( Billon &billonToFill, QVector<qreal> &backgroundProportions, const Interval<int> &intensityInterval, const bool &adaptativeWidth ) const;

	void detectValidSliceInterval( const QVector<qreal> &backgroundProportions );

private:
	int _subWindowWidth;					/*!< Largeur de la fenêtre de voisinage */
	int _subWindowHeight;					/*!< Hauteur de la fenêtre de voisinage */
	qreal _pithShift;						/*!< Décalage autorisé de la moelle entre deux coupes consécutives */
	uint _smoothingRadius;					/*!< Rayon de lissage de la moelle (en nombre de coupes) */
	qreal _minWoodPercentage;				/*!< Proportion de bois minimum sur une coupe pour la considérer comme valide */
	Interval<int> _intensityInterval;		/*!< Interval d'intensité à prednre en compte pour la transformée de Hough */

	bool _ascendingOrder;					/*!< Ordre de traitement des coupes (ascending order if true, descending order otherwise */
	TKD::ExtrapolationType _extrapolation;	/*!< Extrapoler la position de la moelle en dehors de l'intervalle de coupes valides */

	Interval<uint> _validSlices;			/*!< Intervalle de coupes valides */
	uint _firstValidSliceToExtrapolate;		/*!< Poucentage de coupes valides à interpeler en début de moelle */
	uint _lastValidSliceToExtrapolate;		/*!< Poucentage de coupes valides à interpeler en fin de moelle */
};

#endif // PITHEXTRACTORBOUKADIDA_H
