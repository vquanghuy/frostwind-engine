#include "GraphicsChecks.h"

#include <sstream>
#include <stdexcept>
#include <string>

#include <SDL3/SDL.h>
#include <glad/gl.h>

namespace frostwind::detail {

void requireOpenGL(const char* operation)
{
    if (!SDL_GL_GetCurrentContext() || !GLAD_GL_VERSION_3_3 || !glad_glGetError) {
        throw std::runtime_error(std::string(operation) + " requires a current OpenGL 3.3 context and loaded GLAD.");
    }
}

void checkOpenGL(const char* operation)
{
    const GLenum first = glGetError();
    if (first == GL_NO_ERROR) {
        return;
    }
    std::ostringstream message;
    message << operation << " failed: OpenGL error 0x" << std::hex << first;
    for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError()) {
        message << ", 0x" << error;
    }
    throw std::runtime_error(message.str());
}

} // namespace frostwind::detail
