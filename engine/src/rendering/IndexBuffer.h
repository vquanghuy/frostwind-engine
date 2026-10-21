#pragma once

#include <core/Types.h>

namespace frostwind {

// Owns immutable u32 indices. The creating context must be current for GPU
// operations/destruction and outlive this owner. Operations throw on failure;
// destruction never throws. No native handles or draw commands are exposed.
class IndexBuffer {
public:
    IndexBuffer() = default;
    ~IndexBuffer() noexcept;
    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;

    // Consumes indexCount elements, not bytes. One-time after success; retry
    // after failure. Upload preserves buffer bindings and all VAO state.
    void initialize(const u32* indices, usize indexCount);
    // Attaches to the current nonzero VAO during VertexArray initialization.
    void bind() const;
    usize indexCount() const { return indexCount_; }
    // Cached whole-buffer maximum; requires successful initialization.
    u32 maxIndex() const;

private:
    u32 buffer_ = 0;
    usize indexCount_ = 0;
    u32 maxIndex_ = 0;
};

} // namespace frostwind
