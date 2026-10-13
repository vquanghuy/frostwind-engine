cmake_minimum_required(VERSION 3.22)

project(MiniArtillery VERSION 0.2.0 LANGUAGES C CXX)

include(cmake/SDL3.cmake)

add_executable(mini-artillery
    src/main.cpp
    src/app/Application.cpp
    src/platform/Window.cpp
    src/rendering/QuadRenderer.cpp
    src/rendering/ShaderProgram.cpp
    src/rendering/Texture2D.cpp
)
target_compile_features(mini-artillery PRIVATE cxx_std_17)
set_target_properties(mini-artillery PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(mini-artillery PRIVATE SDL3::SDL3)

if(NOT EMSCRIPTEN)
    include(cmake/GLAD.cmake)
    target_link_libraries(mini-artillery PRIVATE GLAD::GLAD)
endif()

# Keep the executable runnable without a system-wide SDL installation.
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_target_properties(mini-artillery PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "@loader_path/Frameworks"
    )
    add_custom_command(TARGET mini-artillery POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:mini-artillery>/Frameworks"
        COMMAND /usr/bin/ditto "${SDL3_FRAMEWORK}" "$<TARGET_FILE_DIR:mini-artillery>/Frameworks/SDL3.framework"
        COMMENT "Copying official SDL3 framework beside the executable"
        VERBATIM
    )
elseif(WIN32)
    add_custom_command(TARGET mini-artillery POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE:SDL3::SDL3>" "$<TARGET_FILE_DIR:mini-artillery>"
        COMMENT "Copying SDL3.dll beside the executable (unverified platform)"
        VERBATIM
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(mini-artillery PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "$ORIGIN/lib"
    )
    add_custom_command(TARGET mini-artillery POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
            "${SDL3_SDK}/lib" "$<TARGET_FILE_DIR:mini-artillery>/lib"
        COMMENT "Copying local SDL3 libraries beside the executable (unverified platform)"
        VERBATIM
    )
endif()

# Keep runtime shaders current even when only an asset changes.
add_custom_target(mini-artillery-assets
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders"
        "$<TARGET_FILE_DIR:mini-artillery>/assets/shaders"
    COMMENT "Copying runtime shaders beside the executable"
    VERBATIM
)
add_dependencies(mini-artillery mini-artillery-assets)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(mini-artillery PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(mini-artillery PRIVATE /W4)
endif()
