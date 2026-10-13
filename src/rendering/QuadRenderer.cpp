#include "QuadRenderer.h"

#include <algorithm>
#include <array>
#include <cstddef>

#include <SDL3/SDL.h>

QuadRenderer::~QuadRenderer()
{
    if (vbo_) {
        glDeleteBuffers(1, &vbo_);
    }
    if (vao_) {
        glDeleteVertexArrays(1, &vao_);
    }
}

bool QuadRenderer::initialize(const std::filesystem::path& shaderDirectory)
{
    if (!program_.initializeFromFiles(shaderDirectory / "quad.vert", shaderDirectory / "quad.frag")) {
        return false;
    }

    // Local quad coordinates: x right, y up; UV origin at the bottom left.
    struct Vertex { float x, y, u, v; };
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

    // Procedural RGBA8 checker: no external asset or decoder required yet.
    std::array<unsigned char, 8 * 8 * 4> pixels{};
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            const bool light = (x + y) % 2 == 0;
            const auto offset = static_cast<std::size_t>((y * 8 + x) * 4);
            pixels[offset] = light ? 103 : 39;
            pixels[offset + 1] = light ? 212 : 112;
            pixels[offset + 2] = light ? 180 : 137;
            pixels[offset + 3] = 255;
        }
    }
    if (!texture_.initializeRGBA8(8, 8, pixels.data(), pixels.size())) {
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
    const float side = static_cast<float>(std::min(width, height)) * 0.66f;
    program_.setVec2(scaleLocation_, side / static_cast<float>(width), side / static_cast<float>(height));
    texture_.bind();
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
