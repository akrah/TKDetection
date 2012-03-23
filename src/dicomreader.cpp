#include "inc/dicomreader.h"

#include "inc/billon.h"

#include <QDebug>

#include <itkMetaDataDictionary.h>
#include <itkImage.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>

namespace DicomReader {

	// Déclaration de fonctions privées
	namespace {
		Billon* makeBillonFromDicomWithITK( const QString &repository );
		QString getTag( const QString &entryId, const itk::MetaDataDictionary &dictionary );
	}


	Billon* read( const QString &repository ) {

		Billon* cube = makeBillonFromDicomWithITK( repository );

		if ( cube == 0 ) {
			qWarning() << QObject::tr("ERREUR : Impossible de lire le contenu du répertoire.");
		}

		return cube;
	}

	// Implémentation des fonction privées
	namespace {
		inline
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
			reader->SetFileNames(nameGenerator->GetInputFileNames());

			try {
				reader->Update();
			}
			catch( itk::ExceptionObject &ex ) {
				qDebug() << ex.GetDescription();
				return 0;
			}

			// Dictionnaire
			const itk::MetaDataDictionary &dictionary = dicomIO->GetMetaDataDictionary();

			// Recherche de tag dans le fichier
			bool size_ok;

			const uint height = getTag("0028|0010",dictionary).toUInt(&size_ok);
			if ( !size_ok ) {
				qWarning() << QObject::tr("ERREUR : Lecture des dimensions de l'image impossible.");
				return 0;
			}
			const uint width = getTag("0028|0011",dictionary).toUInt(&size_ok);
			if ( !size_ok ) {
				qWarning() << QObject::tr("ERREUR : Lecture des dimensions de l'image impossible.");
				return 0;
			}
			const uint depth = reader->GetFileNames().size();

			// Création d'une matrice aux dimensions de l'image
			Billon * const billon = new Billon( height, width, depth );

			const ImageType::Pointer &image = reader->GetOutput();
			itk::ImageRegionConstIterator< ImageType > in( image,image->GetBufferedRegion() );
			int max, min;
			max = min = in.Value();

			for ( uint k=0; k<depth; k++ ) {
				arma::Mat<__billon_type__> &slice = billon->slice(k);
				for ( uint j=0; j<height; j++ ) {
					for ( uint i=0; i<width; i++ ) {
						const int &current = in.Value();
						slice.at(j,i) = current;
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

		inline
		QString getTag( const QString &entryId, const itk::MetaDataDictionary &dictionary ) {
			QString value = QObject::tr("indéfinie");

			itk::MetaDataObject<std::string>::ConstPointer entryValue = 0;
			const itk::MetaDataDictionary::ConstIterator tagItr = dictionary.Find(entryId.toStdString());

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
