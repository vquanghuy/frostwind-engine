#pragma once

#include "GraphicsTypes.h"

namespace frostwind {

class Buffer;
class GraphicsDevice;
class GraphicsPipeline;
class Texture2D;

// A stack-scoped drawing session borrowing its device. No GPU resources are
// allocated by constructing this object. An unfinished frame is abandoned.
class RenderFrame {
public:
    RenderFrame() = default;
    ~RenderFrame();
    RenderFrame(const RenderFrame&) = delete;
    RenderFrame& operator=(const RenderFrame&) = delete;

    // Use the acquired drawable extent during an active callback.
    const FrameTarget& target() const { return target_; }
    // Finite RGBA components; clear the target before a group of draws.
    // Failure marks an active frame failed, preventing presentation.
    bool clear(Color color);
    // Complete bindings per draw; no state is inherited from a previous draw.
    // Optional texture and parameter bytes must match the pipeline description.
    // Parameter bytes are consumed/copied before return. All resources must
    // belong to this frame's device; count must be a nonzero multiple of three
    // within the buffer range. False prevents this active frame presenting.
    bool draw(const GraphicsPipeline& pipeline, const Buffer& vertices,
        u32 vertexCount, u32 firstVertex = 0, const Texture2D* texture = nullptr,
        const void* parameters = nullptr, usize parameterBytes = 0);

private:
    friend class GraphicsDevice;
    GraphicsDevice* device_ = nullptr;
    FrameTarget target_;
    bool used_ = false;
    bool failed_ = false;
};

} // namespace frostwind
