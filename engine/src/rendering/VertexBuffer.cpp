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
    if ((array_ || buffer_) && detail::canDestroy(context_, "Vertex buffer destruction")) {
        if (array_) {
            glDeleteVertexArrays(1, &array_);
        }
        if (buffer_) {
            glDeleteBuffers(1, &buffer_);
        }
    }
}

void VertexBuffer::initialize(const void* data, usize byteCount, usize stride,
    std::initializer_list<VertexAttribute> attributes)
{
    detail::requireOpenGL("Vertex buffer initialization");
    if (buffer_ || !data || !stride || !byteCount || byteCount % stride || attributes.size() == 0 ||
        stride > usize(std::numeric_limits<GLsizei>::max()) ||
        byteCount > usize(std::numeric_limits<GLsizeiptr>::max())) {
        throw std::runtime_error("Invalid vertex buffer data, size, or layout.");
    }
    GLint maxAttributes = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttributes);
    detail::checkOpenGL("Vertex attribute limit query");
    if (maxAttributes <= 0) {
        throw std::runtime_error("Vertex attribute limit must be positive.");
    }
    for (auto it = attributes.begin(); it != attributes.end(); ++it) {
        if (it->components < 1 || it->components > 4 || it->location >= u32(maxAttributes) ||
            it->offset > stride || usize(it->components) * sizeof(f32) > stride - it->offset) {
            throw std::runtime_error("Vertex attribute exceeds the stride or current context's limits.");
        }
        for (auto previous = attributes.begin(); previous != it; ++previous) {
            if (previous->location == it->location) {
                throw std::runtime_error("Vertex attribute locations must be unique.");
            }
        }
    }
    VertexBuffer candidate;
    candidate.context_ = SDL_GL_GetCurrentContext();
    glGenVertexArrays(1, &candidate.array_);
    glGenBuffers(1, &candidate.buffer_);
    if (!candidate.array_ || !candidate.buffer_) {
        throw std::runtime_error("Could not create OpenGL vertex storage/layout.");
    }
    GLint previousArray = 0, previousBuffer = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousArray);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousBuffer);
    glBindVertexArray(candidate.array_);
    glBindBuffer(GL_ARRAY_BUFFER, candidate.buffer_);
    glBufferData(GL_ARRAY_BUFFER, byteCount, data, GL_STATIC_DRAW);
    for (const auto& attribute : attributes) {
        glVertexAttribPointer(attribute.location, attribute.components, GL_FLOAT, GL_FALSE,
            stride, reinterpret_cast<const void*>(attribute.offset));
        glEnableVertexAttribArray(attribute.location);
    }
    glBindVertexArray(previousArray);
    glBindBuffer(GL_ARRAY_BUFFER, previousBuffer);
    detail::checkOpenGL("Vertex buffer initialization");
    std::swap(buffer_, candidate.buffer_);
    std::swap(array_, candidate.array_);
    std::swap(context_, candidate.context_);
    vertexCount_ = byteCount / stride;
}

void VertexBuffer::bind() const
{
    detail::requireContext(context_, "Vertex buffer binding");
    glBindVertexArray(array_);
    detail::checkOpenGL("Vertex buffer binding");
}

} // namespace frostwind
