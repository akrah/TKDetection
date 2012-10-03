#ifndef MARROWEXTRACTOR_H
#define MARROWEXTRACTOR_H

#include <QList>

#include <armadillo>

class Marrow;

class MarrowExtractor {

public:
	MarrowExtractor();

	/**
	 * \fn		QList<Coord>* process( const icube &image, const int &sliceMin, const int &sliceMax )
	 * \brief	Extrait la moelle d'une matrice sous forme d'une liste des coordonnées des voxels qui la compose
	 * \param	image Image 2D
	 * \param	sliceMin Première coupe à traiter
	 * \param	sliceMax Dernière coupe à traiter
	 * \return	la liste des coordonnées de la moelle
	 */
	Marrow* process( const arma::icube &image, const int &sliceMin, const int &sliceMax );

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
	 * \fn		int getMarrowLag()
	 * \return	la distance maximal entre deux voxel pour qu'ils soient acceptés comme appartenant à la moelle
	 */
	int marrowLag();

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
	 * \fn		void setMarrowLag( int lag )
	 * \param	lag Distance maximal entre deux voxel pour qu'ils soient acceptés comme appartenant à la moelle
	 */
	void setMarrowLag( const int &lag );

private:
	/**
	 * \fn		Coord transHough(const arma::imat &slice, int width, int height, int *x, int *y, int* z, int *sliceMaxValue, int* nbContourPoints);
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
	iCoord2D transHough( const arma::imat &slice, int width, int height, int *x, int *y, int *sliceMaxValue, int *nbContourPoints );

	/**
	 * \fn		arma::fmat * contour(const arma::imat &slice, arma::fmat **orientation);
	 * \brief	Détection des contour
	 * \param	slice Coupe à traiter
	 * \param	orientation Matrice contenant le gradient de chaque point du contour
	 * \return	matrice de contour
	 */
	arma::fmat * contour( const arma::imat &slice, arma::fmat **orientation );

	/**
	 * \fn		arma::imat * convolution(const arma::imat &slice, arma::fcolvec verticalFilter, arma::frowvec horizontalFilter);
	 * \brief	Effectue le produit de convolution d'un filtre separable sur une matrice
	 * \param	slice Coupe à traiter
	 * \param	verticalFilter Vecteur colonne du filtre separable
	 * \param	horizontalFilter Vecteur ligne du filtre separable
	 * \return	matrice apres convolution
	 */
	arma::imat * convolution( const arma::imat &slice, arma::fcolvec verticalFilter, arma::frowvec horizontalFilter );

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
	int * drawLine( int xOrigine, int yOrigine, float orientation, int width, int height, int *length );

	/**
	 * \fn		static int floatCompare(const void *first, const void *second);
	 * \brief	Compare deux élement
	 * \param	first Premier élement à comparer
	 * \param	second Deuxième élement à comparer
	 * \return	0 si first == second; une valeur positive si first > second; negative si first < second
	 */
	static int floatCompare( const void *first, const void *second );

	/**
	 * \fn		void minSlice(const arma::imat &slice, int *minValue, int *maxValue, Coord *coordMax);
	 * \brief	Calcul les valeurs minimum et maximum d'une coupe ainsi que la liste des coordonnées ou apparrait la valeur maximum
	 */
	void minSlice( const arma::imat &slice, int *minValue, int *maxValue, iCoord2D *coordMax );

	/**
	 * \fn		void corrigeMoelle(QList<Coord> *moelle, float *listMax, float seuilHough);
	 * \brief	Corrige les valeur erronées de la moelle
	 */
	void correctMarrow( QList<iCoord2D> &marrow, float *listMax, float seuilHough );

private:

	int _falseCutPercent;			/*!< Pourcentage de coupes fausses */
	int _windowWidth;				/*!< Largeur de la fenêtre de voisinage */
	int _windowHeight;				/*!< Hauteur de la fenêtre de voisinage */
	float _binarizationThreshold;	/*!< Valeur du seuil de binarisation d'une image en niveau de gris */
	int _marrowLag;					/*!< Décalage maximal de la position de la moelle sur 2 coupes consécutives */
};



/**********************************
 * Définition des fonctions inline
 **********************************/
inline
int MarrowExtractor::falseCutPercent() {
	return _falseCutPercent;
}

inline
int MarrowExtractor::windowWidth() {
	return _windowWidth;
}

inline
int MarrowExtractor::windowHeight() {
	return _windowHeight;
}

inline
int MarrowExtractor::binarizationThreshold() {
	return _binarizationThreshold;
}

inline
int MarrowExtractor::marrowLag() {
	return _marrowLag;
}

inline
void MarrowExtractor::setFalseCutPercent( const int &percentage ) {
	_falseCutPercent = percentage;
}

inline
void MarrowExtractor::setWindowWidth( const int &width ) {
	_windowWidth = width;
}

inline
void MarrowExtractor::setWindowHeight( const int &height ) {
	_windowHeight = height;
}

inline
void MarrowExtractor::setBinarizationThreshold( const int &threshold ) {
	_binarizationThreshold = threshold;
}

inline void MarrowExtractor::setMarrowLag( const int &lag ) {
	_marrowLag = lag;
}


#endif // MARROWEXTRACTOR_H
