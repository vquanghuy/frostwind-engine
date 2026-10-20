#include "Image.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <memory>
#include <utility>

#include <SDL3/SDL.h>
#include <stb_image.h>

namespace frostwind {

bool Image::load(const std::filesystem::path& path, int maxDimension)
{
    if (!pixels_.empty() || maxDimension <= 0) {
        SDL_Log("Image loading requires an empty image and a positive dimension limit.");
        return false;
    }
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    const std::streamoff size = file ? file.tellg() : std::streampos(-1);
    if (size <= 0 || size > std::numeric_limits<int>::max()) {
        SDL_Log("Could not read image (missing, empty, or oversized): %s", path.u8string().c_str());
        return false;
    }
    const int length = size;
    std::vector<byte> encoded(length);
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(encoded.data()), length)) {
        SDL_Log("Could not read complete image file: %s", path.u8string().c_str());
        return false;
    }
    int width = 0, height = 0, channels = 0;
    if (!stbi_info_from_memory(encoded.data(), length, &width, &height, &channels)) {
        SDL_Log("Could not inspect image %s: %s", path.u8string().c_str(), stbi_failure_reason());
        return false;
    }
    if (width <= 0 || height <= 0 || width > maxDimension || height > maxDimension) {
        SDL_Log("Image dimensions exceed the supplied limit of %d: %s", maxDimension, path.u8string().c_str());
        return false;
    }
    const usize w = width;
    const usize h = height;
    if (w > std::numeric_limits<usize>::max() / 4 / h) {
        SDL_Log("Image byte count exceeds addressable memory: %s", path.u8string().c_str());
        return false;
    }
    const int inspectedWidth = width;
    const int inspectedHeight = height;
    using Pixels = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;
    Pixels decoded(stbi_load_from_memory(encoded.data(), length, &width, &height, &channels, STBI_rgb_alpha), stbi_image_free);
    if (!decoded) {
        SDL_Log("Could not decode image %s: %s", path.u8string().c_str(), stbi_failure_reason());
        return false;
    }
    if (width != inspectedWidth || height != inspectedHeight) {
        SDL_Log("Decoded image dimensions do not match the inspected dimensions.");
        return false;
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
    return true;
}

} // namespace frostwind
