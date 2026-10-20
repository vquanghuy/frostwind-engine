cmake_minimum_required(VERSION 3.22)

project(Frostwind VERSION 0.3.0 LANGUAGES C CXX)

include(cmake/SDL3.cmake)
include(cmake/GLM.cmake)
if(NOT EMSCRIPTEN)
    include(cmake/GLAD.cmake)
endif()

include(cmake/FrostwindExample.cmake)

add_subdirectory(engine)

option(FROSTWIND_BUILD_EXAMPLES "Build Frostwind examples" ON)
if(FROSTWIND_BUILD_EXAMPLES)
    add_subdirectory(examples/01-textured-quad)
    add_subdirectory(examples/02-textured-cube)
    add_subdirectory(examples/03-asteroids)
    add_subdirectory(examples/04-mini-artillery)
endif()

option(FROSTWIND_BUILD_TESTS "Build context-free camera, input, and update checks" OFF)
if(FROSTWIND_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
