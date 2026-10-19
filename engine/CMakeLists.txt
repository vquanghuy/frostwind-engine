add_library(frostwind STATIC
    src/app/Application.cpp
    src/platform/Window.cpp
    src/platform/internal/WindowSurface.cpp
    src/platform/opengl/OpenGLWindowSurface.cpp
    src/rendering/GraphicsDevice.cpp
    src/rendering/backends/opengl/OpenGLBackend.cpp
    src/rendering/ShaderProgram.cpp
    src/rendering/Texture2D.cpp
    src/rendering/StbImage.cpp
)
target_compile_features(frostwind PUBLIC cxx_std_17)
set_target_properties(frostwind PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories(frostwind PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(frostwind SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/external/stb")
target_link_libraries(frostwind PUBLIC SDL3::SDL3)
if(NOT EMSCRIPTEN)
    target_link_libraries(frostwind PUBLIC GLAD::GLAD)
endif()
frostwind_enable_warnings(frostwind)
