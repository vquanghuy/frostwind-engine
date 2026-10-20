#pragma once

#include <core/Types.h>

namespace frostwind::detail {

enum class ResourceKind { Buffer, Texture, Shader, Pipeline };

// Identities are never reused during a device's lifetime. Zero is invalid.
template<ResourceKind Kind>
struct ResourceHandle {
    u64 value = 0;
};

using BufferHandle = ResourceHandle<ResourceKind::Buffer>;
using TextureHandle = ResourceHandle<ResourceKind::Texture>;
using ShaderHandle = ResourceHandle<ResourceKind::Shader>;
using PipelineHandle = ResourceHandle<ResourceKind::Pipeline>;

} // namespace frostwind::detail
