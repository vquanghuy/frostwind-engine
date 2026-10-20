#pragma once

#include <filesystem>

#include <core/Types.h>

struct SDL_GLContextState;

namespace frostwind {

// Direct OpenGL RGBA8 owner: nearest filtering, clamp-to-edge, no mipmaps.
// Its creating context must be current for use and destruction, and outlive it.
class Texture2D {
public:
    Texture2D() = default;
    ~Texture2D();
    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    // Positive dimensions within GL_MAX_TEXTURE_SIZE; exactly width*height*4
    // bytes, bottom row first. Upload preserves binding and pixel-unpack state.
    // One-time after success; failed initialization cleans up and permits retry.
    bool initializeRGBA8(int width, int height, const byte* pixels, usize byteCount);
    bool initializeFromFile(const std::filesystem::path& path);
    // Sets the active unit, binds this texture, and selects its own sampling.
    bool bind(u32 unit = 0) const;

private:
    u32 texture_ = 0;
    SDL_GLContextState* context_ = nullptr;
};

} // namespace frostwind
