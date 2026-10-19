#pragma once

#include <memory>

namespace frostwind {

class Window;
namespace detail {
class GraphicsBackend;
}

struct GraphicsConfig {
    bool vsync = true;
};

// Owns the compiled graphics backend and borrows Window. Example GPU resources
// must be destroyed before this device, and the device before its window.
class GraphicsDevice {
public:
    GraphicsDevice();
    ~GraphicsDevice();
    GraphicsDevice(const GraphicsDevice&) = delete;
    GraphicsDevice& operator=(const GraphicsDevice&) = delete;

    bool initialize(Window& window, const GraphicsConfig& config = {});
    // Transitional presentation entry point for Application. Frame acquisition
    // and submission will replace it when RenderFrame is introduced.
    bool present();

private:
    std::unique_ptr<detail::GraphicsBackend> backend_;
};

} // namespace frostwind
