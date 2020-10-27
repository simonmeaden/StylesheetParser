cmake_minimum_required(VERSION 3.5)

project(StylesheetParser LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)
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
# TODO remove later when I combine this with sm_widgets

add_executable(tst_parser "")
target_sources(tst_parser
   PUBLIC

      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/StylesheetParser_global.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/stylesheetedit.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/stylesheeteditdialog.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/mainwindow.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/x11colors.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/abstractlabelledwidget.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/labelledlineedit.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetparser/labelledspinbox.h>

      $<INSTALL_INTERFACE:include/stylesheetparser/StylesheetParser_global.h>
      $<INSTALL_INTERFACE:include/stylesheetparser/stylesheetedit.h>
      $<INSTALL_INTERFACE:include/stylesheetparser/stylesheeteditdialog.h>
      $<INSTALL_INTERFACE:include/stylesheetparser/mainwindow.h>
      $<INSTALL_INTERFACE:include/stylesheetparser/x11colors.h>
      $<INSTALL_INTERFACE:include/stylesheetparser/abstractlabelledwidget.h>
      $<INSTALL_INTERFACE:include/stylesheetparser/labelledlineedit.h>
      $<INSTALL_INTERFACE:include/stylesheetparser/labelledspinbox.h>

   PRIVATE
      src/main.cpp
      src/parser.h
      src/parser.cpp
      src/parserstate.h
      src/parserstate.cpp
      src/node.cpp
      src/stylesheetedit.qrc
      src/stylesheetedit.cpp
      src/stylesheeteditdialog.cpp
      src/bookmarkarea.h
      src/bookmarkarea.cpp
      src/hoverwidget.h
      src/hoverwidget.cpp
      src/stylesheethighlighter.h
      src/stylesheethighlighter.cpp
      src/mainwindow.cpp
      src/datastore.h
      src/datastore.cpp
      src/common.h
      src/common.cpp
      src/x11colors.cpp
      src/abstractlabelledwidget.cpp
      src/labelledlineedit.cpp
      src/labelledspinbox.cpp
      src/fts_fuzzy_match.h

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
