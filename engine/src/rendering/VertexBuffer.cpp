#include "VertexBuffer.h"

#include <limits>
#include <stdexcept>
#include <utility>

#include <SDL3/SDL.h>
#include <glad/gl.h>

#include "GraphicsChecks.h"

namespace frostwind {

VertexBuffer::~VertexBuffer() noexcept
{
    if (buffer_ && detail::canDestroy(context_, "Vertex buffer destruction")) {
        glDeleteBuffers(1, &buffer_);
    }
}

void VertexBuffer::initialize(const void* data, usize byteCount)
{
    detail::requireOpenGL("Vertex buffer initialization");
    if (buffer_ || !data || !byteCount || byteCount > usize(std::numeric_limits<GLsizeiptr>::max())) {
        throw std::runtime_error("Vertex buffer initialization requires an empty owner and valid vertex bytes.");
    }
    VertexBuffer candidate;
    candidate.context_ = SDL_GL_GetCurrentContext();
    glGenBuffers(1, &candidate.buffer_);
    if (!candidate.buffer_) {
        throw std::runtime_error("Could not create OpenGL vertex storage.");
    }
    GLint previousBuffer = 0;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, candidate.buffer_);
    glBufferData(GL_ARRAY_BUFFER, byteCount, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, previousBuffer);
    detail::checkOpenGL("Vertex buffer upload");
    std::swap(buffer_, candidate.buffer_);
    std::swap(context_, candidate.context_);
    byteCount_ = byteCount;
}

void VertexBuffer::bind() const
{
    detail::requireContext(context_, "Vertex buffer binding");
    glBindBuffer(GL_ARRAY_BUFFER, buffer_);
    detail::checkOpenGL("Vertex buffer binding");
}

} // namespace frostwind
