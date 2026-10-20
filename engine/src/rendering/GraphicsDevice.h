#pragma once

#include <memory>
#include <string_view>

#include "GraphicsTypes.h"
#include "internal/ResourceHandle.h"

namespace frostwind {

class Window;
class Buffer;
class Shader;
class Texture2D;
class GraphicsPipeline;
class RenderFrame;
namespace detail {
class GraphicsBackend;
}

// Owns the compiled graphics backend and borrows Window. Resource owners and
// frames must die before this device, and the device before its window.
class GraphicsDevice {
public:
    GraphicsDevice();
    ~GraphicsDevice();
    GraphicsDevice(const GraphicsDevice&) = delete;
    GraphicsDevice& operator=(const GraphicsDevice&) = delete;

    // Requires an initialized window. One-time after success; failure cleans up
    // the candidate backend and permits retry. Does not take window ownership.
    bool initialize(Window& window, const GraphicsConfig& config = {});
    // Required operation limits; zero values before successful initialization.
    const GraphicsLimits& limits() const { return limits_; }
    // Lower-level frame control; Application normally coordinates these calls.
    // Requires no other active frame and an unused frame object. Unavailable
    // leaves it inactive; Ready borrows this device until submit/scope cleanup.
    FrameStatus beginFrame(RenderFrame& frame);
    // Completes this device's active frame once. Failed commands or presentation
    // return false and abandon it; a foreign frame stays with its owning device.
    bool submit(RenderFrame& frame);

private:
    friend class Buffer;
    friend class Shader;
    friend class Texture2D;
    friend class GraphicsPipeline;
    friend class RenderFrame;

    bool create(Buffer& buffer, const void* data, usize byteCount);
    bool create(Shader& shader, const ShaderDescription& description, std::string_view source);
    bool create(Texture2D& texture, int width, int height, const byte* pixels, usize byteCount);
    bool create(GraphicsPipeline& pipeline, const PipelineDescription& description, const Shader& vertex, const Shader& fragment);
    void release(detail::ResourceKind kind, u64 identity) noexcept;
    void abandon(RenderFrame& frame) noexcept;
    bool clear(RenderFrame& frame, Color color);
    bool draw(RenderFrame& frame, const GraphicsPipeline& pipeline, const Buffer& vertices,
        u32 vertexCount, u32 firstVertex, const Texture2D* texture, const void* parameters, usize parameterBytes);
    bool ready() const;
    bool active(RenderFrame& frame);

    std::unique_ptr<detail::GraphicsBackend> backend_;
    GraphicsLimits limits_;
    RenderFrame* activeFrame_ = nullptr;
};

} // namespace frostwind
