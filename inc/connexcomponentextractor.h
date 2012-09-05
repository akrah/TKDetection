#ifndef CONNEX_COMPONENT_EXTRACTOR_H
#define CONNEX_COMPONENT_EXTRACTOR_H

#include "billon_def.h"

#include <QMap>

namespace ConnexComponentExtractor {
	/**
	 * \fn		void extractConnexComponent( icube &cube, Image3D &image, int seuil = DEFAULT_MINIMUM_NUMBER_FOR_EXTRACTION )
	 * \brief	Extrait les composantes connexes et les ajoute à l'image
	 * \param	cube Matrice cubique contenant l'ensemble des points
	 * \param	image Image à laquelle il faut ajouter les composantes connexes du cube
	 */
	Billon * extractConnexComponents( Billon &billon, const int &minimumSize, const __billon_type__ &threshold );
	Billon * extractBiggestConnexComponent( Billon &billon, const __billon_type__ &threshold );

	Billon * extractConnexComponents( arma::Slice &currentSlice, const int &minimumSize, const __billon_type__ &threshold );
	Billon * extractBiggestConnexComponent( arma::Slice &currentSlice, const __billon_type__ &threshold );
}

#endif // CONNEX_COMPONENT_EXTRACTOR_H
