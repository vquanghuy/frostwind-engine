#pragma once

#include <filesystem>
#include <vector>

#include <core/Types.h>

namespace frostwind {

// CPU-only, tightly packed RGBA8 pixels, first row at the bottom. No graphics
// initialization is required. Loading is fallible and one-time after success.
class Image {
public:
    // Inspect dimensions against maxDimension before full decode. Requires an
    // empty image and positive limit. False logs file/decode/size failures and
    // permits retry. Relative paths use the caller's working directory.
    bool load(const std::filesystem::path& path, int maxDimension);
    int width() const { return width_; }
    int height() const { return height_; }
    const std::vector<byte>& pixels() const { return pixels_; }

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<byte> pixels_;
};

} // namespace frostwind
