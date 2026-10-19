#pragma once

namespace frostwind {
class Window;
struct GraphicsConfig;
}

namespace frostwind::detail {

// Private implementation contract. Add operations as examples migrate to the
// shared resource/frame interfaces; the first step only owns graphics lifetime.
class GraphicsBackend {
public:
    GraphicsBackend() = default;
    virtual ~GraphicsBackend() = default;
    GraphicsBackend(const GraphicsBackend&) = delete;
    GraphicsBackend& operator=(const GraphicsBackend&) = delete;

    virtual bool initialize(Window& window, const GraphicsConfig& config) = 0;
    virtual bool present() = 0;
};

} // namespace frostwind::detail
