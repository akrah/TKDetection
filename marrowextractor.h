#ifndef MARROWEXTRACTOR_H
#define MARROWEXTRACTOR_H

#include <armadillo>


/**
 * \file	marrowextractor.h
 * \brief	fichier d'entête de l'extraction de la moëlle
 * \author	Adrien KRAHENBUHL
 * \date	21 décembre 2011
 */

class MarrowExtractor {

/**
 * \class	MarrowExtractor
 * \brief	Classe regroupant les fonctions nécessaires à l'extraction de la moëlle
 */

public:
	MarrowExtractor();

	/**
	 * \fn		QList<Coord>* extractMoelle( icube *matrix, QList<int> *, int coupe_min, int coupe_max )
	 * \brief	Extrait la moelle d'une matrice et la donne sous frome d'une liste des coordonnées des voxels qui la compose
	 * \param	matrix Matrice contenant l'ensemble des points
	 * \param	coupe_min première coupe valide
	 * \param	coupe_max dernière coupe valide
	 * \return	la liste de coordonnées de la moelle de la matrice
	 */
	QList<Coord> * extractMoelle( icube *, int coupe_min, int coupe_max );

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
	void setFalseCutPercent( int );

	/**
	 * \fn		void setWindowWidth( int width )
	 * \param	width Largeur de la fenêtre du voisinage de la moelle
	 */
	void setWindowWidth( int );

	/**
	 * \fn		void setWindowHeight( int height )
	 * \param	height Hauteur de la fenêtre du voisinage de la moelle
	 */
	void setWindowHeight( int );

	/**
	 * \fn		void setBinarizationThreshold( int threshold )
	 * \param	threshold Valeur minimale en niveau de gris d'un voxel pour le considérer comme noir
	 *
	 * En dessous de ce seuil, ou égal à ce seuil, un voxel sera considéré comme blanc.
	 */
	void setBinarizationThreshold( int );

	/**
	 * \fn		void setMarrowLag( int lag )
	 * \param	lag Distance maximal entre deux voxel pour qu'ils soient acceptés comme appartenant à la moelle
	 */
	void setMarrowLag( int );

private:
	/**
	 * \fn		Coord transHough(imat *matrix, int width, int height, int *x, int *y, int* z, int *max, int* nbptcontour);
	 * \brief	Transformée de hough sur une coupe
	 * \param	matrix matrice contenant l'ensemble des points
	 * \param	width largeur de la fenetre
	 * \param	height hauteur de la fenetre
	 * \param	x coordonnee x de l'origine (coin superieur gauche)
	 * \param	y coordonnee y de l'origine (coin superieur gauche)
	 * \param	z coordonnee z de l'origine (coin superieur gauche)
	 * \param	max valeur maximale de la coupe
	 * \param	nbptcontour nombre de point de contour
	 * \return	coordonnée de la moelle pour la coupe donnée
	 */
	Coord transHough(imat *, int , int , int *, int *, int *, int *, int *);
	/**
	 * \fn		fmat * contour(imat *matrix, fmat **orientation);
	 * \brief	detection des contour
	 * \param	matrix matrice contenant l'ensemble des points
	 * \param	orientation matrice contenant le gradient de chaque point du contour
	 * \return	matrice de contour
	 */
	fmat * contour(imat *, fmat **);
	/**
	 * \fn		imat * convolution(imat *, fcolvec, frowvec);
	 * \brief	effectue le produit de convolution d'un filtre separable sur une matrice
	 * \param	matrix matrice sur laquelle il faut effectuer le produit de convolution
	 * \param	vecteur colonne du filtre separable
	 * \param	vecteur ligne du filtre separable
	 * \return	matrice apres convolution
	 */
	imat * convolution(imat *, fcolvec, frowvec);
	/**
	 * \fn		int * trace_droite(int x_origine, int y_origine, float orientation, int width, int height, int *longueur);
	 * \brief	trace une droite de coordonné x_origine et y_origine et d'angle orientation
	 * \param	x_origine coordonnée x de l'origine de la droite
	 * \param	y_origine coordonnée y de l'origine de la droite
	 * \param	orientation angle de la droite
	 * \param	largeur de l'image sur laquelle on veux tracer
	 * \param	hauteur de l'image sur laquelle on veux tracer
	 * \param	longueur de la droite
	 * \return	tableau de int contenant l'ensemble des points a tracer chaque valeur est un indice de la combinaison y*width+x
	 */
	int * trace_droite(int, int, float, int, int, int *);
	/**
	 * \fn		int arrondi(float value);
	 * \brief	permet d'arrondir une valeur float en la valeur int la plus proche
	 * \param	value valeur a convertir
	 * \return	valeur arrondie
	 */
	int arrondi(float);
	/**
	 * \fn		static int float_cmp(const void *a, const void *b);
	 * \brief	compare deux element
	 * \param	a premier element a copoarer
	 * \param	b deuxieme element a copoarer
	 * \return	0 si a = b; positive si a > b; negative si a < b
	 */
	static int float_cmp(const void *a, const void *b);
	/**
	 * \fn		void minMatrix(imat *matrix, int *min, int *mon, Coord *coordmax);
	 * \brief	retourn la valeur minimum ainsi que maximum d'un matrice et retourne les coordonnées de la valeur maximum
	 */
	void minMatrix(imat *, int *min, int *max, Coord *coordmax);
	/**
	 * \fn		void corrige_moelle(QList<Coord> *moelle, float *listmax, float seuilhough, Coord moyenne);
	 * \brief	corrige les valeur erronées de la moelle
	 * \param	moelle
	 * \param	listmax
	 * \param	seuilhough
	 * \param	moyenne
	 */
	void corrige_moelle(RegressiveElementLinearly *, float *, float);

private:

	int falseCutPercent;			/*!< Pourcentage de coupes fausses */
	int windowWidth;				/*!< Largeur de la fenêtre de voisinage */
	int windowHeight;				/*!< Hauteur de la fenêtre de voisinage */
	float binarizationThreshold;	/*!< Valeur du seuil de binarisation d'une image en niveau de gris */
	int marrowLag;					/*!< Décalage maximal de la position de la moelle sur 2 coupes consécutives */
};

#endif // MARROWEXTRACTOR_H
