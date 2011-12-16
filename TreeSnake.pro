#-------------------------------------------------
#
# Project created by QtCreator 2011-12-08T16:51:12
#
#-------------------------------------------------

QT       *= core gui
CONFIG   *= qt thread

TARGET	  = Visu
TEMPLATE  = app


SOURCES  *= main.cpp \
			mainwindow.cpp \
			billon.cpp \
			dicomreader.cpp \
			slicehistogram.cpp \
	sliceview.cpp

HEADERS  *= mainwindow.h \
			billon.h \
			dicomreader.h \
			slicehistogram.h \
	sliceview.h

FORMS    *= mainwindow.ui

ITK_PATH  = /usr/local/include/InsightToolkit/

INCLUDEPATH  += \
				/usr/include/ \
				/usr/local/include \
				$${ITK_PATH}/IO/ \
				$${ITK_PATH}/Common/ \
				$${ITK_PATH}/ \
				$${ITK_PATH}/Utilities/vxl/vcl/ \
				$${ITK_PATH}/Utilities/vxl/core/ \
				$${ITK_PATH}/gdcm/src/ \
				$${ITK_PATH}/Utilities/

QMAKE_LIBDIR *= /usr/local/lib/InsightToolkit/
LIBS  *= \
		-lITKIO \
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
