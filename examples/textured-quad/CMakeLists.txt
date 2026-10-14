add_executable(frostwind-textured-quad
    main.cpp
    QuadDemo.cpp
    QuadRenderer.cpp
)
target_link_libraries(frostwind-textured-quad PRIVATE frostwind)
set_target_properties(frostwind-textured-quad PROPERTIES CXX_EXTENSIONS OFF)
frostwind_enable_warnings(frostwind-textured-quad)
frostwind_configure_example(frostwind-textured-quad "${CMAKE_CURRENT_SOURCE_DIR}/assets")
