cmake_minimum_required(VERSION 3.5)

project(StylesheetEditor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(Q_COMPILER_INITIALIZER_LISTS)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(FetchContent)

#==== Qt 5 core gui and widgets ==================================
include("CMakeLists.qt5.in")
#==== DOxygen ====================================================
#include("CMakeLists.doxy.in")
#====GoogleTest ==================================================
#include("CMakeLists.gtest.in")
#==== end of standard includes====================================

find_package(Qt5 COMPONENTS Core REQUIRED)
# TODO remove later when I combine this with sm_widgets

find_package(yaml-cpp REQUIRED)
#find_package(qyamlcpp REQUIRED)
FetchContent_Declare(
   rapidfuzz
   SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/RapidFuzz
   PREFIX ${CMAKE_CURRENT_BINARY_DIR}/rapidfuzz
   CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> "${CMAKE_OPT_ARGS}"
 )
 FetchContent_MakeAvailable(rapidfuzz)

#add_library(stylesheet_edit "")
#target_sources(stylesheet_edit
add_executable(tst_parser "")
target_sources(tst_parser
   PUBLIC

      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetedit/StylesheetParser_global.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetedit/stylesheetedit.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetedit/stylesheeteditdialog.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetedit/mainwindow.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetedit/x11colors.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetedit/abstractlabelledwidget.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetedit/abstractlabelledspinbox.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetedit/labelledlineedit.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetedit/labelledtextfield.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetedit/labelledspinbox.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetedit/labelledcombobox.h>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/stylesheetedit/extendedcolordialog.h>

      $<INSTALL_INTERFACE:include/stylesheetedit/StylesheetParser_global.h>
      $<INSTALL_INTERFACE:include/stylesheetedit/stylesheetedit.h>
      $<INSTALL_INTERFACE:include/stylesheetedit/stylesheeteditdialog.h>
      $<INSTALL_INTERFACE:include/stylesheetedit/mainwindow.h>
      $<INSTALL_INTERFACE:include/stylesheetedit/x11colors.h>
      $<INSTALL_INTERFACE:include/stylesheetedit/abstractlabelledwidget.h>
      $<INSTALL_INTERFACE:include/stylesheetedit/abstractlabelledspinbox.h>
      $<INSTALL_INTERFACE:include/stylesheetedit/labelledlineedit.h>
      $<INSTALL_INTERFACE:include/stylesheetedit/labelledtextfield.h>
      $<INSTALL_INTERFACE:include/stylesheetedit/labelledspinbox.h>
      $<INSTALL_INTERFACE:include/stylesheetedit/labelledcombobox.h>
      $<INSTALL_INTERFACE:include/stylesheetedit/extendedcolordialog.h>

   PRIVATE
      include/qyamlcpp/qyamlcpp.h
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
      src/linenumberarea.h
      src/linenumberarea.cpp
      src/stylesheethighlighter.h
      src/stylesheethighlighter.cpp
      src/mainwindow.cpp
      src/datastore.h
      src/datastore.cpp
      src/common.h
      src/common.cpp
      src/abstractlabelledwidget.cpp
      src/abstractlabelledspinbox.cpp
      src/labelledtextfield.cpp
      src/labelledlineedit.cpp
      src/labelledspinbox.cpp
      src/labelledcombobox.cpp
      src/extendedcolordialog.cpp
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
target_link_libraries(tst_parser PRIVATE yaml-cpp)# qyamlcpp)
target_link_libraries(tst_parser PRIVATE rapidfuzz::rapidfuzz)
#target_link_libraries(tst_parser PRIVATE "${CMAKE_CURRENT_LIST_DIR}/lib/libmemprims.a")
#target_link_libraries(tst_parser PRIVATE "${CMAKE_CURRENT_LIST_DIR}/lib/libsafec-1.0.a")
#target_link_libraries(tst_parser PRIVATE "${CMAKE_CURRENT_LIST_DIR}/lib/libsafeccore.a")

target_compile_definitions(tst_parser PRIVATE STYLESHEETEDITOR_LIBRARY)
