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
ITK_NUMBER =  4.4


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
			src/concavitypointseriecurve.cpp \
			src/connexcomponentextractor.cpp \
			src/contour.cpp \
			src/contourbillon.cpp \
			src/contourdistanceshistogram.cpp \
			src/contourslice.cpp \
			src/curvaturehistogram.cpp \
			src/dicomreader.cpp \
			src/globalfunctions.cpp \
			src/intensitydistributionhistogram.cpp \
			src/nearestpointshistogram.cpp \
			src/mainwindow.cpp \
			src/ofsexport.cpp \
			src/opticalflow.cpp \
			src/pgm3dexport.cpp \
			src/piechart.cpp \
			src/piepart.cpp \
			src/pith.cpp \
			src/pithextractor.cpp \
			src/plotconcavitypointseriecurve.cpp \
			src/plotcontourdistanceshistogram.cpp \
			src/plotcurvaturehistogram.cpp \
			src/plotintensitydistributionhistogram.cpp \
			src/plotnearestpointshistogram.cpp \
			src/plotsectorhistogram.cpp \
			src/plotslicehistogram.cpp \
			src/pointpolarseriesdata.cpp \
			src/sectorhistogram.cpp \
			src/slicealgorithm.cpp \
			src/slicehistogram.cpp \
			src/sliceview.cpp \
			src/slicezoomer.cpp \
			tst/test_intervalshistogram.cpp \
			src/tiffreader.cpp \
			src/v3dexport.cpp \

HEADERS	=	inc/billon.h \
			inc/billonalgorithms.h \
			inc/concavitypointseriecurve.h \
			inc/connexcomponentextractor.h \
			inc/contour.h \
			inc/contourbillon.h \
			inc/contourdistanceshistogram.h \
			inc/contourslice.h \
			inc/coordinate.h \
			inc/curvaturehistogram.h \
			inc/define.h \
			def/def_billon.h \
			def/def_coordinate.h \
			def/def_opticalflow.h \
			inc/dicomreader.h \
			inc/globalfunctions.h \
			inc/histogram.h \
			inc/intensitydistributionhistogram.h \
			inc/interval.h \
			inc/nearestpointshistogram.h \
			inc/mainwindow.h \
			inc/ofsexport.h \
			inc/opticalflow.h \
			inc/pgm3dexport.h \
			inc/piepart.h \
			inc/piechart.h \
			inc/pith.h \
			inc/pithextractor.h \
			inc/plotconcavitypointseriecurve.h \
			inc/plotcontourdistanceshistogram.h \
			inc/plotcurvaturehistogram.h \
			inc/plotintensitydistributionhistogram.h \
			inc/plotnearestpointshistogram.h \
			inc/plotsectorhistogram.h \
			inc/plotslicehistogram.h \
			inc/pointpolarseriesdata.h \
			inc/sectorhistogram.h \
			inc/slicealgorithm.h \
			inc/slicehistogram.h \
			inc/sliceview.h \
			inc/slicezoomer.h \
			tst/test_intervalshistogram.h \
			inc/tiffreader.h \
			inc/v3dexport.h \

FORMS =	ui/mainwindow.ui

# Directives compilateur
#-----------------------#
CXXFLAGS *= -std=c++0x
macx:QMAKE_CC=/usr/bin/gcc
macx:QMAKE_CXX=/usr/bin/g++


# Librairies externes
#--------------------#
INCLUDEPATH *=	/usr/include/ /usr/local/include/

QMAKE_LIBDIR *=	/usr/local/lib/

LIBS *= -lblas -llapack -larmadillo   -lDGtal -lDGtalIO

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
