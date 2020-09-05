cmake_minimum_required(VERSION 3.5)

project(StylesheetParser LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

#==== Qt 5 core gui and widgets ==================================
include("CMakeLists.qt5.in")
#==== DOxygen ====================================================
#include("CMakeLists.doxy.in")
#====GoogleTest ==================================================
#include("CMakeLists.gtest.in")
#==== end of standard includes====================================

find_package(Qt5 COMPONENTS Core REQUIRED)

#add_library(parser SHARED)
#target_sources(parser
#   PUBLIC

#      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/StylesheetParser_global.h>
#      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/stylesheetparser.h>
#      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/parsererror.h>
#      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/node.h>

#      $<INSTALL_INTERFACE:include/stylesheetparser/StylesheetParser_global.h>
#      $<INSTALL_INTERFACE:include/stylesheetparser/stylesheetparser.h>
#      $<INSTALL_INTERFACE:include/stylesheetparser/parsererror.h>
#      $<INSTALL_INTERFACE:include/stylesheetparser/node.h>

#   PRIVATE
#      src/stylesheetparser.cpp
#      src/parsererror.cpp
#      src/node.cpp

#   )
#target_include_directories(parser
#    PUBLIC
#        $<INSTALL_INTERFACE:include>
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#    PRIVATE
#        ${CMAKE_CURRENT_SOURCE_DIR}/src
#)
#target_compile_definitions(stylesheetparser PRIVATE STYLESHEETPARSER_LIBRARY)

add_executable(tst_parser "")
target_sources(tst_parser
   PUBLIC

      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/StylesheetParser_global.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/parser.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/parserstate.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/node.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/stylesheetedit.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/stylesheethighlighter.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/mainwindow.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/datastore.h>

      $<INSTALL_INTERFACE:include/stylesheetparser/StylesheetParser_global.h>
      $<INSTALL_INTERFACE:include/stylesheetparser/parser.h>
      $<INSTALL_INTERFACE:include/stylesheetparser/parserstate.h>
      $<INSTALL_INTERFACE:include/stylesheetparser/node.h>
      $<INSTALL_INTERFACE:include/stylesheetparser/stylesheetedit.h>
      $<INSTALL_INTERFACE:include/stylesheetparser/stylesheethighlighter.h>
      $<INSTALL_INTERFACE:include/stylesheetparser/mainwindow.h>
      $<INSTALL_INTERFACE:include/stylesheetparser/datastore.h>

   PRIVATE
      src/main.cpp
      src/parser.cpp
      src/parserstate.cpp
      src/node.cpp
      src/stylesheetedit.cpp
      src/stylesheethighlighter.cpp
      src/mainwindow.cpp
      src/datastore.cpp

   )
target_include_directories(tst_parser
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(tst_parser PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)

target_compile_definitions(tst_parser PRIVATE STYLESHEETPARSER_LIBRARY)
