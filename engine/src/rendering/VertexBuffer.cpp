#include "VertexBuffer.h"

#include <limits>
#include <stdexcept>
#include <utility>

#include <glad/gl.h>

#include "GraphicsChecks.h"

namespace frostwind {

VertexBuffer::~VertexBuffer() noexcept
{
    if (buffer_) {
        glDeleteBuffers(1, &buffer_);
    }
}

void VertexBuffer::initialize(const void* data, usize byteCount)
{
    detail::requireOpenGL("[VertexBuffer] Vertex buffer initialization");
    if (buffer_ || !data || !byteCount || byteCount > usize(std::numeric_limits<GLsizeiptr>::max())) {
        throw std::runtime_error("[VertexBuffer] Vertex buffer initialization requires an empty owner and valid vertex bytes.");
    }
    VertexBuffer candidate;
    glGenBuffers(1, &candidate.buffer_);
    if (!candidate.buffer_) {
        throw std::runtime_error("[VertexBuffer] Could not create OpenGL vertex storage.");
    }
    GLint previousBuffer = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, candidate.buffer_);
    glBufferData(GL_ARRAY_BUFFER, byteCount, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, previousBuffer);
    detail::checkOpenGL("[VertexBuffer] Vertex buffer upload");
    std::swap(buffer_, candidate.buffer_);
    byteCount_ = byteCount;
}

void VertexBuffer::bind() const
{
    detail::requireOpenGL("[VertexBuffer] Vertex buffer binding");
    if (!buffer_) {
        throw std::runtime_error("[VertexBuffer] VertexBuffer must be initialized before use.");
    }
    glBindBuffer(GL_ARRAY_BUFFER, buffer_);
    detail::checkOpenGL("[VertexBuffer] Vertex buffer binding");
}

} // namespace frostwind
