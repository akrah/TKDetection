#ifndef CONNEX_COMPONENT_EXTRACTOR_H
#define CONNEX_COMPONENT_EXTRACTOR_H

#include "def/def_billon.h"

namespace ConnexComponentExtractor {
	/**
	 * \fn		void extractConnexComponent( icube &cube, Image3D &image, int seuil = DEFAULT_MINIMUM_NUMBER_FOR_EXTRACTION )
	 * \brief	Extrait les composantes connexes et les ajoute à l'image
	 * \param	cube Matrice cubique contenant l'ensemble des points
	 * \param	image Image à laquelle il faut ajouter les composantes connexes du cube
	 */
	Billon * extractConnexComponents( Billon &billon, const int &minimumSize, const int &threshold );
	Billon * extractBiggestConnexComponent( Billon &billon, const int &threshold );

	Billon * extractConnexComponents( Slice &currentSlice, const int &minimumSize, const int &threshold );
	Billon * extractBiggestConnexComponent( Slice &currentSlice, const int &threshold );
}

#endif // CONNEX_COMPONENT_EXTRACTOR_H
