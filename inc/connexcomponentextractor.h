#ifndef CONNEX_COMPONENT_EXTRACTOR_H
#define CONNEX_COMPONENT_EXTRACTOR_H

#include "def/def_billon.h"

namespace ConnexComponentExtractor
{
	Billon * extractConnexComponents( const Billon &billon, const int &minimumSize, const int &threshold );
	Slice *  extractConnexComponents( const Slice &slice, const int &minimumSize, const int &threshold );
}

#endif // CONNEX_COMPONENT_EXTRACTOR_H
