add_executable(frostwind-textured-quad
    main.cpp
    QuadDemo.cpp
    QuadDemo.h
)
target_link_libraries(frostwind-textured-quad PRIVATE frostwind)
set_target_properties(frostwind-textured-quad PROPERTIES CXX_EXTENSIONS OFF)
frostwind_enable_warnings(frostwind-textured-quad)
frostwind_configure_example(frostwind-textured-quad)

# Rebuild this example after editing runtime shaders or textures.
add_custom_command(TARGET frostwind-textured-quad POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:frostwind-textured-quad>/assets"
    COMMENT "Copying Textured Quad runtime assets"
    VERBATIM
)
