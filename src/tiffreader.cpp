#include "inc/tiffreader.h"

#include "inc/billon.h"

#include <QString>
#include <iostream>

#include <itkImage.h>
#include <itkImageSeriesReader.h>
#include <itkTIFFImageIO.h>
#include <itkNumericSeriesFileNames.h>

namespace TiffReader
{
	// Déclaration de fonctions privées
	namespace
	{
		Billon* makeBillonFromTiffWithITK( const QString &filename );
	}

	Billon *read( const QString &filename )
	{
		Billon* cube = makeBillonFromTiffWithITK( filename );
		if ( !cube ) qWarning() << QObject::tr("ERREUR : Impossible de lire le contenu du fichier.");
		return cube;
	}

	// Implémentation des fonction privées
	namespace
	{
		Billon* makeBillonFromTiffWithITK( const QString &filename )
		{
			const unsigned int InputDimension = 3;
			typedef unsigned char PixelType;
			typedef itk::Image< PixelType, InputDimension > ImageType;
			typedef itk::ImageFileReader< ImageType > ReaderType;
			ReaderType::Pointer reader = ReaderType::New();

			// Définition du tiff
			typedef itk::TIFFImageIO ImageIOType;
			ImageIOType::Pointer tiffIO = ImageIOType::New();
			reader->SetImageIO(tiffIO);;
			reader->SetFileName(filename.toStdString().c_str());

			try
			{
				reader->Update();
			}
			catch( itk::ExceptionObject &e )
			{
				qDebug() << e.GetDescription();
				return 0;
			}

			const ImageType::Pointer &image = reader->GetOutput();
			itk::Size<InputDimension> imageSize = image->GetLargestPossibleRegion().GetSize();
			const uint width = imageSize[0];
			const uint height = imageSize[1];
			const uint depth = imageSize[2];

			// Création d'une matrice aux dimensions de l'image
			Billon * const billon = new Billon( height, width, depth );

			itk::ImageRegionConstIterator< ImageType > in( image,image->GetBufferedRegion() );
			int max, min;
			max = min = in.Value();

			for ( uint k=0; k<depth; k++ )
			{
				Slice &slice = billon->slice(k);
				for ( uint j=0; j<height; j++ )
				{
					for ( uint i=0; i<width; i++ )
					{
						const int &current = in.Value();
						slice(j,i) = current;
						max = qMax(max,current);
						min = qMin(min,current);
						++in;
					}
				}
			}

			billon->setMaxValue(max);
			billon->setMinValue(min);

			const ImageType::SpacingType &spacing = image->GetSpacing();
			billon->setVoxelSize(spacing[0],spacing[1],spacing[2]);

			return billon;
		}
	}
}
