cmake_minimum_required(VERSION 3.22)

project(Frostwind VERSION 0.3.0 LANGUAGES C CXX)

include(cmake/SDL3.cmake)
if(NOT EMSCRIPTEN)
    include(cmake/GLAD.cmake)
endif()
include(cmake/FrostwindExample.cmake)

add_subdirectory(engine)

option(FROSTWIND_BUILD_EXAMPLES "Build Frostwind examples" ON)
if(FROSTWIND_BUILD_EXAMPLES)
    add_subdirectory(examples/01-textured-quad)
    add_subdirectory(examples/02-asteroids)
    add_subdirectory(examples/03-mini-artillery)
endif()
