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

#------------- NE PAS MODIFIER EN DESSOUS -------------#

QT			*=	core gui xml
CONFIG		*=	qwt qxt qwtpolar
QXT			=	core gui

TEMPLATE	=	app
TARGET		=	TKDetection

DEPENDPATH	*=	./src/ ./inc/ ./ui/ ./tst/

SOURCES		=	main.cpp \
				connexcomponentextractor.cpp \
				datexport.cpp \
				dicomreader.cpp \
				histoexport.cpp \
				histogram.cpp \
				interval.cpp \
				intervalscomputer.cpp \
				mainwindow.cpp \
				marrow.cpp \
				marrowextractor.cpp \
				marrowextractor_def.cpp \
				ofsexport.cpp \
				opticalflow.cpp \
				pgm3dexport.cpp \
				piechart.cpp \
				piechartdiagrams.cpp \
				piepart.cpp \
				pointpolarseriesdata.cpp \
				slicehistogram.cpp \
				sliceview.cpp \
				slicezoomer.cpp \
				test_intervalshistogram.cpp \
				v3dexport.cpp \
				v3dreader.cpp \

HEADERS		=	billon.h \
				billon_def.h \
				connexcomponentextractor.h \
				datexport.h \
				dicomreader.h \
				global.h \
				histoexport.h \
				histogram.h \
				interval.h \
				intervalscomputer.h \
				intervalscomputerdefaultparameters.h \
				mainwindow.h \
				marrow.h \
				marrow_def.h \
				marrowextractor.h \
				marrowextractor_def.h \
				ofsexport.h \
				opticalflow.h \
				opticalflow_def.h \
				pgm3dexport.h \
				piepart.h \
				piechartdiagrams.h \
				piechart.h \
				pointpolarseriesdata.h \
				slicehistogram.h \
				sliceview.h \
				sliceview_def.h \
				slicezoomer.h \
				test_intervalshistogram.h \
				v3dexport.h \
				v3dreader.h \

FORMS		=	mainwindow.ui

INCLUDEPATH *=  /usr/include/ \
				/usr/local/include/ \
				/usr/include/qwt/

CXXFLAGS += -std=c++0x

LIBS *= -lblas -llapack -larmadillo

CONFIG += $${ITK_VERSION}
itk3 {
# SI ITK_VERSION = itk3
	ITK_PATH	 =	/usr/local/include/InsightToolkit/
	INCLUDEPATH	*=	$${ITK_PATH}/ \
					$${ITK_PATH}/IO/ \
					$${ITK_PATH}/Common/ \
					$${ITK_PATH}/gdcm/src/ \
					$${ITK_PATH}/Utilities/ \
					$${ITK_PATH}/Utilities/vxl/vcl/ \
					$${ITK_PATH}/Utilities/vxl/core/

	QMAKE_LIBDIR *=	/usr/local/lib/

	LIBS		*=	-lITKIO \
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
} else:itk4 {
# SI ITK_VERSION = itk4
	ITK_PATH	 =	/usr/local/include/ITK-$${ITK_NUMBER}/
	INCLUDEPATH	*=	$${ITK_PATH}/

	LIBS		*=	\
				-lITKIOGDCM-$${ITK_NUMBER} \
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
