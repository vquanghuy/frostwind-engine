#pragma once

#include <filesystem>

#include "ShaderProgram.h"
#include "Texture2D.h"

// Temporary graphics demo; replace when terrain rendering is implemented.
// Resources must be destroyed while their OpenGL context is still current.
class QuadRenderer {
public:
    QuadRenderer() = default;
    ~QuadRenderer();
    QuadRenderer(const QuadRenderer&) = delete;
    QuadRenderer& operator=(const QuadRenderer&) = delete;

    bool initialize(const std::filesystem::path& shaderDirectory);
    void draw(int width, int height) const;

private:
    ShaderProgram program_;
    Texture2D texture_;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLint scaleLocation_ = -1;
};
