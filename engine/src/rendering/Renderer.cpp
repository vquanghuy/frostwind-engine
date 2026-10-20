#include "Renderer.h"

#include <cmath>
#include <limits>
#include <stdexcept>

#include <glad/gl.h>

#include "GraphicsChecks.h"
#include "IndexBuffer.h"
#include "VertexArray.h"

namespace frostwind {
namespace {

struct VertexArrayBinding {
    GLint previous = 0;
    VertexArrayBinding()
    {
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous);
    }
    ~VertexArrayBinding()
    {
        glBindVertexArray(previous);
    }
};

} // namespace

void Renderer::clear(Color color) const
{
    detail::requireOpenGL("Color clear");
    if (!std::isfinite(color.r) || !std::isfinite(color.g) || !std::isfinite(color.b) || !std::isfinite(color.a)) {
        throw std::runtime_error("Color clear requires finite RGBA components.");
    }
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
    detail::checkOpenGL("Color clear");
}

void Renderer::drawTriangles(const VertexArray& vertexArray) const
{
    detail::requireOpenGL("Triangle draw");
    const auto* indices = vertexArray.indexBuffer();
    const usize count = indices ? vertexArray.indexCount() : vertexArray.vertexCount();
    if (!count || count % 3 || count > usize(std::numeric_limits<GLsizei>::max())) {
        throw std::runtime_error("Triangle draw requires a nonempty complete triangle list within the GL count limit.");
    }
    if (indices && indices->maxIndex() >= vertexArray.vertexCount()) {
        throw std::runtime_error("Triangle draw references a vertex outside the vertex buffer.");
    }
    GLint program = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    if (!program) {
        throw std::runtime_error("Activate a shader program before drawing vertices.");
    }
    {
        const VertexArrayBinding restore;
        vertexArray.bind();
        const GLsizei drawCount = count;
        if (indices) {
            glDrawElements(GL_TRIANGLES, drawCount, GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, drawCount);
        }
    }
    detail::checkOpenGL(indices ? "Indexed triangle draw" : "Triangle draw");
}

} // namespace frostwind
