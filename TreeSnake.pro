#-------------------------------------------------
#
# Project created by QtCreator 2011-12-08T16:51:12
#
#-------------------------------------------------

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
				piecharthistograms.cpp \
				piepart.cpp \
				slicehistogram.cpp \
				sliceview.cpp \
    src/pointpolarseriesdata.cpp

HEADERS		=	billon.h \
				dicomreader.h \
				mainwindow.h \
				marrow.h \
				marrow_def.h \
				marrowextractor.h \
				marrowextractor_def.h \
				pie_def.h \
				piepart.h \
				piecharthistograms.h \
				piechart.h \
				slicehistogram.h \
				sliceview.h \
				sliceview_def.h \
    inc/pointpolarseriesdata.h

FORMS		=	mainwindow.ui

ITK_PATH	=	/usr/local/include/InsightToolkit

INCLUDEPATH	*=	$${ITK_PATH}/IO/ \
				$${ITK_PATH}/Common/ \
				$${ITK_PATH}/ \
				$${ITK_PATH}/Utilities/vxl/vcl/ \
				$${ITK_PATH}/Utilities/vxl/core/ \
				$${ITK_PATH}/gdcm/src/ \
				$${ITK_PATH}/Utilities/

QMAKE_LIBDIR *=	/usr/local/lib/InsightToolkit/

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
