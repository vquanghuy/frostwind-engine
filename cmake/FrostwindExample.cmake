function(frostwind_enable_warnings target)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    elseif(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    endif()
endfunction()

# An optional second argument selects the example's runtime asset directory.
function(frostwind_configure_example target)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${target}"
    )

    # Keep the official SDL SDK beside each executable.
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set_target_properties(${target} PROPERTIES
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH "@loader_path/Frameworks"
        )
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:${target}>/Frameworks"
            COMMAND /usr/bin/ditto "${SDL3_FRAMEWORK}" "$<TARGET_FILE_DIR:${target}>/Frameworks/SDL3.framework"
            COMMENT "Copying official SDL3 framework beside ${target}"
            VERBATIM
        )
    elseif(WIN32)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "$<TARGET_FILE:SDL3::SDL3>" "$<TARGET_FILE_DIR:${target}>"
            COMMENT "Copying SDL3.dll beside ${target} (unverified platform)"
            VERBATIM
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set_target_properties(${target} PROPERTIES
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH "$ORIGIN/lib"
        )
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${SDL3_SDK}/lib" "$<TARGET_FILE_DIR:${target}>/lib"
            COMMENT "Copying local SDL3 libraries beside ${target} (unverified platform)"
            VERBATIM
        )
    endif()

    if(ARGC GREATER 1)
        # Runs on every example build, including asset-only changes.
        add_custom_target(${target}-assets
            COMMAND "${CMAKE_COMMAND}" -E copy_directory
                "${ARGV1}" "$<TARGET_FILE_DIR:${target}>/assets"
            COMMENT "Copying runtime assets for ${target}"
            VERBATIM
        )
        add_dependencies(${target} ${target}-assets)
    endif()
endfunction()
