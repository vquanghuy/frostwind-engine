#include "QuadRenderer.h"

#include <algorithm>
#include <cstddef>

#include <SDL3/SDL.h>

#include <core/Types.h>

QuadRenderer::~QuadRenderer()
{
    if (vbo_) {
        glDeleteBuffers(1, &vbo_);
    }
    if (vao_) {
        glDeleteVertexArrays(1, &vao_);
    }
}

bool QuadRenderer::initialize(const std::filesystem::path& assetDirectory)
{
    if (!program_.initializeFromFiles(assetDirectory / "shaders" / "quad.vert", assetDirectory / "shaders" / "quad.frag")) {
        return false;
    }

    // Local quad coordinates: x right, y up; UV origin at the bottom left.
    struct Vertex { frostwind::f32 x, y, u, v; };
    constexpr Vertex vertices[] = {
        {-1, -1, 0, 0}, {1, -1, 1, 0}, {1, 1, 1, 1},
        {-1, -1, 0, 0}, {1, 1, 1, 1}, {-1, 1, 0, 1},
    };
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        reinterpret_cast<const void*>(offsetof(Vertex, u)));
    glEnableVertexAttribArray(1);

    if (!texture_.initializeFromFile(assetDirectory / "textures" / "crate.png")) {
        return false;
    }

    program_.bind();
    scaleLocation_ = program_.uniformLocation("uScale");
    const GLint textureLocation = program_.uniformLocation("uTexture");
    if (scaleLocation_ < 0 || textureLocation < 0) {
        SDL_Log("Required quad shader uniform is missing.");
        return false;
    }
    program_.setInt(textureLocation, 0);
    glBindVertexArray(0);
    const GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        SDL_Log("Quad resource initialization failed: OpenGL error 0x%x", error);
        return false;
    }
    return true;
}

void QuadRenderer::draw(int width, int height) const
{
    glViewport(0, 0, width, height);
    glClearColor(24.0f / 255.0f, 28.0f / 255.0f, 34.0f / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    program_.bind();
    const frostwind::f32 side = std::min(width, height) * 0.66f;
    program_.setVec2(scaleLocation_, side / width, side / height);
    texture_.bind();
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
