#include "GraphicsPipeline.h"

#include "GraphicsDevice.h"

namespace frostwind {

GraphicsPipeline::~GraphicsPipeline()
{
    if (device_) {
        device_->release(detail::ResourceKind::Pipeline, handle_.value);
    }
}

bool GraphicsPipeline::initialize(GraphicsDevice& device, const PipelineDescription& description, const Shader& vertex, const Shader& fragment)
{
    return device.create(*this, description, vertex, fragment);
}

} // namespace frostwind
