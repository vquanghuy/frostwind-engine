# Generated GLAD sources are supplied locally; never fetch during configuration.
set(GLAD_ROOT "${CMAKE_CURRENT_LIST_DIR}/../external/glad")
foreach(required_file src/gl.c include/glad/gl.h include/KHR/khrplatform.h)
    if(NOT EXISTS "${GLAD_ROOT}/${required_file}")
        message(FATAL_ERROR "Missing GLAD file: ${GLAD_ROOT}/${required_file}. See external/glad/README.integration.md.")
    endif()
endforeach()

add_library(toybox-glad STATIC "${GLAD_ROOT}/src/gl.c")
add_library(GLAD::GLAD ALIAS toybox-glad)
target_include_directories(toybox-glad SYSTEM PUBLIC "${GLAD_ROOT}/include")
set_target_properties(toybox-glad PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS OFF
)
