#include "VertexArray.h"

#include <limits>
#include <stdexcept>
#include <utility>

#include <glad/gl.h>

#include "GraphicsChecks.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

namespace frostwind {
namespace {

struct VertexBindings {
    GLint array = 0;
    GLint buffer = 0;
    VertexBindings()
    {
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &array);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &buffer);
    }
    ~VertexBindings()
    {
        glBindVertexArray(array);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
    }
};

} // namespace

VertexArray::~VertexArray() noexcept
{
    if (array_) {
        glDeleteVertexArrays(1, &array_);
    }
}

void VertexArray::initialize(const VertexBuffer& vertices, const VertexLayout& layout)
{
    initialize(vertices, nullptr, layout);
}

void VertexArray::initialize(const VertexBuffer& vertices, const IndexBuffer& indices, const VertexLayout& layout)
{
    initialize(vertices, &indices, layout);
}

void VertexArray::initialize(const VertexBuffer& vertices, const IndexBuffer* indices, const VertexLayout& layout)
{
    detail::requireOpenGL("[VertexArray] Vertex array initialization");
    if (array_ || !vertices.byteCount() || !layout.stride || vertices.byteCount() % layout.stride ||
        layout.stride > usize(std::numeric_limits<GLsizei>::max()) || layout.attributes.empty()) {
        throw std::runtime_error("[VertexArray] Vertex array initialization requires an empty owner, initialized vertices, and a valid layout.");
    }
    if (indices && !indices->indexCount()) {
        throw std::runtime_error("[VertexArray] Vertex array initialization requires an initialized IndexBuffer when supplied.");
    }
    GLint maxAttributes = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttributes);
    detail::checkOpenGL("[VertexArray] Vertex attribute limit query");
    if (maxAttributes <= 0) {
        throw std::runtime_error("[VertexArray] Vertex attribute limit must be positive.");
    }
    for (auto it = layout.attributes.begin(); it != layout.attributes.end(); ++it) {
        if (it->components < 1 || it->components > 4 || it->location >= u32(maxAttributes) ||
            it->offset > layout.stride || usize(it->components) * sizeof(f32) > layout.stride - it->offset) {
            throw std::runtime_error("[VertexArray] Vertex attribute exceeds the stride or current context's limits.");
        }
        for (auto previous = layout.attributes.begin(); previous != it; ++previous) {
            if (previous->location == it->location) {
                throw std::runtime_error("[VertexArray] Vertex attribute locations must be unique.");
            }
        }
    }
    VertexArray candidate;
    glGenVertexArrays(1, &candidate.array_);
    if (!candidate.array_) {
        throw std::runtime_error("[VertexArray] Could not create OpenGL vertex array.");
    }
    {
        const VertexBindings restore;
        glBindVertexArray(candidate.array_);
        vertices.bind();
        for (const auto& attribute : layout.attributes) {
            glVertexAttribPointer(attribute.location, attribute.components, GL_FLOAT, GL_FALSE,
                layout.stride, reinterpret_cast<const void*>(attribute.offset));
            glEnableVertexAttribArray(attribute.location);
        }
        detail::checkOpenGL("[VertexArray] Vertex attribute layout");
        if (indices) {
            indices->bind();
        }
    }
    detail::checkOpenGL("[VertexArray] Vertex array initialization");
    std::swap(array_, candidate.array_);
    vertexCount_ = vertices.byteCount() / layout.stride;
    indexCount_ = indices ? indices->indexCount() : 0;
    indices_ = indices;
}

void VertexArray::bind() const
{
    detail::requireOpenGL("[VertexArray] Vertex array binding");
    if (!array_) {
        throw std::runtime_error("[VertexArray] VertexArray must be initialized before use.");
    }
    glBindVertexArray(array_);
    detail::checkOpenGL("[VertexArray] Vertex array binding");
}

} // namespace frostwind
