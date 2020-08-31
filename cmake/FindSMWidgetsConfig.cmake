get_filename_component(SMWIDGETS_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(CMakeFindDependencyMacro)

list(APPEND CMAKE_MODULE_PATH ${MIN_SMWIDGETS_VERSION})

find_dependency(yamlcpp @MIN_SMWIDGETS_VERSION@)

if(NOT TARGET qyamlcpp)
    include("${SMWIDGETS_CMAKE_DIR}/qyamlcppConfigTargets.cmake")
endif()
