#pragma once

#include <core/Types.h>

#include "internal/ResourceHandle.h"

namespace frostwind {

class GraphicsDevice;

// Immutable vertex bytes. The creating device must outlive this owner.
class Buffer {
public:
    Buffer() = default;
    ~Buffer();
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    // Requires an initialized device, nonempty data, and a nonzero byte count
    // within its limit. Copies/consumes caller bytes before returning. One-time
    // after success; failure permits retry on this owner.
    bool initialize(GraphicsDevice& device, const void* data, usize byteCount);

private:
    friend class GraphicsDevice;
    GraphicsDevice* device_ = nullptr;
    detail::BufferHandle handle_;
};

} // namespace frostwind
