cmake_minimum_required(VERSION 3.1.0)

# set path to additional CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

project(TKDetection)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ui/)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

include_directories(.)

find_package(Qt5 COMPONENTS Widgets Gui PrintSupport REQUIRED)
find_package(Qwt REQUIRED)
include_directories(${QWT_INCLUDE_DIRS})
find_package(QwtPolar REQUIRED)
include_directories(${QWTPOLAR_INCLUDE_DIR})
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
find_package(GSL REQUIRED)
include_directories(${GSL_INCLUDE_DIRS})

add_executable(TKDetection
	ui/mainwindow.ui
	src/billonalgorithms.cpp
	src/connexcomponentextractor.cpp
	src/dicomreader.cpp
	src/ellipticalaccumulationhistogram.cpp
	src/globalfunctions.cpp
	src/knotellipseradiihistogram.cpp
	src/knotpithprofile.cpp
	src/lowess.cpp
	src/ofsexport.cpp
	src/pgm3dexport.cpp
	src/piechart.cpp
	src/piepart.cpp
	src/pith.cpp
	src/pithextractor.cpp
	src/pithextractorboukadida.cpp
	src/plotellipticalaccumulationhistogram.cpp
	src/plotknotellipseradiihistogram.cpp
	src/plotknotpithprofile.cpp
	src/plotsectorhistogram.cpp
	src/plotslicehistogram.cpp
	src/pointpolarseriesdata.cpp
	src/qxtspanslider.cpp
	inc/qxtspanslider.h
	inc/qxtspanslider_p.h
	src/sectorhistogram.cpp
	src/slicealgorithm.cpp
	src/slicehistogram.cpp
	src/sliceview.cpp
	src/slicezoomer.cpp
	inc/slicezoomer.h
	src/tangentialtransform.cpp
	tst/test_intervalshistogram.cpp
	src/tiffreader.cpp
	src/v3dexport.cpp
	src/mainwindow.cpp
	inc/mainwindow.h
	src/main.cpp
)

target_link_libraries(TKDetection Qt5::Widgets Qt5::Gui Qt5::PrintSupport ${QWT_LIBRARIES} ${QWT_LIBRARIES} ${QWTPOLAR_LIBRARY} ${ITK_LIBRARIES} ${GSL_LIBRARIES})
