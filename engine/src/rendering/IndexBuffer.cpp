#include "IndexBuffer.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

#include <SDL3/SDL.h>
#include <glad/gl.h>

#include "GraphicsChecks.h"

namespace frostwind {

IndexBuffer::~IndexBuffer() noexcept
{
    if (buffer_ && detail::canDestroy(context_, "Index buffer destruction")) {
        glDeleteBuffers(1, &buffer_);
    }
}

void IndexBuffer::initialize(const u32* indices, usize indexCount)
{
    detail::requireOpenGL("Index buffer initialization");
    if (buffer_ || !indices || !indexCount ||
        indexCount > usize(std::numeric_limits<GLsizeiptr>::max()) / sizeof(u32)) {
        throw std::runtime_error("Index buffer initialization requires an empty owner and a valid u32 index array.");
    }
    IndexBuffer candidate;
    candidate.maxIndex_ = *std::max_element(indices, indices + indexCount);
    candidate.context_ = SDL_GL_GetCurrentContext();
    glGenBuffers(1, &candidate.buffer_);
    if (!candidate.buffer_) {
        throw std::runtime_error("Could not create OpenGL index storage.");
    }
    // An element-buffer binding belongs to a VAO. Upload through a neutral
    // buffer target so initialization also works with VAO 0 bound.
    GLint previousBuffer = 0;
    glGetIntegerv(GL_COPY_WRITE_BUFFER, &previousBuffer);
    glBindBuffer(GL_COPY_WRITE_BUFFER, candidate.buffer_);
    glBufferData(GL_COPY_WRITE_BUFFER, indexCount * sizeof(u32), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_COPY_WRITE_BUFFER, previousBuffer);
    detail::checkOpenGL("Index buffer upload");
    std::swap(buffer_, candidate.buffer_);
    std::swap(context_, candidate.context_);
    indexCount_ = indexCount;
    std::swap(maxIndex_, candidate.maxIndex_);
}

void IndexBuffer::bind() const
{
    detail::requireContext(context_, "Index buffer binding");
    GLint array = 0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &array);
    if (!array) {
        throw std::runtime_error("Index buffer binding requires an active vertex array.");
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_);
    detail::checkOpenGL("Index buffer binding");
}

u32 IndexBuffer::maxIndex() const
{
    if (!indexCount_) {
        throw std::runtime_error("IndexBuffer must be initialized before querying its maximum index.");
    }
    return maxIndex_;
}

} // namespace frostwind
