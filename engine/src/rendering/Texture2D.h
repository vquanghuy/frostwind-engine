#pragma once

#include <cstddef>

#include <glad/gl.h>

namespace toybox {

// Owns a 2D texture. The context must outlive this object.
class Texture2D {
public:
    Texture2D() = default;
    ~Texture2D();
    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    // Tightly packed RGBA8, first row at the bottom; copies pixels to the GPU.
    // Uses nearest filtering and clamp-to-edge without mipmaps.
    bool initializeRGBA8(int width, int height, const unsigned char* pixels, std::size_t byteCount);
    void bind(unsigned int unit = 0) const;

private:
    GLuint texture_ = 0;
};

} // namespace toybox
