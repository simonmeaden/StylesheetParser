cmake_minimum_required(VERSION 3.5)

project(StylesheetParser LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core REQUIRED)

add_library(StylesheetParser SHARED
  StylesheetParser_global.h
  stylesheetparser.cpp
  stylesheetparser.h
)

target_link_libraries(StylesheetParser PRIVATE Qt5::Core)

target_compile_definitions(StylesheetParser PRIVATE STYLESHEETPARSER_LIBRARY)
