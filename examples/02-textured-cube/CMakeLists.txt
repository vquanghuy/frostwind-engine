add_executable(frostwind-textured-cube
    main.cpp
    CubeDemo.cpp
    CubeDemo.h
)
target_link_libraries(frostwind-textured-cube PRIVATE frostwind)
set_target_properties(frostwind-textured-cube PROPERTIES CXX_EXTENSIONS OFF)
frostwind_enable_warnings(frostwind-textured-cube)
frostwind_configure_example(frostwind-textured-cube)

# Rebuild this example after editing runtime shaders or textures.
add_custom_command(TARGET frostwind-textured-cube POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:frostwind-textured-cube>/assets"
    COMMENT "Copying Textured Cube runtime assets"
    VERBATIM
)
