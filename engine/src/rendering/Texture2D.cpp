#include "Texture2D.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

#include <SDL3/SDL.h>
#include <glad/gl.h>

#include "Image.h"
#include "GraphicsChecks.h"

namespace frostwind {
namespace {

// EXT/ARB_texture_filter_anisotropic and OpenGL 4.6 share these tokens.
// They are absent from our pinned OpenGL 3.3-only GLAD header.
constexpr GLenum textureMaxAnisotropy = 0x84FE;
constexpr GLenum maxTextureMaxAnisotropy = 0x84FF;

f32 selectAnisotropy(const TextureSampling& sampling)
{
    if ((sampling.filter != TextureFilter::Nearest && sampling.filter != TextureFilter::Trilinear) ||
        !std::isfinite(sampling.anisotropy) || sampling.anisotropy < 1) {
        throw std::runtime_error("Texture sampling requires a known filter and finite anisotropy >= 1.");
    }
    if (sampling.anisotropy == 1) {
        return 1;
    }
    GLint major = 0, minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    const bool supported = major > 4 || (major == 4 && minor >= 6) ||
        SDL_GL_ExtensionSupported("GL_EXT_texture_filter_anisotropic") ||
        SDL_GL_ExtensionSupported("GL_ARB_texture_filter_anisotropic");
    GLfloat maximum = 1;
    if (supported) {
        glGetFloatv(maxTextureMaxAnisotropy, &maximum);
    }
    detail::checkOpenGL("Texture anisotropy limit query");
    if (!std::isfinite(maximum) || maximum < 1) {
        throw std::runtime_error("Texture anisotropy limit is invalid.");
    }
    return std::min(sampling.anisotropy, maximum);
}

} // namespace

Texture2D::~Texture2D() noexcept
{
    if (texture_ && detail::canDestroy(context_, "Texture destruction")) {
        glDeleteTextures(1, &texture_);
    }
}

void Texture2D::initializeRGBA8(int width, int height, const byte* pixels, usize byteCount,
    const TextureSampling& sampling)
{
    detail::requireOpenGL("Texture initialization");
    if (texture_ || width <= 0 || height <= 0 || !pixels) {
        throw std::runtime_error("Texture initialization requires an empty owner, positive dimensions, and pixels.");
    }
    const f32 anisotropy = selectAnisotropy(sampling);
    GLint maxSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    const usize w = width, h = height;
    detail::checkOpenGL("Texture limit query");
    if (width > maxSize || height > maxSize ||
        w > std::numeric_limits<usize>::max() / 4 / h || byteCount != w * h * 4) {
        throw std::runtime_error("Texture dimensions or RGBA8 byte count are invalid.");
    }
    Texture2D candidate;
    candidate.context_ = SDL_GL_GetCurrentContext();
    glGenTextures(1, &candidate.texture_);
    if (!candidate.texture_) {
        throw std::runtime_error("Could not create OpenGL texture.");
    }
    GLint binding = 0, alignment = 0, rowLength = 0, skipPixels = 0, skipRows = 0, unpackBuffer = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &binding);
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowLength);
    glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skipPixels);
    glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skipRows);
    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &unpackBuffer);
    glBindTexture(GL_TEXTURE_2D, candidate.texture_);
    const bool trilinear = sampling.filter == TextureFilter::Trilinear;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, trilinear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, trilinear ? GL_LINEAR : GL_NEAREST);
    if (anisotropy > 1) {
        glTexParameterf(GL_TEXTURE_2D, textureMaxAnisotropy, anisotropy);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    if (trilinear) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, skipPixels);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, skipRows);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpackBuffer);
    glBindTexture(GL_TEXTURE_2D, binding);
    detail::checkOpenGL("Texture upload");
    std::swap(texture_, candidate.texture_);
    std::swap(context_, candidate.context_);
    if (sampling.anisotropy > 1) {
        SDL_Log("Texture anisotropy requested %gx; applied %gx.", sampling.anisotropy, anisotropy);
    }
}

void Texture2D::initializeFromFile(const std::filesystem::path& path, const TextureSampling& sampling)
{
    detail::requireOpenGL("Texture file loading");
    if (texture_) {
        throw std::runtime_error("Texture file loading requires an empty owner and current context.");
    }
    GLint maxSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
    detail::checkOpenGL("Texture limit query");
    Image image;
    image.load(path, maxSize);
    initializeRGBA8(image.width(), image.height(), image.pixels().data(), image.pixels().size(), sampling);
}

void Texture2D::bind(u32 unit) const
{
    detail::requireContext(context_, "Texture binding");
    GLint units = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &units);
    if (units <= 0 || unit >= u32(units)) {
        throw std::runtime_error("Texture unit exceeds the current context's limit.");
    }
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindSampler(unit, 0);
    glBindTexture(GL_TEXTURE_2D, texture_);
    detail::checkOpenGL("Texture binding");
}

} // namespace frostwind
