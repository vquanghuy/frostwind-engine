#pragma once

#include <core/Types.h>

struct SDL_GLContextState;

namespace frostwind {

// Owns immutable vertex bytes. The creating context must be current for GPU
// operations/destruction and outlive this owner and any borrowing VertexArray.
// Initialization/use throw std::runtime_error; destruction never throws.
class VertexBuffer {
public:
    VertexBuffer() = default;
    ~VertexBuffer() noexcept;
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    // Consumes bytes before returning. One-time after success; retry after
    // failure. Preserves the previous array-buffer binding and all VAO state.
    void initialize(const void* data, usize byteCount);
    // Selects the array buffer; VertexArray uses this to configure attributes.
    void bind() const;
    usize byteCount() const { return byteCount_; }

private:
    u32 buffer_ = 0;
    usize byteCount_ = 0;
    SDL_GLContextState* context_ = nullptr;
};

} // namespace frostwind
