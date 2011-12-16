#ifndef DICOMREADER_H
#define DICOMREADER_H

#include <QString>
#include <itkMetaDataDictionary.h>

class Billon;

/**
 * \file	iodicom.h
 * \brief	Fichier pour l'importation et l'exportationde fichiers DICOM
 * \author	Adrien KRAHENBUHL et Laurent HORY
 * \date	22 Juillet 2010
 */

namespace DicomReader {

	/**
	 * \fn		Image3D* read( QString repository )
	 * \brief	Construit une image à partir d'un fichier DICOM
	 * \param	repository Chemin vers le fichier source
	 * \return	l'image 3D correspondant au fichier lu
	 */
	Billon* read( const QString & );
}

#endif // DICOMREADER_H
