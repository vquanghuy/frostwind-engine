#include "Texture2D.h"

#include <limits>

#include <SDL3/SDL.h>

namespace toybox {

Texture2D::~Texture2D()
{
    if (texture_) {
        glDeleteTextures(1, &texture_);
    }
}

bool Texture2D::initializeRGBA8(int width, int height, const unsigned char* pixels, std::size_t byteCount)
{
    if (texture_) {
        SDL_Log("Texture2D is already initialized.");
        return false;
    }
    if (width <= 0 || height <= 0 || !pixels) {
        SDL_Log("Texture2D requires positive dimensions and RGBA8 pixels.");
        return false;
    }
    const auto w = static_cast<std::size_t>(width);
    const auto h = static_cast<std::size_t>(height);
    if (w > std::numeric_limits<std::size_t>::max() / 4 / h || byteCount != w * h * 4) {
        SDL_Log("Texture2D pixel byte count does not match its RGBA8 dimensions.");
        return false;
    }
    GLint maxSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    if (width > maxSize || height > maxSize) {
        SDL_Log("Texture2D dimensions exceed the device limit of %d.", maxSize);
        return false;
    }

    Texture2D candidate;
    glGenTextures(1, &candidate.texture_);
    if (!candidate.texture_) {
        SDL_Log("Could not create OpenGL texture.");
        return false;
    }
    candidate.bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Client-memory uploads must not inherit a PBO or another upload's layout.
    GLint alignment = 0, rowLength = 0, skipPixels = 0, skipRows = 0, unpackBuffer = 0;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowLength);
    glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skipPixels);
    glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skipRows);
    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &unpackBuffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, skipPixels);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, skipRows);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, static_cast<GLuint>(unpackBuffer));

    const GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        SDL_Log("Texture2D upload failed: OpenGL error 0x%x", error);
        return false;
    }
    texture_ = candidate.texture_;
    candidate.texture_ = 0;
    return true;
}

void Texture2D::bind(unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture_);
}

} // namespace toybox
