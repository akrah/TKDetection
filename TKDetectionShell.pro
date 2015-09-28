#-------------------------------------------------------
#
# Fichier de configuration de compilation de TKDetection
#
# Project created by QtCreator 2011-12-08T16:51:12
#
#		Adrien Krähenbühl
#
#-------------------------------------------------------

# Version d'ITK installée : itk3 | itk4
ITK_VERSION = itk4
ITK_NUMBER = 4.8


#                                                                #
#                  Ne PAS MODIFIER EN DESSOUS                    #
##################################################################

# Configuration
#--------------#
TEMPLATE	=  app
QT			*= core
CONFIG		*= warn_on

CONFIG		*= qxt $${ITK_VERSION} $${ITK_NUMBER}
QXT			*= core

# Répertoires
#------------#
TARGET		= TKDetectionShell
MOC_DIR		= moc
OBJECTS_DIR	= obj

# Traductions
#------------#
TRANSLATIONS	= TKDetection_en.ts

# Fichiers
#---------#
SOURCES	=	src/mainshell.cpp \
			src/billonalgorithms.cpp \
			src/detection/knotareadetector.cpp \
			src/detection/knotbywhorldetector.cpp \
			src/detection/knotbyzmotionmapdetector.cpp \
			src/detection/sectorhistogram.cpp \
			src/detection/slicehistogram.cpp \
			src/detection/zmotionaccumulator.cpp \
			src/dicomreader.cpp \
			src/globalfunctions.cpp \
			src/knotarea.cpp \
			src/lowess.cpp \
			src/piechart.cpp \
			src/piepart.cpp \
			src/pith.cpp \
			src/pithextractorboukadida.cpp \
			src/slicealgorithm.cpp \
			src/segmentation/ellipseradiihistogram.cpp \
			src/segmentation/ellipticalaccumulationhistogram.cpp \
			src/segmentation/pithprofile.cpp \
			src/segmentation/tangentialgenerator.cpp \
			src/v3dexport.cpp \

HEADERS	=	inc/billon.h \
			inc/billonalgorithms.h \
			inc/coordinate.h \
			inc/define.h \
			def/def_billon.h \
			def/def_coordinate.h \
			inc/detection/knotareadetector.h \
			inc/detection/knotbywhorldetector.h \
			inc/detection/knotbyzmotionmapdetector.h \
			inc/detection/sectorhistogram.h \
			inc/detection/slicehistogram.h \
			inc/detection/zmotionaccumulator.h \
			inc/dicomreader.h \
			inc/globalfunctions.h \
			inc/histogram.h \
			inc/interval.h \
			inc/knotarea.h \
			inc/lowess.h \
			inc/piepart.h \
			inc/piechart.h \
			inc/pith.h \
			inc/pithextractorboukadida.h \
			inc/segmentation/ellipticalaccumulationhistogram.h \
			inc/segmentation/ellipseradiihistogram.h \
			inc/segmentation/pithprofile.h \
			inc/segmentation/tangentialgenerator.h \
			inc/slicealgorithm.h \
			inc/v3dexport.h \

# Directives compilateur
#-----------------------#
QMAKE_CXXFLAGS	*= -std=c++11


# Librairies externes
#--------------------#
INCLUDEPATH		*=	/usr/include/ \
					/usr/local/include/ \
					/usr/local/Qxt/include/QxtCore/
QMAKE_LIBDIR	*=	/usr/local/lib/


# Armadillo
#---------#
DEFINES	+=	ARMA_DONT_USE_WRAPPER
LIBS	*=	-larmadillo \
			-lopenblas \
			-llapack

# GSL
#---#
LIBS *=	-lgsl \
		-lgslcblas


# ITK
#----#
itk3 {
	# SI ITK_VERSION = itk3
	#----------------------#
	ITK_PATH	 =	/usr/local/include/InsightToolkit/
	INCLUDEPATH	*=	$${ITK_PATH}/ \
					$${ITK_PATH}/IO/ \
					$${ITK_PATH}/Common/ \
					$${ITK_PATH}/gdcm/src/ \
					$${ITK_PATH}/Utilities/ \
					$${ITK_PATH}/Utilities/vxl/vcl/ \
					$${ITK_PATH}/Utilities/vxl/core/

	LIBS *=	-lITKIO \
			-litkgdcm \
			-litkjpeg8 \
			-litktiff \
			-lITKMetaIO \
			-lITKNrrdIO \
			-litkpng \
			-litkzlib \
			-lITKDICOMParser \
			-litkjpeg12 \
			-litkjpeg16 \
			-litkopenjpeg \
			-lITKniftiio \
			-lITKznz \
			-lITKCommon \
			-litksys \
			-litkvnl_algo \
			-litkv3p_netlib \
			-litkvnl
}
else:itk4 {
	# SI ITK_VERSION = itk4
	#----------------------#
	ITK_PATH	 =	/usr/local/include/ITK-$${ITK_NUMBER}/
	INCLUDEPATH	*=	$${ITK_PATH}/

	LIBS *= -lITKIOGDCM-$${ITK_NUMBER} \
			-litkgdcmDICT-$${ITK_NUMBER} \
			-litkgdcmMSFF-$${ITK_NUMBER} \
			-litkgdcmIOD-$${ITK_NUMBER} \
			-litkgdcmDSED-$${ITK_NUMBER} \
			-litkzlib-$${ITK_NUMBER} \
			-litkgdcmCommon-$${ITK_NUMBER} \
			-litkgdcmuuid-$${ITK_NUMBER} \

	4.8  {	LIBS *=	-litkgdcmcharls-$${ITK_NUMBER} -litkgdcmopenjpeg-$${ITK_NUMBER} }
	else {	LIBS *=	-litkopenjpeg-$${ITK_NUMBER} }

	LIBS *=	-litkgdcmjpeg12-$${ITK_NUMBER} \
			-litkgdcmjpeg16-$${ITK_NUMBER} \
			-litkgdcmjpeg8-$${ITK_NUMBER} \
			-lITKIOImageBase-$${ITK_NUMBER} \
			-lITKCommon-$${ITK_NUMBER} \
			-litksys-$${ITK_NUMBER} \
			-litkvnl_algo-$${ITK_NUMBER} \
			-litkv3p_netlib-$${ITK_NUMBER} \
			-litkvnl-$${ITK_NUMBER} \
			-lITKIOTIFF-$${ITK_NUMBER} \
			-litktiff-$${ITK_NUMBER} \
			-litkjpeg-$${ITK_NUMBER} \
			-lITKVNLInstantiation-$${ITK_NUMBER} \
			-ldl \
			-lexpat
}
