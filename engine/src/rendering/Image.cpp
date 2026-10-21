#include "Image.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <stb_image.h>

namespace frostwind {

void Image::load(const std::filesystem::path& path, int maxDimension)
{
    if (!pixels_.empty() || maxDimension <= 0) {
        throw std::runtime_error("[Image] Image loading requires an empty image and a positive dimension limit.");
    }
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    const std::streamoff size = file ? file.tellg() : std::streampos(-1);
    if (size <= 0 || size > std::numeric_limits<int>::max()) {
        throw std::runtime_error("[Image] Could not read image (missing, empty, or oversized): " + path.u8string());
    }
    const int length = size;
    std::vector<byte> encoded(length);
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(encoded.data()), length)) {
        throw std::runtime_error("[Image] Could not read complete image file: " + path.u8string());
    }
    int width = 0, height = 0, channels = 0;
    if (!stbi_info_from_memory(encoded.data(), length, &width, &height, &channels)) {
        throw std::runtime_error("[Image] Could not inspect image " + path.u8string() + ": " + stbi_failure_reason());
    }
    if (width <= 0 || height <= 0 || width > maxDimension || height > maxDimension) {
        throw std::runtime_error("[Image] Image dimensions exceed the supplied limit of " + std::to_string(maxDimension) + ": " + path.u8string());
    }
    const usize w = width;
    const usize h = height;
    if (w > std::numeric_limits<usize>::max() / 4 / h) {
        throw std::runtime_error("[Image] Image byte count exceeds addressable memory: " + path.u8string());
    }
    const int inspectedWidth = width;
    const int inspectedHeight = height;
    using Pixels = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;
    Pixels decoded(stbi_load_from_memory(encoded.data(), length, &width, &height, &channels, STBI_rgb_alpha), stbi_image_free);
    if (!decoded) {
        throw std::runtime_error("[Image] Could not decode image " + path.u8string() + ": " + stbi_failure_reason());
    }
    if (width != inspectedWidth || height != inspectedHeight) {
        throw std::runtime_error("[Image] Decoded image dimensions do not match the inspected dimensions.");
    }
    std::vector<byte> pixels(decoded.get(), decoded.get() + w * h * 4);
    // Keep orientation local to this loader instead of changing decoder globals.
    const usize rowBytes = w * 4;
    for (usize row = 0; row < h / 2; ++row) {
        auto* lower = pixels.data() + row * rowBytes;
        auto* upper = pixels.data() + (h - row - 1) * rowBytes;
        std::swap_ranges(lower, lower + rowBytes, upper);
    }
    pixels_ = std::move(pixels);
    width_ = width;
    height_ = height;
}

} // namespace frostwind
