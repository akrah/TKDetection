PROJECT(KnotVisu)

#Required in DGtal
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)


SET(Boost_USE_STATIC_LIBS   ON)
SET(Boost_USE_MULTITHREADED ON)
SET(Boost_USE_STATIC_RUNTIME OFF)
SET(Boost_FOUND FALSE)
FIND_PACKAGE(Boost 1.46.0 REQUIRED COMPONENTS program_options)
if ( Boost_FOUND )
  message(STATUS "Boost and boost_program_options found.")
  include_directories( ${Boost_INCLUDE_DIRS} )
  SET(DGtalToolsLibDependencies ${DGtalToolsLibDependencies} 
     ${Boost_LIBRAIRIES}  
     ${Boost_PROGRAM_OPTIONS_LIBRARY})
   SET(DGtalLibInc ${Boost_INCLUDE_DIRS})
endif( Boost_FOUND) 

FIND_PACKAGE(DGtal REQUIRED)
INCLUDE_DIRECTORIES(${DGTAL_INCLUDE_DIRS})
LINK_DIRECTORIES(${DGTAL_LIBRARY_DIRS})

ADD_EXECUTABLE(knotVisu knotVisu)
TARGET_LINK_LIBRARIES(knotVisu ${DGTAL_LIBRARIES} ${DGtalToolsLibDependencies})
INSTALL(TARGETS knotVisu  RUNTIME DESTINATION bin)

ADD_EXECUTABLE(trunkVisu trunkVisu)
TARGET_LINK_LIBRARIES(trunkVisu ${DGTAL_LIBRARIES} ${DGtalToolsLibDependencies})
INSTALL(TARGETS trunkVisu  RUNTIME DESTINATION bin)

ADD_EXECUTABLE(trunkVisuFig1_Step2_3 trunkVisuFig1_Step2_3)
TARGET_LINK_LIBRARIES(trunkVisuFig1_Step2_3 ${DGTAL_LIBRARIES} ${DGtalToolsLibDependencies})
INSTALL(TARGETS trunkVisuFig1_Step2_3  RUNTIME DESTINATION bin)

ADD_EXECUTABLE(trunkVisuFig1_Step2_3colors trunkVisuFig1_Step2_3colors)
TARGET_LINK_LIBRARIES(trunkVisuFig1_Step2_3colors ${DGTAL_LIBRARIES} ${DGtalToolsLibDependencies})
INSTALL(TARGETS trunkVisuFig1_Step2_3colors  RUNTIME DESTINATION bin)

ADD_EXECUTABLE(trunkVisuFig1_Step1 trunkVisuFig1_Step1)
TARGET_LINK_LIBRARIES(trunkVisuFig1_Step1 ${DGTAL_LIBRARIES} ${DGtalToolsLibDependencies})
INSTALL(TARGETS trunkVisuFig1_Step1  RUNTIME DESTINATION bin)

ADD_EXECUTABLE(trunkVisuFig0 trunkVisuFig0)
TARGET_LINK_LIBRARIES(trunkVisuFig0 ${DGTAL_LIBRARIES} ${DGtalToolsLibDependencies})
INSTALL(TARGETS trunkVisuFig0  RUNTIME DESTINATION bin)

ADD_EXECUTABLE(tangentialImageExtraction tangentialImageExtraction)
TARGET_LINK_LIBRARIES(tangentialImageExtraction ${DGTAL_LIBRARIES} ${DGtalToolsLibDependencies})
INSTALL(TARGETS tangentialImageExtraction  RUNTIME DESTINATION bin)







