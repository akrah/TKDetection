#-----------------------------------------------------
#
# Fichier de configuration de compilation de TreeSnake
#
# Project created by QtCreator 2011-12-08T16:51:12
#
#                Adrien Krähenbühl
#
#-----------------------------------------------------

# Version d'ITK installée : itk3 | itk4
ITK_VERSION =   itk4

#------------ NE PAS MODIFIER EN DESSOUS ------------#

QT			*=	core gui
CONFIG		*=	qwt qxt qwtpolar
QXT			=	core gui

TEMPLATE	=	app
TARGET		=	TreeSnake

DEPENDPATH	*=	./src/ ./inc/ ./ui/

SOURCES		=	main.cpp \
				datexport.cpp \
				dicomreader.cpp \
				intensityinterval.cpp \
				interval.cpp \
				mainwindow.cpp \
				marrow.cpp \
				marrowextractor.cpp \
				marrowextractor_def.cpp \
				ofsexport.cpp \
				opticalflow.cpp \
				piechart.cpp \
				piechartdiagrams.cpp \
				piepart.cpp \
				pointpolarseriesdata.cpp \
				slicehistogram.cpp \
				sliceview.cpp \
				slicezoomer.cpp \
				slicesinterval.cpp \

HEADERS		=	billon.h \
				billon_def.h \
				datexport.h \
				dicomreader.h \
				global.h \
				intensityinterval.h \
				interval.h \
				mainwindow.h \
				marrow.h \
				marrow_def.h \
				marrowextractor.h \
				marrowextractor_def.h \
				ofsexport.h \
				opticalflow.h \
				opticalflow_def.h \
				piepart.h \
				piechartdiagrams.h \
				piechart.h \
				pointpolarseriesdata.h \
				slicehistogram.h \
				slicesinterval.h \
				sliceview.h \
				sliceview_def.h \
				slicezoomer.h \

FORMS		=	mainwindow.ui

INCLUDEPATH *=  /usr/include/ \
				/usr/local/include/

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
					$${ITK_PATH}/Utilities/vxl/core/ \

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
	ITK_PATH	 =	/usr/local/include/ITK-4.1/
	INCLUDEPATH	*=	$${ITK_PATH}/
	QMAKE_LIBDIR *=	/usr/local/lib/
	LIBS		*=	\
				-lITKIOGDCM-4.1 \
					-litkgdcmDICT-4.1 \
					-litkgdcmMSFF-4.1 \
						-litkgdcmIOD-4.1 \
						-litkgdcmDSED-4.1 \
							-litkzlib-4.1 \
						-litkgdcmCommon-4.1 \
						-litkgdcmuuid-4.1 \
						-litkopenjpeg-4.1 \
						-litkgdcmjpeg12-4.1 \
						-litkgdcmjpeg16-4.1 \
						-litkgdcmjpeg8-4.1 \
					-lITKIOImageBase-4.1 \
						-lITKCommon-4.1 \
							-litksys-4.1 \
							-litkvnl_algo-4.1 \
							-litkv3p_netlib-4.1 \
							-litkvnl-4.1

}
