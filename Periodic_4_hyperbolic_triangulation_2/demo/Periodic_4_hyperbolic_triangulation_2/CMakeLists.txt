cmake_minimum_required(VERSION 3.1...3.13)

project (Periodic_4_hyperbolic_triangulation_2_Demo)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories( ${CMAKE_BINARY_DIR} )

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON) 

find_package(CGAL QUIET COMPONENTS Core Qt5)
include(${CGAL_USE_FILE})

find_package(LEDA QUIET)

find_package(Qt5 QUIET COMPONENTS Widgets)
if(CGAL_FOUND AND (CGAL_Core_FOUND OR LEDA_FOUND) AND Qt5_FOUND AND CGAL_Qt5_FOUND)

  include_directories (BEFORE include )

  # ui files, created with Qt Designer
  qt5_wrap_ui( UIS P4HDT2.ui )

  # qrc files (resources files, that contain icons, at least)
  qt5_add_resources ( RESOURCE_FILES Main_resources.qrc )

  # cpp files
  add_executable ( P4HDT2
  P4HDT2.cpp ${RESOURCE_FILES} ${UIS})

  target_link_libraries( P4HDT2 PRIVATE
    CGAL::CGAL
    CGAL::CGAL_Qt5
    Qt5::Widgets )
  if(TARGET CGAL::CGAL_Core)
    target_link_libraries( P4HDT2 PRIVATE CGAL::CGAL_Core )
  endif()
  if(LEDA_FOUND)
    target_link_libraries( P4HDT2 PRIVATE ${LEDA_LIBRARIES} )
  endif()
else()
  message(STATUS "NOTICE: This demo requires Qt5 and will not be compiled.")
endif()
