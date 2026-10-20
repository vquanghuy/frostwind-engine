#include "Texture2D.h"

#include <limits>
#include <utility>

#include <SDL3/SDL.h>
#include <glad/gl.h>

#include "Image.h"
#include "OpenGLChecks.h"

namespace frostwind {

Texture2D::~Texture2D()
{
    if (texture_ && detail::requireContext(context_, "Texture destruction")) {
        glDeleteTextures(1, &texture_);
    }
}

bool Texture2D::initializeRGBA8(int width, int height, const byte* pixels, usize byteCount)
{
    if (texture_ || !detail::requireOpenGL("Texture initialization") ||
        width <= 0 || height <= 0 || !pixels) {
        SDL_Log("Texture initialization requires an empty owner, positive dimensions, and pixels.");
        return false;
    }
    GLint maxSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    const usize w = width, h = height;
    if (!detail::checkOpenGL("Texture limit query") || width > maxSize || height > maxSize ||
        w > std::numeric_limits<usize>::max() / 4 / h || byteCount != w * h * 4) {
        SDL_Log("Texture dimensions or RGBA8 byte count are invalid.");
        return false;
    }
    Texture2D candidate;
    candidate.context_ = SDL_GL_GetCurrentContext();
    glGenTextures(1, &candidate.texture_);
    if (!candidate.texture_) {
        SDL_Log("Could not create OpenGL texture.");
        return false;
    }
    GLint binding = 0, alignment = 0, rowLength = 0, skipPixels = 0, skipRows = 0, unpackBuffer = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &binding);
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowLength);
    glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skipPixels);
    glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skipRows);
    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &unpackBuffer);
    glBindTexture(GL_TEXTURE_2D, candidate.texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpackBuffer);
    glBindTexture(GL_TEXTURE_2D, binding);
    if (!detail::checkOpenGL("Texture upload")) {
        return false;
    }
    std::swap(texture_, candidate.texture_);
    std::swap(context_, candidate.context_);
    return true;
}

bool Texture2D::initializeFromFile(const std::filesystem::path& path)
{
    if (texture_ || !detail::requireOpenGL("Texture file loading")) {
        SDL_Log("Texture file loading requires an empty owner and current context.");
        return false;
    }
    GLint maxSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    if (!detail::checkOpenGL("Texture limit query")) {
        return false;
    }
    Image image;
    if (!image.load(path, maxSize)) {
        return false;
    }
    return initializeRGBA8(image.width(), image.height(), image.pixels().data(), image.pixels().size());
}

bool Texture2D::bind(u32 unit) const
{
    if (!texture_ || !detail::requireContext(context_, "Texture binding")) {
        return false;
    }
    GLint units = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &units);
    if (units <= 0 || unit >= u32(units)) {
        SDL_Log("Texture unit exceeds the current context's limit.");
        return false;
    }
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindSampler(unit, 0);
    glBindTexture(GL_TEXTURE_2D, texture_);
    return detail::checkOpenGL("Texture binding");
}

} // namespace frostwind
