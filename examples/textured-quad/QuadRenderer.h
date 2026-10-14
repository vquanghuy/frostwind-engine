#pragma once

#include <filesystem>

#include <rendering/ShaderProgram.h>
#include <rendering/Texture2D.h>

// Geometry and rendering behavior for the textured-quad example.
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
    toybox::ShaderProgram program_;
    toybox::Texture2D texture_;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLint scaleLocation_ = -1;
};
