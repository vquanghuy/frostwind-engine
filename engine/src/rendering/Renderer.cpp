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

void Renderer::clearDepth() const
{
    detail::requireOpenGL("Depth clear");
    glDisable(GL_SCISSOR_TEST);
    glDepthMask(GL_TRUE);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
    detail::checkOpenGL("Depth clear");
}

void Renderer::setDepthTest(bool enabled) const
{
    detail::requireOpenGL("Depth test");
    if (enabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    detail::checkOpenGL("Depth test");
}

void Renderer::setBackFaceCulling(bool enabled) const
{
    detail::requireOpenGL("Back-face culling");
    if (enabled) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    detail::checkOpenGL("Back-face culling");
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
