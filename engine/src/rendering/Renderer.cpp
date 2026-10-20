#include "Renderer.h"

#include <cmath>
#include <limits>

#include <SDL3/SDL.h>
#include <glad/gl.h>

#include "OpenGLChecks.h"
#include "VertexBuffer.h"

namespace frostwind {

bool Renderer::clear(Color color) const
{
    if (!detail::requireOpenGL("Color clear") || !std::isfinite(color.r) ||
        !std::isfinite(color.g) || !std::isfinite(color.b) || !std::isfinite(color.a)) {
        return false;
    }
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
    return detail::checkOpenGL("Color clear");
}

bool Renderer::drawTriangles(const VertexBuffer& vertices, u32 vertexCount, u32 firstVertex) const
{
    const usize availableVertices = vertices.vertexCount();
    if (!vertexCount || vertexCount % 3 || firstVertex > availableVertices ||
        vertexCount > availableVertices - firstVertex || vertexCount > u32(std::numeric_limits<GLsizei>::max()) ||
        firstVertex > u32(std::numeric_limits<GLint>::max()) || !detail::requireOpenGL("Triangle draw")) {
        SDL_Log("Triangle draw requires initialized geometry and a valid triangle range.");
        return false;
    }
    GLint program = 0, previousArray = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    if (!program) {
        SDL_Log("Activate a shader program before drawing vertices.");
        return false;
    }
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousArray);
    const bool bound = vertices.bind();
    if (bound) {
        glDrawArrays(GL_TRIANGLES, firstVertex, vertexCount);
    }
    glBindVertexArray(previousArray);
    return detail::checkOpenGL("Triangle draw") && bound;
}

} // namespace frostwind
