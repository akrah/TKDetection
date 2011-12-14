#include "dicomreader.h"

#include "billon.h"
#include <QObject>

#include <algorithm>

#include <itkImageFileWriter.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
//#include <itkObjectFactory.h>
//#include <BasicFilters/itkExtractImageFilter.h>
//#include <Numerics/Statistics/itkScalarImageToHistogramGenerator.h>


/**
 * \file	dicomreader.cpp
 * \brief	Lecteur de fichiers DICOM
 * \author	Adrien KRAHENBUHL et Laurent HORY
 * \date	22 Juillet 2010
 */

namespace DicomReader {

	// Déclaration de fonctions privées
	int getSliceNumber( std::string const & name );
	bool compareSliceNumbers( std::string const & name1, std::string const & name2 );
	Billon* makeCubeFromDicomWithITK( QString repository );
	QString getTag( std::string entryId, const itk::MetaDataDictionary & dictionary );


	Billon* read( QString repository ) {

		Billon* cube = makeCubeFromDicomWithITK( repository );

		if ( cube == 0 ) {
			std::cout << QObject::tr("ERREUR lors de la lecture des fichiers.").toStdString() << std::endl;
			std::cout << QObject::tr("    ==> Abandon du chargement.").toStdString() << std::endl;
		}

		return cube;
	}

	Billon* makeCubeFromDicomWithITK( QString repository ) {

		// Définition des type de l'image
		typedef itk::Image<unsigned int,3> ImageType;
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

		const int height = getTag("0028|0010",dictionary).toInt(&size_ok);
		if ( !size_ok ) {
			std::cout << QObject::tr("ERREUR : Lecture des dimensions de l'image impossible.").toStdString() << std::endl;
			return 0;
		}
		const int width = getTag("0028|0011",dictionary).toInt(&size_ok);
		if ( !size_ok ) {
			std::cout << QObject::tr("ERREUR : Lecture des dimensions de l'image impossible.").toStdString() << std::endl;
			return 0;
		}
		const int depth = fileNames.size();


		// Création d'une matrice aux dimensions de l'image
		Billon *cube = new Billon( height, width, depth );

		ImageType::Pointer image = reader->GetOutput();
		itk::ImageRegionConstIterator< ImageType > in( image,image->GetRequestedRegion() );
		int current, max, min;
		max = min = in.Get();
		for ( int k=0; k<depth; k++ ) {
			for ( int j=0; j<height; j++ ) {
				for ( int i=0; i<width; i++ ) {
					current = in.Get();
					cube->slice(k)(j,i) = current;
					if ( current>max ) {
						max = current;
					}
					else if ( current<min ) {
						min = current;
					}
					++in;
				}
			}
		}

		cube->setMaxValue(max);
		cube->setMinValue(min);

		return cube;
	}

	QString getTag( std::string entryId, const itk::MetaDataDictionary & dictionary ) {
		itk::MetaDataDictionary::ConstIterator tagItr;
		itk::MetaDataObject<std::string>::ConstPointer entryValue = 0;
		QString value;

		tagItr = dictionary.Find(entryId);
		if( tagItr != dictionary.End ()) {
			entryValue = dynamic_cast<itk::MetaDataObject<std::string> *> (tagItr->second.GetPointer());
		}
		if ( entryValue ) {
			value = QString::fromStdString(entryValue->GetMetaDataObjectValue());
		}
		else {
			value = "undefined";
		}
		return value;
	}
}
