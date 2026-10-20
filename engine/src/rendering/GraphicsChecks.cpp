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

void requireContext(SDL_GLContextState* owner, const char* operation)
{
    requireOpenGL(operation);
    if (!owner || SDL_GL_GetCurrentContext() != owner) {
        throw std::runtime_error(std::string(operation) + " requires the resource's creating context to be current.");
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

bool canDestroy(SDL_GLContextState* owner, const char* operation) noexcept
{
    if (!owner || SDL_GL_GetCurrentContext() != owner || !GLAD_GL_VERSION_3_3 || !glad_glGetError) {
        SDL_Log("%s skipped: the resource's creating context must be current with loaded GLAD.", operation);
        return false;
    }
    return true;
}

} // namespace frostwind::detail
