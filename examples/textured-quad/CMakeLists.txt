add_executable(toybox-textured-quad
    main.cpp
    QuadDemo.cpp
    QuadRenderer.cpp
)
target_link_libraries(toybox-textured-quad PRIVATE toybox)
set_target_properties(toybox-textured-quad PROPERTIES CXX_EXTENSIONS OFF)
toybox_enable_warnings(toybox-textured-quad)
toybox_configure_example(toybox-textured-quad "${CMAKE_CURRENT_SOURCE_DIR}/assets")
