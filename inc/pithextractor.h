#ifndef MARROWEXTRACTOR_H
#define MARROWEXTRACTOR_H

#include "def/def_billon.h"
#include "def/def_coordinate.h"
#include "inc/define.h"

#include <armadillo>

template <typename T> class QVector;
class Pith;

class PithExtractor {

public:

	PithExtractor( const int &falseCutPercent = FALSE_CUT_PERCENT_BILLON, const int &windowWidth = NEIGHBORHOOD_WINDOW_WIDTH_BILLON,
				   const int &windowHeight = NEIGHBORHOOD_WINDOW_HEIGHT_BILLON, const int &binarizationThreshold = BINARIZATION_THRESHOLD_BILLON,
				   const int &pithLag = PITH_LAG_BILLON );
	~PithExtractor();


	/**
	 * \fn		Pith* process( const icube &image, const int &sliceMin, const int &sliceMax )
	 * \brief	Extrait la moelle d'une matrice sous forme d'une liste des coordonnées des voxels qui la compose
	 * \param	image Image 2D
	 * \param	sliceMin Première coupe à traiter
	 * \param	sliceMax Dernière coupe à traiter
	 */
	void process( Billon &billon ) const;

	/********************************************************
	 * Get functions
	 ********************************************************/
	/**
	 * \fn		int getFalseCutPercent()
	 * \return	le pourcentage de coupes fausses lors de l'extraction de la moelle
	 */
	int falseCutPercent();

	/**
	 * \fn		int getWindowWidth()
	 * \return	la largeur de la fenêtre du voisinage de la moelle
	 */
	int windowWidth();

	/**
	 * \fn		int getWindowHeight()
	 * \return	la hauteur de la fenêtre du voisinage de la moelle
	 */
	int windowHeight();

	/**
	 * \fn		int getBinarizationThreshold()
	 * \return	la valeur minimale en niveau de gris d'un voxel pour le considérer comme noir
	 *
	 * En dessous de ce seuil, ou égal à ce seuil, un voxel est considéré comme blanc.
	 */
	int binarizationThreshold();

	/**
	 * \fn		int getPithLag()
	 * \return	la distance maximal entre deux voxel pour qu'ils soient acceptés comme appartenant à la moelle
	 */
	int pithLag();

	/********************************************************
	 * Set functions
	 ********************************************************/
	/**
	 * \fn		void setFalseCutPercent( int percentage )
	 * \param	percentage Le pourcentage de coupes fausses lors de l'extraction de la moelle
	 */
	void setFalseCutPercent( const int &percentage );

	/**
	 * \fn		void setWindowWidth( int width )
	 * \param	width Largeur de la fenêtre du voisinage de la moelle
	 */
	void setWindowWidth( const int &width );

	/**
	 * \fn		void setWindowHeight( int height )
	 * \param	height Hauteur de la fenêtre du voisinage de la moelle
	 */
	void setWindowHeight( const int &height );

	/**
	 * \fn		void setBinarizationThreshold( int threshold )
	 * \param	threshold Valeur minimale en niveau de gris d'un voxel pour le considérer comme noir
	 *
	 * En dessous de ce seuil, ou égal à ce seuil, un voxel sera considéré comme blanc.
	 */
	void setBinarizationThreshold( const int &threshold );

	/**
	 * \fn		void setPithLag( int lag )
	 * \param	lag Distance maximal entre deux voxel pour qu'ils soient acceptés comme appartenant à la moelle
	 */
	void setPithLag( const int &lag );

private:
	/**
	 * \fn		Coord transHough(const Slice &slice, int width, int height, int *x, int *y, int* z, int *sliceMaxValue, int* nbContourPoints);
	 * \brief	Transformée de Hough sur une coupe
	 * \param	slice Coupe à traiter
	 * \param	width Largeur de la fenetre
	 * \param	height Hauteur de la fenetre
	 * \param	x Coordonnee x de l'origine (coin superieur gauche)
	 * \param	y Coordonnee y de l'origine (coin superieur gauche)
	 * \param	slieceMaxValue Valeur maximale de la coupe
	 * \param	nbContourPoints Nombre de points de contour
	 * \return	les coordonnées de la moelle pour la coupe traitée
	 */
	uiCoord2D transHough( const Slice &slice, int width, int height, int *x, int *y, int *sliceMaxValue, int *nbContourPoints ) const;

	/**
	 * \fn		arma::fmat * contour(const Slice &slice, arma::fmat **orientation);
	 * \brief	Détection des contour
	 * \param	slice Coupe à traiter
	 * \param	orientation Matrice contenant le gradient de chaque point du contour
	 * \return	matrice de contour
	 */
	arma::fmat * contour( const Slice &slice, arma::fmat **orientation ) const;

	/**
	 * \fn		Slice * convolution(const Slice &slice, arma::fcolvec verticalFilter, arma::frowvec horizontalFilter);
	 * \brief	Effectue le produit de convolution d'un filtre separable sur une matrice
	 * \param	slice Coupe à traiter
	 * \param	verticalFilter Vecteur colonne du filtre separable
	 * \param	horizontalFilter Vecteur ligne du filtre separable
	 * \return	matrice apres convolution
	 */
	Slice * convolution( const Slice &slice, arma::fcolvec verticalFilter, arma::frowvec horizontalFilter ) const;

	/**
	 * \fn		int * drawLine(int xOrigine, int yOrigine, float orientation, int width, int height, int *length);
	 * \brief	Trace un un segment d'origine x_origine et y_origine et d'angle orientation
	 * \param	xOrigine Coordonnée x de l'origine du segment
	 * \param	yOrigine Coordonnée y de l'origine du segment
	 * \param	orientation Angle de la droite avec l'axe des abscisses
	 * \param	width Largeur de la coupe sur laquelle on veux tracer le segment
	 * \param	height Hauteur de la coupe sur laquelle on veux tracer le segment
	 * \param	length Longueur du segment
	* \return	un tableau d'entiers contenant l'ensemble des points du segment. Chaque valeur est un indice de la combinaison y*width+x
	 */
	int * drawLine( int xOrigine, int yOrigine, float orientation, int width, int height, int *length ) const;

	/**
	 * \fn		static int floatCompare(const void *first, const void *second);
	 * \brief	Compare deux élement
	 * \param	first Premier élement à comparer
	 * \param	second Deuxième élement à comparer
	 * \return	0 si first == second; une valeur positive si first > second; negative si first < second
	 */
	static int floatCompare( const void *first, const void *second );

	/**
	 * \fn		void minSlice(const Slice &slice, int *minValue, int *maxValue, Coord *coordMax);
	 * \brief	Calcul les valeurs minimum et maximum d'une coupe ainsi que la liste des coordonnées ou apparrait la valeur maximum
	 */
	void minSlice(const Slice &slice, int *minValue, int *maxValue, uiCoord2D *coordMax ) const;

	/**
	 * \fn		void corrigeMoelle(QVector<Coord> *moelle, float *listMax, float seuilHough);
	 * \brief	Corrige les valeur erronées de la moelle
	 */
	void correctPith(QVector<rCoord2D> &pith, float *listMax, float seuilHough ) const;

private:

	int _falseCutPercent;			/*!< Pourcentage de coupes fausses */
	int _windowWidth;				/*!< Largeur de la fenêtre de voisinage */
	int _windowHeight;				/*!< Hauteur de la fenêtre de voisinage */
	float _binarizationThreshold;	/*!< Valeur du seuil de binarisation d'une image en niveau de gris */
	int _pithLag;					/*!< Décalage maximal de la position de la moelle sur 2 coupes consécutives */
};



/**********************************
 * Définition des fonctions inline
 **********************************/
inline
int PithExtractor::falseCutPercent() {
	return _falseCutPercent;
}

inline
int PithExtractor::windowWidth() {
	return _windowWidth;
}

inline
int PithExtractor::windowHeight() {
	return _windowHeight;
}

inline
int PithExtractor::binarizationThreshold() {
	return _binarizationThreshold;
}

inline
int PithExtractor::pithLag() {
	return _pithLag;
}

inline
void PithExtractor::setFalseCutPercent( const int &percentage ) {
	_falseCutPercent = percentage;
}

inline
void PithExtractor::setWindowWidth( const int &width ) {
	_windowWidth = width;
}

inline
void PithExtractor::setWindowHeight( const int &height ) {
	_windowHeight = height;
}

inline
void PithExtractor::setBinarizationThreshold( const int &threshold ) {
	_binarizationThreshold = threshold;
}

inline void PithExtractor::setPithLag( const int &lag ) {
	_pithLag = lag;
}


#endif // MARROWEXTRACTOR_H
