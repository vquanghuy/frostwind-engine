#include "GraphicsDevice.h"

#include <utility>

#include <SDL3/SDL.h>

#include "internal/GraphicsBackend.h"
#include "internal/GraphicsBackendFactory.h"

namespace frostwind {

GraphicsDevice::GraphicsDevice() = default;
GraphicsDevice::~GraphicsDevice() = default;

bool GraphicsDevice::initialize(Window& window, const GraphicsConfig& config)
{
    if (backend_) {
        SDL_Log("GraphicsDevice is already initialized.");
        return false;
    }
    auto candidate = detail::createGraphicsBackend();
    if (!candidate) {
        SDL_Log("Could not create the compiled graphics backend.");
        return false;
    }
    if (!candidate->initialize(window, config)) {
        return false;
    }
    backend_ = std::move(candidate);
    return true;
}

bool GraphicsDevice::present()
{
    if (!backend_) {
        SDL_Log("Cannot present before GraphicsDevice initialization.");
        return false;
    }
    return backend_->present();
}

} // namespace frostwind
