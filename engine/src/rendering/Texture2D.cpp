#include "Texture2D.h"

#include <fstream>
#include <limits>
#include <memory>
#include <vector>

#include <SDL3/SDL.h>
#include <stb_image.h>

namespace frostwind {

bool Texture2D::initializeSupport()
{
    maxSize_ = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize_);
    if (maxSize_ <= 0) {
        SDL_Log("Could not initialize texture support: invalid maximum texture size.");
        return false;
    }
    stbi_set_flip_vertically_on_load(true);
    return true;
}

Texture2D::~Texture2D()
{
    if (texture_) {
        glDeleteTextures(1, &texture_);
    }
}

bool Texture2D::initializeFromFile(const std::filesystem::path& path)
{
    if (maxSize_ <= 0) {
        SDL_Log("Texture2D support must be initialized before loading textures.");
        return false;
    }
    if (texture_) {
        SDL_Log("Texture2D is already initialized: %s", path.u8string().c_str());
        return false;
    }
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    const std::streamoff size = file ? file.tellg() : std::streampos(-1);
    if (size <= 0 || size > std::numeric_limits<int>::max()) {
        SDL_Log("Could not read texture (missing, empty, or oversized): %s", path.u8string().c_str());
        return false;
    }
    const int length = size;
    std::vector<byte> encoded(length);
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(encoded.data()), length)) {
        SDL_Log("Could not read complete texture file: %s", path.u8string().c_str());
        return false;
    }

    int width = 0, height = 0, channels = 0;
    if (!stbi_info_from_memory(encoded.data(), length, &width, &height, &channels)) {
        SDL_Log("Could not inspect texture %s: %s", path.u8string().c_str(), stbi_failure_reason());
        return false;
    }
    if (width <= 0 || height <= 0 || width > maxSize_ || height > maxSize_) {
        SDL_Log("Texture dimensions are invalid or exceed the device limit of %d: %s", maxSize_, path.u8string().c_str());
        return false;
    }
    const usize w = width;
    const usize h = height;
    if (w > std::numeric_limits<usize>::max() / 4 / h) {
        SDL_Log("Texture byte count exceeds addressable memory: %s", path.u8string().c_str());
        return false;
    }
    using ImagePixels = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;
    ImagePixels pixels(stbi_load_from_memory(encoded.data(), length, &width, &height, &channels, STBI_rgb_alpha), stbi_image_free);
    if (!pixels) {
        SDL_Log("Could not decode texture %s: %s", path.u8string().c_str(), stbi_failure_reason());
        return false;
    }

    const auto byteCount = w * h * 4;
    if (!initializeRGBA8(width, height, pixels.get(), byteCount)) {
        SDL_Log("Could not upload texture: %s", path.u8string().c_str());
        return false;
    }
    return true;
}

bool Texture2D::initializeRGBA8(int width, int height, const byte* pixels, usize byteCount)
{
    if (maxSize_ <= 0) {
        SDL_Log("Texture2D support must be initialized before uploading textures.");
        return false;
    }
    if (texture_) {
        SDL_Log("Texture2D is already initialized.");
        return false;
    }
    if (width <= 0 || height <= 0 || !pixels) {
        SDL_Log("Texture2D requires positive dimensions and RGBA8 pixels.");
        return false;
    }
    const usize w = width;
    const usize h = height;
    if (w > std::numeric_limits<usize>::max() / 4 / h || byteCount != w * h * 4) {
        SDL_Log("Texture2D pixel byte count does not match its RGBA8 dimensions.");
        return false;
    }
    if (width > maxSize_ || height > maxSize_) {
        SDL_Log("Texture2D dimensions exceed the device limit of %d.", maxSize_);
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
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpackBuffer);

    const GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        SDL_Log("Texture2D upload failed: OpenGL error 0x%x", error);
        return false;
    }
    texture_ = candidate.texture_;
    candidate.texture_ = 0;
    return true;
}

void Texture2D::bind(u32 unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture_);
}

} // namespace frostwind
