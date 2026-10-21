#pragma once

#include <vector>

#include <core/Types.h>

namespace frostwind {

class VertexBuffer;
class IndexBuffer;

struct VertexAttribute {
    u32 location;
    int components; // 1..4 floats per vertex attribute.
    usize offset;   // Bytes from the beginning of each vertex.
};

struct VertexLayout {
    usize stride = 0;
    std::vector<VertexAttribute> attributes;
};

// Owns a VAO describing one interleaved vertex buffer and optional u32 indices.
// Borrows buffers: they and the creating context must outlive this object.
// Operations require that context to be current and throw std::runtime_error
// on failure. Destruction never throws. No native handles are exposed.
class VertexArray {
public:
    VertexArray() = default;
    ~VertexArray() noexcept;
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    // Buffers must already be initialized in the current context. Consumes the
    // layout before returning; preserves VAO/array-buffer bindings. One-time
    // after success; partial failure releases the VAO and permits retry.
    void initialize(const VertexBuffer& vertices, const VertexLayout& layout);
    void initialize(const VertexBuffer& vertices, const IndexBuffer& indices, const VertexLayout& layout);
    void bind() const;
    usize vertexCount() const { return vertexCount_; }
    usize indexCount() const { return indexCount_; }
    // Optional borrowed owner; null for a non-indexed vertex array.
    const IndexBuffer* indexBuffer() const { return indices_; }

private:
    void initialize(const VertexBuffer& vertices, const IndexBuffer* indices, const VertexLayout& layout);
    u32 array_ = 0;
    usize vertexCount_ = 0;
    usize indexCount_ = 0;
    const IndexBuffer* indices_ = nullptr;
};

} // namespace frostwind
