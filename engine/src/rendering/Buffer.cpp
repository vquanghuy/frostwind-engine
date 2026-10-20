#include "Buffer.h"

#include "GraphicsDevice.h"

namespace frostwind {

Buffer::~Buffer()
{
    if (device_) {
        device_->release(detail::ResourceKind::Buffer, handle_.value);
    }
}

bool Buffer::initialize(GraphicsDevice& device, const void* data, usize byteCount)
{
    return device.create(*this, data, byteCount);
}

} // namespace frostwind
