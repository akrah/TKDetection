#ifndef CONNEX_COMPONENT_EXTRACTOR_H
#define CONNEX_COMPONENT_EXTRACTOR_H

#include "def/def_billon.h"

namespace ConnexComponentExtractor
{
	void extractConnexComponents( Billon &resultBillon, const Billon &billon, const int &minimumSize, const int &threshold );
	Billon * extractConnexComponents( const Billon &billon, const int &minimumSize, const int &threshold );
	void  extractConnexComponents( Slice &resulSlice, const Slice &slice, const int &minimumSize, const int &threshold );
	Slice *  extractConnexComponents( const Slice &slice, const int &minimumSize, const int &threshold );
}

#endif // CONNEX_COMPONENT_EXTRACTOR_H
