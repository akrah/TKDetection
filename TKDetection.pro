#-------------------------------------------------------
#
# Fichier de configuration de compilation de TKDetection
#
# Project created by QtCreator 2011-12-08T16:51:12
#
#		Adrien Krähenbühl
#
#-------------------------------------------------------

# Version d'ITK installée
ITK_NUMBER =  4.13


#                                                                #
#                  Ne PAS MODIFIER EN DESSOUS                    #
##################################################################

# Configuration
#--------------#
TEMPLATE	=	app
QT			*=	widgets gui printsupport
CONFIG		*=	warn_on
CONFIG		*=	qwt qwtpolar $${ITK_NUMBER}

# Répertoires
#------------#
TARGET		= TKDetection
MOC_DIR		= moc
OBJECTS_DIR	= obj

# Traductions
#------------#
TRANSLATIONS = TKDetection_en.ts

# Fichiers
#---------#
SOURCES	=	src/main.cpp \
			src/billonalgorithms.cpp \
			src/connexcomponentextractor.cpp \
			src/dicomreader.cpp \
			src/ellipticalaccumulationhistogram.cpp \
			src/globalfunctions.cpp \
			src/knotellipseradiihistogram.cpp \
			src/knotpithprofile.cpp \
			src/lowess.cpp \
			src/mainwindow.cpp \
			src/ofsexport.cpp \
			src/pgm3dexport.cpp \
			src/piechart.cpp \
			src/piepart.cpp \
			src/pith.cpp \
			src/pithextractor.cpp \
			src/pithextractorboukadida.cpp \
			src/plotellipticalaccumulationhistogram.cpp \
			src/plotknotellipseradiihistogram.cpp \
			src/plotknotpithprofile.cpp \
			src/plotsectorhistogram.cpp \
			src/plotslicehistogram.cpp \
			src/pointpolarseriesdata.cpp \
			src/qxtspanslider.cpp \
			src/sectorhistogram.cpp \
			src/slicealgorithm.cpp \
			src/slicehistogram.cpp \
			src/sliceview.cpp \
			src/slicezoomer.cpp \
			src/tangentialtransform.cpp \
			tst/test_intervalshistogram.cpp \
			src/tiffreader.cpp \
			src/v3dexport.cpp \

HEADERS	=	inc/billon.h \
			inc/billonalgorithms.h \
			inc/connexcomponentextractor.h \
			inc/coordinate.h \
			inc/define.h \
			def/def_billon.h \
			def/def_coordinate.h \
			def/def_opticalflow.h \
			inc/dicomreader.h \
			inc/ellipticalaccumulationhistogram.h \
			inc/globalfunctions.h \
			inc/histogram.h \
			inc/interval.h \
			inc/knotellipseradiihistogram.h \
			inc/knotpithprofile.h \
			inc/lowess.h \
			inc/mainwindow.h \
			inc/ofsexport.h \
			inc/pgm3dexport.h \
			inc/piepart.h \
			inc/piechart.h \
			inc/pith.h \
			inc/pithextractor.h \
			inc/pithextractorboukadida.h \
			inc/plotellipticalaccumulationhistogram.h \
			inc/plotknotellipseradiihistogram.h \
			inc/plotknotpithprofile.h \
			inc/plotsectorhistogram.h \
			inc/plotslicehistogram.h \
			inc/pointpolarseriesdata.h \
			inc/qxtglobal.h \
			inc/qxtspanslider.h \
			inc/qxtspanslider_p.h \
			inc/sectorhistogram.h \
			inc/slicealgorithm.h \
			inc/slicehistogram.h \
			inc/sliceview.h \
			inc/slicezoomer.h \
			tst/test_intervalshistogram.h \
			inc/tangentialtransform.h \
			inc/tiffreader.h \
			inc/v3dexport.h \

FORMS =	ui/mainwindow.ui

# Directives compilateur
#-----------------------#
QMAKE_CXXFLAGS *= -std=c++14

# Librairies externes
#--------------------#
LIBS *= -larmadillo -lgsl -lgslcblas

# ITK
#----#
ITK_PATH	 =	/usr/local/include/ITK-$${ITK_NUMBER}/
INCLUDEPATH	*=	$${ITK_PATH}/

LIBS *= -lITKIOGDCM-$${ITK_NUMBER} \
		-litkgdcmDICT-$${ITK_NUMBER} \
		-litkgdcmMSFF-$${ITK_NUMBER} \
		-litkgdcmIOD-$${ITK_NUMBER} \
		-litkgdcmDSED-$${ITK_NUMBER} \
		-litkgdcmopenjp2-$${ITK_NUMBER} \
		-litkgdcmcharls-$${ITK_NUMBER} \
		-litkgdcmCommon-$${ITK_NUMBER} \
		-litkgdcmuuid-$${ITK_NUMBER} \
		-litkgdcmjpeg12-$${ITK_NUMBER} \
		-litkgdcmjpeg16-$${ITK_NUMBER} \
		-litkgdcmjpeg8-$${ITK_NUMBER} \
		-lITKIOBruker-$${ITK_NUMBER} \
		-lITKIOSiemens-$${ITK_NUMBER} \
		-lITKIOIPL-$${ITK_NUMBER} \
		-lITKIOJPEG-$${ITK_NUMBER} \
		-lITKIOBMP-$${ITK_NUMBER} \
		-lITKIOBioRad-$${ITK_NUMBER} \
		-lITKIOGE-$${ITK_NUMBER} \
		-lITKIOGIPL-$${ITK_NUMBER} \
		-lITKIOMINC-$${ITK_NUMBER} \
		-litkminc2-$${ITK_NUMBER} \
		-lITKIOHDF5-$${ITK_NUMBER} \
		-litkhdf5_cpp \
		-litkhdf5 \
		-lITKIOLSM-$${ITK_NUMBER} \
		-lITKIOMRC-$${ITK_NUMBER} \
		-lITKIONIFTI-$${ITK_NUMBER} \
		-lITKniftiio-$${ITK_NUMBER} \
		-lITKIONRRD-$${ITK_NUMBER} \
		-lITKNrrdIO-$${ITK_NUMBER} \
		-lITKIOPNG-$${ITK_NUMBER} \
		-litkpng-$${ITK_NUMBER} \
		-lITKIOStimulate-$${ITK_NUMBER} \
		-lITKIOTIFF-$${ITK_NUMBER} \
		-litktiff-$${ITK_NUMBER} \
		-litkjpeg-$${ITK_NUMBER} \
		-lITKIOVTK-$${ITK_NUMBER} \
		-lITKIOMeta-$${ITK_NUMBER} \
		-lITKMetaIO-$${ITK_NUMBER} \
		-lITKIOImageBase-$${ITK_NUMBER} \
		-lITKznz-$${ITK_NUMBER} \
		-litkzlib-$${ITK_NUMBER} \
		-lITKCommon-$${ITK_NUMBER} \
		-litksys-$${ITK_NUMBER} \
		-litkvnl_algo-$${ITK_NUMBER} \
		-litkv3p_netlib-$${ITK_NUMBER} \
		-litkvnl-$${ITK_NUMBER} \
		-litkvcl-$${ITK_NUMBER} \
		-lITKVNLInstantiation-$${ITK_NUMBER} \
		-lITKStatistics-$${ITK_NUMBER} \
		-litkdouble-conversion-$${ITK_NUMBER} \
		-lITKLabelMap-$${ITK_NUMBER} \
		-lITKEXPAT-$${ITK_NUMBER} \
		-lITKTransform-$${ITK_NUMBER} \
		-ldl \
		-lexpat
