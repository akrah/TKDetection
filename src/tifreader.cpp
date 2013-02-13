#include "inc/tifreader.h"

#include "inc/billon.h"

#include <QString>
#include <iostream>
#include <tiffio.h>

namespace TifReader
{
	Billon *read( const QString &filename )
	{
		TIFF* tif = TIFFOpen(filename.toStdString().c_str(), "r");
		if (!tif) return 0;

		uint32 depth, width, height;

		TIFFGetField(tif,TIFFTAG_IMAGEWIDTH,&width);
		TIFFGetField(tif,TIFFTAG_IMAGELENGTH,&height);
		depth = 0;
		do { depth++; } while (TIFFReadDirectory(tif));

		tif = TIFFOpen(filename.toStdString().c_str(), "r");
		if (!tif) return 0;

		Billon * const billon = new Billon( width, height, depth );

		tsize_t scanlineSize;
		tdata_t buf;
		uint i, j, k;
		for ( k=0 ; k<depth ; ++k )
		{
			TIFFSetDirectory(tif, k);
			scanlineSize = TIFFScanlineSize(tif);
			buf = _TIFFmalloc(scanlineSize);
			Slice &slice = billon->slice(k);
			for ( j=0 ; j<width ; ++j )
			{
				TIFFReadScanline( tif, buf, j );
				for ( i=0 ; i<scanlineSize ; ++i )
				{
					slice.at(j,i) = ((__billon_type__*)buf)[i];
				}
			}
			_TIFFfree(buf);
		}

		std::cout << "width / height / depth   : " << width << " / " << height << " / " << depth << std::endl;

		TIFFClose(tif);

		delete billon;

		return 0;
	}
}
