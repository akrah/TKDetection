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
ITK_NUMBER =  4.5


#                                                                #
#                  Ne PAS MODIFIER EN DESSOUS                    #
##################################################################

# Configuration
#--------------#
TEMPLATE	=	app
QT			*=	core gui xml
CONFIG		*=	warn_on
CONFIG		*=	qwt qxt qwtpolar $${ITK_VERSION}
QXT			=	core gui widgets

# Répertoires
#------------#
TARGET				= TKDetection
macx:MOC_DIR		= .moc
macx:OBJECTS_DIR	= .obj
CONFIG  += qxt
QXT     += core gui

# Traductions
#------------#
TRANSLATIONS = TKDetection_en.ts

# Fichiers
#---------#
SOURCES	=	src/main.cpp \
			src/billonalgorithms.cpp \
			src/connexcomponentextractor.cpp \
			src/dicomreader.cpp \
			src/segmentation/ellipticalaccumulationhistogram.cpp \
			src/globalfunctions.cpp \
			src/knotareadetector.cpp \
			src/segmentation/knotellipseradiihistogram.cpp \
			src/segmentation/knotpithprofile.cpp \
			src/lowess.cpp \
			src/mainwindow.cpp \
			src/ofsexport.cpp \
			src/pgm3dexport.cpp \
			src/piechart.cpp \
			src/piepart.cpp \
			src/pith.cpp \
			src/pithextractorboukadida.cpp \
			src/segmentation/plotellipticalaccumulationhistogram.cpp \
			src/segmentation/plotknotellipseradiihistogram.cpp \
			src/segmentation/plotknotpithprofile.cpp \
			src/plotsectorhistogram.cpp \
			src/plotslicehistogram.cpp \
			src/pointpolarseriesdata.cpp \
			src/sectorhistogram.cpp \
			src/slicealgorithm.cpp \
			src/slicehistogram.cpp \
			src/slicepainter.cpp \
			src/sliceui.cpp \
			src/slicezoomer.cpp \
			src/segmentation/tangentialtransform.cpp \
			tst/test_intervalshistogram.cpp \
			src/tiffreader.cpp \
			src/v3dexport.cpp \
			src/zmotionaccumulator.cpp \

HEADERS	=	inc/billon.h \
			inc/billonalgorithms.h \
			inc/connexcomponentextractor.h \
			inc/coordinate.h \
			inc/define.h \
			def/def_billon.h \
			def/def_coordinate.h \
			def/def_opticalflow.h \
			inc/dicomreader.h \
			inc/segmentation/ellipticalaccumulationhistogram.h \
			inc/globalfunctions.h \
			inc/histogram.h \
			inc/interval.h \
			inc/knotareadetector.h \
			inc/segmentation/knotellipseradiihistogram.h \
			inc/segmentation/knotpithprofile.h \
			inc/lowess.h \
			inc/mainwindow.h \
			inc/ofsexport.h \
			inc/pgm3dexport.h \
			inc/piepart.h \
			inc/piechart.h \
			inc/pith.h \
			inc/pithextractorboukadida.h \
			inc/segmentation/plotellipticalaccumulationhistogram.h \
			inc/segmentation/plotknotellipseradiihistogram.h \
			inc/segmentation/plotknotpithprofile.h \
			inc/plotsectorhistogram.h \
			inc/plotslicehistogram.h \
			inc/pointpolarseriesdata.h \
			inc/sectorhistogram.h \
			inc/slicealgorithm.h \
			inc/slicehistogram.h \
			inc/slicepainter.h \
			inc/sliceui.h \
			inc/slicezoomer.h \
			tst/test_intervalshistogram.h \
			inc/segmentation/tangentialtransform.h \
			inc/tiffreader.h \
			inc/v3dexport.h \
			inc/zmotionaccumulator.h \

FORMS =	ui/mainwindow.ui

# Directives compilateur
#-----------------------#
QMAKE_CXXFLAGS *= -std=c++0x
macx:QMAKE_CC=/usr/bin/gcc
macx:QMAKE_CXX=/usr/bin/g++


# Librairies externes
#--------------------#
INCLUDEPATH *=	/usr/include/ /usr/local/include/

QMAKE_LIBDIR *=	/usr/local/lib/

LIBS *= -larmadillo -lgsl -lgslcblas

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
					-litkopenjpeg-$${ITK_NUMBER} \
					-litkgdcmjpeg12-$${ITK_NUMBER} \
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
				-ldl \
				-lexpat
}
