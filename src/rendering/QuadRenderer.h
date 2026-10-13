#pragma once

#include <glad/gl.h>

// Temporary graphics demo; replace when terrain rendering is implemented.
// Resources must be destroyed while their OpenGL context is still current.
class QuadRenderer {
public:
    QuadRenderer() = default;
    ~QuadRenderer();
    QuadRenderer(const QuadRenderer&) = delete;
    QuadRenderer& operator=(const QuadRenderer&) = delete;

    bool initialize();
    void draw(int width, int height) const;

private:
    GLuint program_ = 0;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint texture_ = 0;
    GLint scaleLocation_ = -1;
};
