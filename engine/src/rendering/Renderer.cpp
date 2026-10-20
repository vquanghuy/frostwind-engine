#include "Renderer.h"

#include <cmath>
#include <limits>
#include <stdexcept>

#include <glad/gl.h>

#include "GraphicsChecks.h"
#include "VertexBuffer.h"

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

void Renderer::drawTriangles(const VertexBuffer& vertices, u32 vertexCount, u32 firstVertex) const
{
    detail::requireOpenGL("Triangle draw");
    const usize availableVertices = vertices.vertexCount();
    if (!vertexCount || vertexCount % 3 || firstVertex > availableVertices ||
        vertexCount > availableVertices - firstVertex || vertexCount > u32(std::numeric_limits<GLsizei>::max()) ||
        firstVertex > u32(std::numeric_limits<GLint>::max())) {
        throw std::runtime_error("Triangle draw requires initialized geometry and a valid triangle range.");
    }
    GLint program = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    if (!program) {
        throw std::runtime_error("Activate a shader program before drawing vertices.");
    }
    {
        const VertexArrayBinding restore;
        vertices.bind();
        glDrawArrays(GL_TRIANGLES, firstVertex, vertexCount);
    }
    detail::checkOpenGL("Triangle draw");
}

} // namespace frostwind
