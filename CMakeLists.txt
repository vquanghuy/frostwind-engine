cmake_minimum_required(VERSION 3.22)

project(Toybox VERSION 0.2.0 LANGUAGES C CXX)

include(cmake/SDL3.cmake)
if(NOT EMSCRIPTEN)
    include(cmake/GLAD.cmake)
endif()
include(cmake/ToyboxExample.cmake)

add_subdirectory(engine)

option(TOYBOX_BUILD_EXAMPLES "Build Toybox examples" ON)
if(TOYBOX_BUILD_EXAMPLES)
    add_subdirectory(examples/textured-quad)
endif()
