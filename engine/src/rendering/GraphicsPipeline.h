#pragma once

#include "GraphicsTypes.h"
#include "internal/ResourceHandle.h"

namespace frostwind {

class GraphicsDevice;
class Shader;

// Immutable drawing state. Shader owners may be destroyed after initialization;
// the pipeline retains the backend resources it needs. Device must outlive it.
class GraphicsPipeline {
public:
    GraphicsPipeline() = default;
    ~GraphicsPipeline();
    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    // Requires initialized vertex/fragment shaders from this device. Copies the
    // description and validates supported state, active attributes, and binding
    // layout. One-time after success; partial failure cleans up and permits retry.
    bool initialize(GraphicsDevice& device, const PipelineDescription& description,
        const Shader& vertex, const Shader& fragment);

private:
    friend class GraphicsDevice;
    GraphicsDevice* device_ = nullptr;
    detail::PipelineHandle handle_;
};

} // namespace frostwind
