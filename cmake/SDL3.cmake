# Match each platform's headers and binary behind one application target.
set(SDL3_ROOT "${CMAKE_CURRENT_LIST_DIR}/../external/sdl3")

function(require_sdl3_file path)
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR
            "Missing local SDL3 SDK file: ${path}\n"
            "See external/sdl3/README.mini-artillery.md for the required layout. "
            "Windows and Linux SDKs are not supplied or verified yet.")
    endif()
endfunction()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(SDL3_FRAMEWORK "${SDL3_ROOT}/macos-universal/SDL3.framework")
    require_sdl3_file("${SDL3_FRAMEWORK}/SDL3")
    add_library(SDL3::SDL3 SHARED IMPORTED GLOBAL)
    set_target_properties(SDL3::SDL3 PROPERTIES
        FRAMEWORK TRUE
        IMPORTED_LOCATION "${SDL3_FRAMEWORK}/SDL3"
        IMPORTED_SONAME "@rpath/SDL3.framework/Versions/A/SDL3"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Prepared for the official Visual C++ x64 development package; unverified.
    if(NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "The prepared Windows SDK layout requires MSVC targeting x64.")
    endif()
    if(CMAKE_GENERATOR_PLATFORM AND NOT CMAKE_GENERATOR_PLATFORM STREQUAL "x64")
        message(FATAL_ERROR "The prepared Windows SDL3 SDK supports x64 only.")
    endif()
    set(SDL3_SDK "${SDL3_ROOT}/windows-x64")
    require_sdl3_file("${SDL3_SDK}/include/SDL3/SDL.h")
    require_sdl3_file("${SDL3_SDK}/lib/SDL3.lib")
    require_sdl3_file("${SDL3_SDK}/bin/SDL3.dll")
    add_library(SDL3::SDL3 SHARED IMPORTED GLOBAL)
    set_target_properties(SDL3::SDL3 PROPERTIES
        IMPORTED_IMPLIB "${SDL3_SDK}/lib/SDL3.lib"
        IMPORTED_LOCATION "${SDL3_SDK}/bin/SDL3.dll"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL3_SDK}/include"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Prepared for a separately supplied Linux x86_64 SDK; unverified.
    if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|amd64|AMD64)$")
        message(FATAL_ERROR "The prepared Linux SDL3 SDK layout supports x86_64 only.")
    endif()
    set(SDL3_SDK "${SDL3_ROOT}/linux-x64")
    require_sdl3_file("${SDL3_SDK}/include/SDL3/SDL.h")
    require_sdl3_file("${SDL3_SDK}/lib/libSDL3.so.0")
    add_library(SDL3::SDL3 SHARED IMPORTED GLOBAL)
    set_target_properties(SDL3::SDL3 PROPERTIES
        IMPORTED_LOCATION "${SDL3_SDK}/lib/libSDL3.so.0"
        IMPORTED_SONAME "libSDL3.so.0"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL3_SDK}/include"
    )
else()
    message(FATAL_ERROR "No local SDL3 SDK integration for ${CMAKE_SYSTEM_NAME}. Browser support is pending.")
endif()
