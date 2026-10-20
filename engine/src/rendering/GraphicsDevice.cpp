#include "GraphicsDevice.h"

#include <cmath>
#include <limits>
#include <utility>

#include <SDL3/SDL.h>

#include "Buffer.h"
#include "Shader.h"
#include "Texture2D.h"
#include "GraphicsPipeline.h"
#include "RenderFrame.h"
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
    if (!candidate || !candidate->initialize(window, config)) {
        SDL_Log("Could not initialize the compiled graphics backend.");
        return false;
    }
    limits_ = candidate->limits();
    backend_ = std::move(candidate);
    return true;
}

bool GraphicsDevice::ready() const
{
    if (!backend_) {
        SDL_Log("GraphicsDevice must be initialized before use.");
        return false;
    }
    return true;
}

bool GraphicsDevice::create(Buffer& buffer, const void* data, usize byteCount)
{
    if (!ready()) {
        return false;
    }
    if (buffer.device_ || !data || byteCount == 0 || byteCount > limits_.maxBufferSize) {
        SDL_Log("Buffer initialization requires an empty owner and valid vertex bytes within device limits.");
        return false;
    }
    if (!backend_->createBuffer(data, byteCount, buffer.handle_)) {
        return false;
    }
    buffer.device_ = this;
    return true;
}

bool GraphicsDevice::create(Shader& shader, const ShaderDescription& description, std::string_view source)
{
    if (!ready()) {
        return false;
    }
    if (shader.device_ || source.empty() || source.size() > usize(std::numeric_limits<i32>::max())) {
        SDL_Log("Shader initialization requires an empty owner and nonempty source within size limits.");
        return false;
    }
    if (!backend_->createShader(description, source, shader.handle_)) {
        return false;
    }
    shader.device_ = this;
    return true;
}

bool GraphicsDevice::create(Texture2D& texture, int width, int height, const byte* pixels, usize byteCount)
{
    if (!ready()) {
        return false;
    }
    if (texture.device_ || !pixels || width <= 0 || height <= 0 ||
        width > limits_.maxTextureSize || height > limits_.maxTextureSize) {
        SDL_Log("Texture initialization requires an empty owner, RGBA8 pixels, and dimensions within device limits.");
        return false;
    }
    const usize w = width;
    const usize h = height;
    if (w > std::numeric_limits<usize>::max() / 4 / h || byteCount != w * h * 4) {
        SDL_Log("Texture byte count does not match its RGBA8 dimensions.");
        return false;
    }
    if (!backend_->createTexture(width, height, pixels, texture.handle_)) {
        return false;
    }
    texture.device_ = this;
    return true;
}

bool GraphicsDevice::create(GraphicsPipeline& pipeline, const PipelineDescription& description, const Shader& vertex, const Shader& fragment)
{
    if (!ready()) {
        return false;
    }
    if (pipeline.device_ || vertex.device_ != this || fragment.device_ != this) {
        SDL_Log("Pipeline initialization requires an empty owner and shader stages from this device.");
        return false;
    }
    if (description.targetFormat != ColorFormat::RGBA8 || description.topology != PrimitiveTopology::TriangleList ||
        description.blend != BlendMode::Opaque || description.depth != DepthMode::Disabled || description.cull != CullMode::None) {
        SDL_Log("Unsupported pipeline target, topology, or drawing state.");
        return false;
    }
    if (description.vertexStride == 0 || description.vertexStride > usize(std::numeric_limits<i32>::max()) ||
        description.attributes.empty() || description.attributes.size() > limits_.maxVertexAttributes) {
        SDL_Log("Pipeline vertex layout is empty or exceeds device limits.");
        return false;
    }
    for (usize i = 0; i < description.attributes.size(); ++i) {
        const auto& attribute = description.attributes[i];
        if (attribute.format != VertexFormat::Float2 || attribute.location >= limits_.maxVertexAttributes ||
            attribute.offset > description.vertexStride || description.vertexStride - attribute.offset < 2 * sizeof(f32)) {
            SDL_Log("Pipeline vertex attribute has an invalid format, location, or offset.");
            return false;
        }
        for (usize j = 0; j < i; ++j) {
            if (description.attributes[j].location == attribute.location) {
                SDL_Log("Pipeline vertex attribute locations must be unique.");
                return false;
            }
        }
    }
    if (description.parameters.name.empty() != (description.parameters.byteSize == 0) ||
        description.parameters.byteSize % 16 != 0 || description.parameters.byteSize > limits_.maxParameterBytes) {
        SDL_Log("Pipeline parameter block needs a name and a matching 16-byte-aligned size within device limits.");
        return false;
    }
    if (!backend_->createPipeline(description, vertex.handle_, fragment.handle_, pipeline.handle_)) {
        return false;
    }
    pipeline.device_ = this;
    return true;
}

void GraphicsDevice::release(detail::ResourceKind kind, u64 identity) noexcept
{
    backend_->release(kind, identity);
}

FrameStatus GraphicsDevice::beginFrame(RenderFrame& frame)
{
    if (!ready()) {
        return FrameStatus::Failed;
    }
    if (activeFrame_ || frame.device_ || frame.used_) {
        SDL_Log("Cannot acquire a nested frame or reuse a completed RenderFrame.");
        return FrameStatus::Failed;
    }
    const auto status = backend_->beginFrame(frame.target_);
    if (status == FrameStatus::Ready) {
        frame.device_ = this;
        frame.used_ = true;
        activeFrame_ = &frame;
    }
    return status;
}

bool GraphicsDevice::active(RenderFrame& frame)
{
    if (activeFrame_ != &frame || frame.device_ != this || frame.failed_) {
        SDL_Log("RenderFrame is inactive, belongs to another device, or has already failed.");
        return false;
    }
    return true;
}

bool GraphicsDevice::clear(RenderFrame& frame, Color color)
{
    if (!active(frame)) {
        return false;
    }
    if (!std::isfinite(color.r) || !std::isfinite(color.g) || !std::isfinite(color.b) || !std::isfinite(color.a)) {
        SDL_Log("Clear color must be finite.");
        frame.failed_ = true;
        return false;
    }
    frame.failed_ = !backend_->clear(color);
    return !frame.failed_;
}

bool GraphicsDevice::draw(RenderFrame& frame, const GraphicsPipeline& pipeline, const Buffer& vertices,
    u32 vertexCount, u32 firstVertex, const Texture2D* texture, const void* parameters, usize parameterBytes)
{
    if (!active(frame)) {
        return false;
    }
    if (pipeline.device_ != this || vertices.device_ != this || (texture && texture->device_ != this) ||
        vertexCount == 0 || vertexCount % 3 != 0 || vertexCount > u32(std::numeric_limits<i32>::max()) ||
        firstVertex > u32(std::numeric_limits<i32>::max()) || (parameterBytes != 0 && !parameters)) {
        SDL_Log("Draw requires initialized resources from this device, valid triangle vertices, and valid parameter bytes.");
        frame.failed_ = true;
        return false;
    }
    frame.failed_ = !backend_->draw(pipeline.handle_, vertices.handle_, texture ? texture->handle_ : detail::TextureHandle{},
        parameters, parameterBytes, firstVertex, vertexCount);
    return !frame.failed_;
}

bool GraphicsDevice::submit(RenderFrame& frame)
{
    if (!active(frame)) {
        // A foreign frame must remain owned by its original device.
        if (activeFrame_ == &frame) {
            abandon(frame);
        }
        return false;
    }
    const bool success = backend_->submit();
    abandon(frame);
    return success;
}

void GraphicsDevice::abandon(RenderFrame& frame) noexcept
{
    if (activeFrame_ == &frame) {
        backend_->abandon();
        activeFrame_ = nullptr;
        frame.device_ = nullptr;
    }
}

} // namespace frostwind
