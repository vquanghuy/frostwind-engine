#pragma once

#include <filesystem>

#include <core/Types.h>

struct SDL_GLContextState;

namespace frostwind {

enum class TextureFilter { Nearest, Trilinear };

struct TextureSampling {
    TextureFilter filter = TextureFilter::Nearest;
    // Finite and >= 1. Clamp to the context's limit; unsupported falls back to 1.
    f32 anisotropy = 1;
};

// Direct OpenGL RGBA8 owner with clamp-to-edge wrapping. Default sampling is
// nearest/no mipmaps; Trilinear generates mipmaps and uses linear magnification.
// Its creating context must be current for use and destruction, and outlive it.
// Initialization/use throw std::runtime_error with operation diagnostics.
// Destruction never throws.
class Texture2D {
public:
    Texture2D() = default;
    ~Texture2D() noexcept;
    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    // Positive dimensions within GL_MAX_TEXTURE_SIZE; exactly width*height*4
    // bytes, bottom row first. Requires default pixel-unpack state and no unpack
    // buffer bound. Upload preserves the prior texture binding.
    // One-time after success; failed initialization cleans up and permits retry.
    void initializeRGBA8(int width, int height, const byte* pixels, usize byteCount,
        const TextureSampling& sampling = {});
    void initializeFromFile(const std::filesystem::path& path, const TextureSampling& sampling = {});
    // Sets the active unit, binds this texture, and selects its own sampling.
    void bind(u32 unit = 0) const;

private:
    u32 texture_ = 0;
    SDL_GLContextState* context_ = nullptr;
};

} // namespace frostwind
