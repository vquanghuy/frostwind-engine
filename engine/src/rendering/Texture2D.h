#pragma once

#include <filesystem>

#include <core/Types.h>

#include "internal/ResourceHandle.h"

namespace frostwind {

class GraphicsDevice;

// RGBA8, nearest filtering, clamp-to-edge, no mipmaps. Device must outlive it.
class Texture2D {
public:
    Texture2D() = default;
    ~Texture2D();
    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    // Requires an initialized device, positive dimensions within its limit, and
    // exactly width * height * 4 tightly packed bytes, first row at the bottom.
    // Copied/consumed before return; one-time after success, retry after failure.
    bool initializeRGBA8(GraphicsDevice& device, int width, int height, const byte* pixels, usize byteCount);
    // CPU Image decode followed by upload, using the device dimension limit.
    // Relative paths use the caller's working directory; logs identify failures.
    bool initializeFromFile(GraphicsDevice& device, const std::filesystem::path& path);

private:
    friend class GraphicsDevice;
    GraphicsDevice* device_ = nullptr;
    detail::TextureHandle handle_;
};

} // namespace frostwind
