#pragma once

#include <filesystem>

#include <core/Types.h>
#include <glad/gl.h>

namespace frostwind {

// Owns a 2D texture. The context must outlive this object.
class Texture2D {
public:
    Texture2D() = default;
    ~Texture2D();
    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    // Call at startup with the single OpenGL context current and GLAD loaded.
    // Configures image orientation and caches the context's texture-size limit.
    static bool initializeSupport();

    // Tightly packed RGBA8, first row at the bottom; copies pixels to the GPU.
    // Uses nearest filtering and clamp-to-edge without mipmaps.
    bool initializeRGBA8(int width, int height, const byte* pixels, usize byteCount);
    // PNG/JPEG decoded to RGBA8 and flipped to the bottom-left UV convention.
    // Application startup initializes texture support before textures are loaded.
    // Relative paths resolve against the caller's working directory.
    bool initializeFromFile(const std::filesystem::path& path);
    void bind(u32 unit = 0) const;

private:
    inline static GLint maxSize_ = 0;
    GLuint texture_ = 0;
};

} // namespace frostwind
