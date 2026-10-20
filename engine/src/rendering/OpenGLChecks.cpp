#include "OpenGLChecks.h"

#include <SDL3/SDL.h>
#include <glad/gl.h>

namespace frostwind::detail {

bool requireOpenGL(const char* operation)
{
    if (!SDL_GL_GetCurrentContext() || !GLAD_GL_VERSION_3_3 || !glad_glGetError) {
        SDL_Log("%s requires a current OpenGL 3.3 context and loaded GLAD.", operation);
        return false;
    }
    return true;
}

bool requireContext(SDL_GLContextState* owner, const char* operation)
{
    if (!owner || !requireOpenGL(operation) || SDL_GL_GetCurrentContext() != owner) {
        SDL_Log("%s requires the resource's creating context to be current.", operation);
        return false;
    }
    return true;
}

bool checkOpenGL(const char* operation)
{
    bool success = true;
    for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError()) {
        SDL_Log("%s failed: OpenGL error 0x%x", operation, error);
        success = false;
    }
    return success;
}

} // namespace frostwind::detail
