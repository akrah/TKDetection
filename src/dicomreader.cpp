#include "inc/dicomreader.h"

#include "inc/billon.h"

#include <QDebug>

#include <itkMetaDataDictionary.h>
#include <itkImage.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>

namespace DicomReader
{
	// Déclaration de fonctions privées
	namespace
	{
		Billon* makeBillonFromDicomWithITK(const QString &repository , const bool &sliceOrderInversed);
	}

	Billon* read( const QString &repository, const bool &sliceOrderInversed )
	{
		Billon* cube = makeBillonFromDicomWithITK( repository, sliceOrderInversed );
		if ( !cube ) qWarning() << QObject::tr("ERREUR : Impossible de lire le contenu du répertoire.");
		return cube;
	}

	// Implémentation des fonction privées
	namespace
	{
		Billon* makeBillonFromDicomWithITK( const QString &repository, const bool &sliceOrderInversed )
		{
			// Définition des type de l'image
			const unsigned int InputDimension = 3;
			typedef int PixelType;
			typedef itk::Image<PixelType,InputDimension> ImageType;
			typedef itk::ImageSeriesReader<ImageType> ReaderType;
			ReaderType::Pointer reader = ReaderType::New();

			// Définition du dicom
			typedef itk::GDCMImageIO ImageIOType;
			ImageIOType::Pointer dicomIO = ImageIOType::New();
			reader->SetImageIO(dicomIO);

			// Mise en place de la lecture en serie
			typedef itk::GDCMSeriesFileNames NamesGeneratorType;
			NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
			nameGenerator->SetDirectory(repository.toStdString());
			reader->SetFileNames(nameGenerator->GetInputFileNames());

			try
			{
				reader->Update();
			}
			catch( itk::ExceptionObject &ex )
			{
				qDebug() << ex.GetDescription();
				return 0;
			}

			// Pointeur sur l'image obtenu par ITK
			const ImageType::Pointer &image = reader->GetOutput();

			// Recherche de tag dans le fichier
			const ImageType::SizeType& inputSize = image->GetLargestPossibleRegion().GetSize();
			const uint &width = inputSize[0];
			const uint &height = inputSize[1];
			const uint &depth = inputSize[2];

			// Création d'un billon aux dimensions de l'image
			Billon * const billon = new Billon( height, width, depth );

			// Copie de l'image ITK dans le billon
			itk::ImageRegionConstIterator< ImageType > in( image,image->GetBufferedRegion() );
			int max, min;
			max = min = in.Value();
			for ( uint k=0; k<depth; k++ )
			{
				Slice &slice = billon->slice(sliceOrderInversed?depth-1-k:k);
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

			// Espacement des coupes et tailles des pixels en mm.
			const ImageType::SpacingType &spacing = image->GetSpacing();
			billon->setVoxelSize(spacing[0],spacing[1],spacing[2]);

			return billon;
		}
	}
}

