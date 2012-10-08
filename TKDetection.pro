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
ITK_NUMBER =  4.1

#                                                                #
#                  NE PAS MODIFIER EN DESSOUS                    #
##################################################################

# Configuration
#--------------#
TEMPLATE	=	app
QT			=	core gui xml
CONFIG		*=	copy_dir_files warn_on
CONFIG		*=	qwt qxt qwtpolar $${ITK_VERSION}
QXT			=	core gui

# Répertoires
#------------#
TARGET				= TKDetection
DEPENDPATH			= ./src/ ./inc/ ./ui/ ./tst/ ./def/
macx:MOC_DIR		= .moc
macx:OBJECTS_DIR	= .obj

# Fichiers
#---------#
SOURCES	=	main.cpp \
			connexcomponentextractor.cpp \
			contourcurve.cpp \
			datexport.cpp \
			dicomreader.cpp \
			mainwindow.cpp \
			ofsexport.cpp \
			opticalflow.cpp \
			pgm3dexport.cpp \
			piechart.cpp \
			piepart.cpp \
			pith.cpp \
			pithextractor.cpp \
			plotsectorhistogram.cpp \
			plotslicehistogram.cpp \
			pointpolarseriesdata.cpp \
			sectorhistogram.cpp \
			slicehistogram.cpp \
			sliceview.cpp \
			slicezoomer.cpp \
			test_intervalshistogram.cpp \
			v3dexport.cpp \
			v3dreader.cpp \
    src/knotareahistogram.cpp

HEADERS	=	billon.h \
			connexcomponentextractor.h \
			contourcurve.h \
			coordinate.h \
			datexport.h \
			define.h \
			def_billon.h \
			def_coordinate.h \
			def_opticalflow.h \
			dicomreader.h \
			histogram.h \
			interval.h \
			mainwindow.h \
			ofsexport.h \
			opticalflow.h \
			pgm3dexport.h \
			piepart.h \
			piechart.h \
			pith.h \
			pithextractor.h \
			plotsectorhistogram.h \
			plotslicehistogram.h \
			pointpolarseriesdata.h \
			sectorhistogram.h \
			slicehistogram.h \
			sliceview.h \
			slicezoomer.h \
			test_intervalshistogram.h \
			v3dexport.h \
			v3dreader.h \
    inc/knotareahistogram.h

FORMS =	mainwindow.ui

# Directives compilateur
#-----------------------#
CXXFLAGS += -std=c++0x
macx:QMAKE_CC=/usr/bin/clang
macx:QMAKE_CXX=/usr/bin/clang++


# Librairies externes
#--------------------#
INCLUDEPATH *=	/usr/include/ \
				/usr/local/include/

QMAKE_LIBDIR *=	/usr/local/lib/

LIBS *= -lblas -llapack -larmadillo

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

	LIBS *=	-lITKIOGDCM-$${ITK_NUMBER} \
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
						-litkvnl-$${ITK_NUMBER}

}
