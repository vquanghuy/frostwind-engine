# Generated GLAD sources are supplied locally; never fetch during configuration.
set(GLAD_ROOT "${CMAKE_CURRENT_LIST_DIR}/../external/glad")
foreach(required_file src/gl.c include/glad/gl.h include/KHR/khrplatform.h)
    if(NOT EXISTS "${GLAD_ROOT}/${required_file}")
        message(FATAL_ERROR "Missing GLAD file: ${GLAD_ROOT}/${required_file}. See external/glad/README.mini-artillery.md.")
    endif()
endforeach()

add_library(mini-artillery-glad STATIC "${GLAD_ROOT}/src/gl.c")
add_library(GLAD::GLAD ALIAS mini-artillery-glad)
target_include_directories(mini-artillery-glad SYSTEM PUBLIC "${GLAD_ROOT}/include")
set_target_properties(mini-artillery-glad PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS OFF
)
