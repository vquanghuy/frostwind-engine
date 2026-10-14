add_library(toybox STATIC
    src/app/Application.cpp
    src/platform/Window.cpp
    src/rendering/ShaderProgram.cpp
    src/rendering/Texture2D.cpp
)
target_compile_features(toybox PUBLIC cxx_std_17)
set_target_properties(toybox PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories(toybox PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(toybox PUBLIC SDL3::SDL3)
if(NOT EMSCRIPTEN)
    target_link_libraries(toybox PUBLIC GLAD::GLAD)
endif()
toybox_enable_warnings(toybox)
