#-------------------------------------------------
#
# Project created by QtCreator 2011-12-08T16:51:12
#
#-------------------------------------------------

QT       *= core gui
CONFIG   *= qt thread

#CONFIG   *= qxt
#QXT      *= core gui

TARGET	  = Visu
TEMPLATE  = app


SOURCES  *= main.cpp \
			mainwindow.cpp \
			billon.cpp \
			dicomreader.cpp \
			slicehistogram.cpp \
			sliceview.cpp \
			#marrowextractor.cpp

HEADERS  *= mainwindow.h \
			billon.h \
			dicomreader.h \
			slicehistogram.h \
			sliceview.h \
			defines.h \
			#marrowextractor.h

FORMS    *= mainwindow.ui

ITK_PATH  = /usr/local/include/InsightToolkit/
QXT_PATH  = /usr/include/qxt
QWT_PATH  = /usr/include/qwt

INCLUDEPATH  *= \
				$${ITK_PATH}/IO/ \
				$${ITK_PATH}/Common/ \
				$${ITK_PATH}/ \
				$${ITK_PATH}/Utilities/vxl/vcl/ \
				$${ITK_PATH}/Utilities/vxl/core/ \
				$${ITK_PATH}/gdcm/src/ \
				$${ITK_PATH}/Utilities/ \
				$${QXT_PATH}/QxtGui/ \
				$${QXT_PATH}/QxtCore/ \
				$${QWT_PATH}

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
		-litkvnl \
		-lQxtGui \
		-lqwt
