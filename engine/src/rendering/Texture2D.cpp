#include "Texture2D.h"

#include <SDL3/SDL.h>

#include "GraphicsDevice.h"
#include "Image.h"

namespace frostwind {

Texture2D::~Texture2D()
{
    if (device_) {
        device_->release(detail::ResourceKind::Texture, handle_.value);
    }
}

bool Texture2D::initializeRGBA8(GraphicsDevice& device, int width, int height, const byte* pixels, usize byteCount)
{
    return device.create(*this, width, height, pixels, byteCount);
}

bool Texture2D::initializeFromFile(GraphicsDevice& device, const std::filesystem::path& path)
{
    if (device_) {
        SDL_Log("Texture2D is already initialized.");
        return false;
    }
    Image image;
    if (!image.load(path, device.limits().maxTextureSize)) {
        return false;
    }
    return initializeRGBA8(device, image.width(), image.height(), image.pixels().data(), image.pixels().size());
}

} // namespace frostwind
