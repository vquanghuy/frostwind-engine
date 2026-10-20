#include "VertexBuffer.h"

#include <limits>
#include <utility>

#include <SDL3/SDL.h>
#include <glad/gl.h>

#include "OpenGLChecks.h"

namespace frostwind {

VertexBuffer::~VertexBuffer()
{
    if ((array_ || buffer_) && detail::requireContext(context_, "Vertex buffer destruction")) {
        if (array_) {
            glDeleteVertexArrays(1, &array_);
        }
        if (buffer_) {
            glDeleteBuffers(1, &buffer_);
        }
    }
}

bool VertexBuffer::initialize(const void* data, usize byteCount, usize stride,
    std::initializer_list<VertexAttribute> attributes)
{
    if (buffer_ || !data || !stride || !byteCount || byteCount % stride || attributes.size() == 0 ||
        stride > usize(std::numeric_limits<GLsizei>::max()) ||
        byteCount > usize(std::numeric_limits<GLsizeiptr>::max()) ||
        !detail::requireOpenGL("Vertex buffer initialization")) {
        SDL_Log("Invalid vertex buffer data, size, or layout.");
        return false;
    }
    GLint maxAttributes = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttributes);
    if (!detail::checkOpenGL("Vertex attribute limit query") || maxAttributes <= 0) {
        return false;
    }
    for (auto it = attributes.begin(); it != attributes.end(); ++it) {
        if (it->components < 1 || it->components > 4 || it->location >= u32(maxAttributes) ||
            it->offset > stride || usize(it->components) * sizeof(f32) > stride - it->offset) {
            SDL_Log("Vertex attribute exceeds the stride or current context's limits.");
            return false;
        }
        for (auto previous = attributes.begin(); previous != it; ++previous) {
            if (previous->location == it->location) {
                SDL_Log("Vertex attribute locations must be unique.");
                return false;
            }
        }
    }
    VertexBuffer candidate;
    candidate.context_ = SDL_GL_GetCurrentContext();
    glGenVertexArrays(1, &candidate.array_);
    glGenBuffers(1, &candidate.buffer_);
    if (!candidate.array_ || !candidate.buffer_) {
        SDL_Log("Could not create OpenGL vertex storage/layout.");
        return false;
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
    if (!detail::checkOpenGL("Vertex buffer initialization")) {
        return false;
    }
    std::swap(buffer_, candidate.buffer_);
    std::swap(array_, candidate.array_);
    std::swap(context_, candidate.context_);
    vertexCount_ = byteCount / stride;
    return true;
}

bool VertexBuffer::bind() const
{
    if (!array_ || !detail::requireContext(context_, "Vertex buffer binding")) {
        return false;
    }
    glBindVertexArray(array_);
    return detail::checkOpenGL("Vertex buffer binding");
}

} // namespace frostwind
