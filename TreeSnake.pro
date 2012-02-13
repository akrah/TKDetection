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
				billon.cpp \
				dicomreader.cpp \
				mainwindow.cpp \
				marrow.cpp \
				marrow_def.cpp \
				marrowextractor.cpp \
				marrowextractor_def.cpp \
				piechart.cpp \
				piechartdiagrams.cpp \
				piepart.cpp \
				pointpolarseriesdata.cpp \
				slicehistogram.cpp \
				sliceview.cpp \
    src/slicezoomer.cpp

HEADERS		=	billon.h \
				dicomreader.h \
				global.h \
				mainwindow.h \
				marrow.h \
				marrow_def.h \
				marrowextractor.h \
				marrowextractor_def.h \
				piepart.h \
				piechartdiagrams.h \
				piechart.h \
				pointpolarseriesdata.h \
				slicehistogram.h \
				sliceview.h \
				sliceview_def.h \
    inc/slicezoomer.h

FORMS		=	mainwindow.ui

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
	ITK_PATH	 =	/usr/local/include/ITK-4.0/
	INCLUDEPATH	*=	$${ITK_PATH}/

	LIBS		*=	\
				-lITKIOGDCM-4.0 \
					-litkgdcmDICT-4.0 \
					-litkgdcmMSFF-4.0 \
						-litkgdcmIOD-4.0 \
						-litkgdcmDSED-4.0 \
							-litkzlib-4.0 \
						-litkgdcmCommon-4.0 \
						-litkgdcmuuid-4.0 \
						-litkopenjpeg-4.0 \
						-litkgdcmjpeg12-4.0 \
						-litkgdcmjpeg16-4.0 \
						-litkgdcmjpeg8-4.0 \
					-lITKIOImageBase-4.0 \
						-lITKCommon-4.0 \
							-litksys-4.0 \
							-litkvnl_algo-4.0 \
							-litkv3p_netlib-4.0 \
							-litkvnl-4.0

}
