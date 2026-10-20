#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "../../internal/GraphicsBackend.h"
#include "../../../platform/opengl/OpenGLWindowSurface.h"
#include "OpenGLResources.h"

namespace frostwind::detail {

class OpenGLBackend final : public GraphicsBackend {
public:
    ~OpenGLBackend() override;
    bool initialize(Window& window, const GraphicsConfig& config) override;
    GraphicsLimits limits() const override { return limits_; }
    bool createBuffer(const void* data, usize byteCount, BufferHandle& result) override;
    bool createTexture(int width, int height, const byte* pixels, TextureHandle& result) override;
    bool createShader(const ShaderDescription& description, std::string_view source, ShaderHandle& result) override;
    bool createPipeline(const PipelineDescription& description, ShaderHandle vertex, ShaderHandle fragment, PipelineHandle& result) override;
    void release(ResourceKind kind, u64 identity) noexcept override;
    FrameStatus beginFrame(FrameTarget& target) override;
    bool clear(Color color) override;
    bool draw(PipelineHandle pipeline, BufferHandle vertices, TextureHandle texture,
        const void* parameters, usize parameterBytes, u32 firstVertex, u32 vertexCount) override;
    bool submit() override;
    void abandon() noexcept override;

private:
    bool makeCurrent() const;
    u64 nextIdentity();
    OpenGLWindowSurface surface_;
    Window* window_ = nullptr;
    bool loaded_ = false;
    bool frameActive_ = false;
    GraphicsLimits limits_;
    u64 lastIdentity_ = 0;
    std::unordered_map<u64, std::shared_ptr<OpenGLBuffer>> buffers_;
    std::unordered_map<u64, std::shared_ptr<OpenGLTexture>> textures_;
    std::unordered_map<u64, std::shared_ptr<OpenGLShader>> shaders_;
    std::unordered_map<u64, std::shared_ptr<OpenGLPipeline>> pipelines_;
    std::vector<std::shared_ptr<void>> frameReferences_;
};

} // namespace frostwind::detail
