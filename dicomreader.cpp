#include "dicomreader.h"

#include "billon.h"

#include <QObject>
#include <algorithm>

#include <itkImageFileWriter.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>


/**
 * \file	dicomreader.cpp
 * \brief	Lecteur de fichiers DICOM
 * \author	Adrien KRAHENBUHL et Laurent HORY
 * \date	22 Juillet 2010
 */

namespace DicomReader {

	// Déclaration de fonctions privées
	namespace {
		Billon* makeBillonFromDicomWithITK( const QString &repository );
		QString getTag( const std::string &entryId, const itk::MetaDataDictionary & dictionary );
	}


	Billon* read( const QString &repository ) {

		Billon* cube = makeBillonFromDicomWithITK( repository );

		if ( cube == 0 ) {
			std::cout << QObject::tr("ERREUR lors de la lecture des fichiers.").toStdString() << std::endl;
			std::cout << QObject::tr("    ==> Abandon du chargement.").toStdString() << std::endl;
		}

		return cube;
	}

	// Implémentation des fonction privées
	namespace {
		Billon* makeBillonFromDicomWithITK( const QString &repository ) {

			// Définition des type de l'image
			typedef itk::Image<int,3> ImageType;
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
			std::vector<std::string> fileNames = nameGenerator->GetInputFileNames();

			reader->SetFileNames(fileNames);

			try {
				reader->Update();
			}
			catch(itk::ExceptionObject &ex) {
				std::cout << ex << std::endl;
				return 0;
			}

			// Dictionnaire
			const itk::MetaDataDictionary & dictionary = dicomIO->GetMetaDataDictionary();

			// Recherche de tag dans le fichier
			bool size_ok;

			const uint height = getTag("0028|0010",dictionary).toUInt(&size_ok);
			if ( !size_ok ) {
				std::cout << QObject::tr("ERREUR : Lecture des dimensions de l'image impossible.").toStdString() << std::endl;
				return 0;
			}
			const uint width = getTag("0028|0011",dictionary).toUInt(&size_ok);
			if ( !size_ok ) {
				std::cout << QObject::tr("ERREUR : Lecture des dimensions de l'image impossible.").toStdString() << std::endl;
				return 0;
			}
			const uint depth = fileNames.size();


			// Création d'une matrice aux dimensions de l'image
			Billon * const billon = new Billon( height, width, depth );

			const ImageType::Pointer image = reader->GetOutput();
			itk::ImageRegionConstIterator< ImageType > in( image,image->GetRequestedRegion() );
			int max, min;
			max = min = in.Get();

			for ( uint k=0; k<depth; k++ ) {
				imat &slice = billon->slice(k);
				for ( uint j=0; j<height; j++ ) {
					for ( uint i=0; i<width; i++ ) {
						const int current = in.Get();
						slice.at(j,i) = current;
						if ( current>max ) max = current;
						else if ( current<min ) min = current;
						++in;
					}
				}
			}

			billon->setMaxValue(max);
			billon->setMinValue(min);
			cout << "min = " << min << " et max = " << max << std::endl;

			return billon;
		}

		QString getTag( const std::string &entryId, const itk::MetaDataDictionary &dictionary ) {
			QString value = "undefined";

			itk::MetaDataObject<std::string>::ConstPointer entryValue = 0;
			itk::MetaDataDictionary::ConstIterator tagItr = dictionary.Find(entryId);

			if( tagItr != dictionary.End ()) {
				entryValue = dynamic_cast<itk::MetaDataObject<std::string> *> (tagItr->second.GetPointer());
				if ( entryValue ) {
					value = QString::fromStdString(entryValue->GetMetaDataObjectValue());
				}
			}
			return value;
		}
	}
}
