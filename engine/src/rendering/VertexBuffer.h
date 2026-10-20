#pragma once

#include <initializer_list>

#include <core/Types.h>

struct SDL_GLContextState;

namespace frostwind {

struct VertexAttribute {
    u32 location;
    int components; // 1..4 floats per vertex attribute.
    usize offset;   // Bytes from the beginning of each vertex.
};

// Owns an OpenGL VBO and its VAO layout. The creating context must be current
// for use/destruction and outlive this owner. Geometry is independent of shaders.
class VertexBuffer {
public:
    VertexBuffer() = default;
    ~VertexBuffer();
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    // Immutable interleaved float attributes; consumes bytes before returning.
    // One-time after success, retry after failure. Preserves VAO/VBO bindings.
    bool initialize(const void* data, usize byteCount, usize stride,
        std::initializer_list<VertexAttribute> attributes);
    // Selects this vertex layout. Requires the creating context to be current.
    bool bind() const;
    usize vertexCount() const { return vertexCount_; }

private:
    u32 buffer_ = 0;
    u32 array_ = 0;
    usize vertexCount_ = 0;
    SDL_GLContextState* context_ = nullptr;
};

} // namespace frostwind
