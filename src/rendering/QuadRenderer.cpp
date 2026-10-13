#include "QuadRenderer.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

namespace {

// Embedded for this first native rendering experiment; browser shader variants
// will be introduced when the Emscripten build is configured.
constexpr const char* vertexSource = R"glsl(#version 330 core
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aUV;
uniform vec2 uScale;
out vec2 vUV;
void main() {
    vUV = aUV;
    gl_Position = vec4(aPosition * uScale, 0.0, 1.0);
}
)glsl";

constexpr const char* fragmentSource = R"glsl(#version 330 core
in vec2 vUV;
uniform sampler2D uTexture;
out vec4 fragColor;
void main() {
    fragColor = texture(uTexture, vUV);
}
)glsl";

GLuint compileShader(GLenum type, const char* source)
{
    const GLuint shader = glCreateShader(type);
    if (!shader) {
        SDL_Log("Could not create OpenGL shader.");
        return 0;
    }
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(static_cast<std::size_t>(std::max(length, 1)), '\0');
        glGetShaderInfoLog(shader, length, nullptr, log.data());
        SDL_Log("%s shader compilation failed: %s",
            type == GL_VERTEX_SHADER ? "Vertex" : "Fragment", log.data());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

} // namespace

QuadRenderer::~QuadRenderer()
{
    if (texture_) {
        glDeleteTextures(1, &texture_);
    }
    if (vbo_) {
        glDeleteBuffers(1, &vbo_);
    }
    if (vao_) {
        glDeleteVertexArrays(1, &vao_);
    }
    if (program_) {
        glDeleteProgram(program_);
    }
}

bool QuadRenderer::initialize()
{
    const GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
    if (!vertex) {
        return false;
    }
    const GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!fragment) {
        glDeleteShader(vertex);
        return false;
    }

    program_ = glCreateProgram();
    if (!program_) {
        SDL_Log("Could not create OpenGL program.");
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return false;
    }
    glAttachShader(program_, vertex);
    glAttachShader(program_, fragment);
    glLinkProgram(program_);
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    GLint success = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        GLint length = 0;
        glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(static_cast<std::size_t>(std::max(length, 1)), '\0');
        glGetProgramInfoLog(program_, length, nullptr, log.data());
        SDL_Log("Shader program linking failed: %s", log.data());
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
    glGenTextures(1, &texture_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    glUseProgram(program_);
    scaleLocation_ = glGetUniformLocation(program_, "uScale");
    const GLint textureLocation = glGetUniformLocation(program_, "uTexture");
    if (scaleLocation_ < 0 || textureLocation < 0) {
        SDL_Log("Required quad shader uniform is missing.");
        return false;
    }
    glUniform1i(textureLocation, 0);
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
    glUseProgram(program_);
    const float side = static_cast<float>(std::min(width, height)) * 0.66f;
    glUniform2f(scaleLocation_, side / static_cast<float>(width), side / static_cast<float>(height));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
