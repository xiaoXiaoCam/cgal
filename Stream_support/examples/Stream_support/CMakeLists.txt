# Created by the script cgal_create_CMakeLists
# This is the CMake script for compiling a set of CGAL applications.

project( Stream_support_Examples)


cmake_minimum_required(VERSION 2.8.11)

# CGAL and its components
find_package( CGAL QUIET)
if ( CGAL_FOUND )
  include( ${CGAL_USE_FILE} )

  find_package( Boost QUIET )

  if ( NOT Boost_FOUND )

    message(STATUS "This project requires the Boost library, and will not be compiled.")

    return()  

  endif()

    create_single_source_cgal_program( "Point_WKT.cpp" )
    create_single_source_cgal_program( "Polygon_WKT.cpp" )
    create_single_source_cgal_program( "Linestring_WKT.cpp" )
    create_single_source_cgal_program( "read_WKT.cpp" )

    create_single_source_cgal_program( "read_xml.cpp" )
  
else ()
  message(STATUS "This project requires the CGAL library, and will not be compiled.")
  return()  

endif()

