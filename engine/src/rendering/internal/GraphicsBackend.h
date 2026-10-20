#pragma once

#include <string_view>

#include "../GraphicsTypes.h"
#include "ResourceHandle.h"

namespace frostwind {
class Window;
}

namespace frostwind::detail {

// One private native dispatch boundary. Descriptions and identities contain no
// graphics API types. Extend this contract only as examples exercise new work.
class GraphicsBackend {
public:
    virtual ~GraphicsBackend() = default;
    virtual bool initialize(Window& window, const GraphicsConfig& config) = 0;
    virtual GraphicsLimits limits() const = 0;
    virtual bool createBuffer(const void* data, usize byteCount, BufferHandle& result) = 0;
    virtual bool createTexture(int width, int height, const byte* pixels, TextureHandle& result) = 0;
    virtual bool createShader(const ShaderDescription& description, std::string_view source, ShaderHandle& result) = 0;
    virtual bool createPipeline(const PipelineDescription& description, ShaderHandle vertex, ShaderHandle fragment, PipelineHandle& result) = 0;
    virtual void release(ResourceKind kind, u64 identity) noexcept = 0;
    virtual FrameStatus beginFrame(FrameTarget& target) = 0;
    virtual bool clear(Color color) = 0;
    virtual bool draw(PipelineHandle pipeline, BufferHandle vertices, TextureHandle texture,
        const void* parameters, usize parameterBytes, u32 firstVertex, u32 vertexCount) = 0;
    virtual bool submit() = 0;
    virtual void abandon() noexcept = 0;
};

} // namespace frostwind::detail
