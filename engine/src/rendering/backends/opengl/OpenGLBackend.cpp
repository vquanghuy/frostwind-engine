#include "OpenGLBackend.h"

#include <memory>

#include <SDL3/SDL.h>
#include <glad/gl.h>

#include "../../GraphicsDevice.h"
#include "../../Texture2D.h"
#include "../../internal/GraphicsBackendFactory.h"

namespace frostwind::detail {

std::unique_ptr<GraphicsBackend> createGraphicsBackend()
{
    return std::make_unique<OpenGLBackend>();
}

bool OpenGLBackend::initialize(Window& window, const GraphicsConfig& config)
{
    if (!surface_.initialize(window)) {
        return false;
    }
    const int version = gladLoadGL(SDL_GL_GetProcAddress);
    if (version == 0 || !GLAD_GL_VERSION_3_3) {
        SDL_Log("GLAD could not load the required OpenGL 3.3 Core functions.");
        return false;
    }
    SDL_Log("OpenGL target 3.3 Core; actual context %s; renderer: %s; detected %d.%d",
        reinterpret_cast<const char*>(glGetString(GL_VERSION)),
        reinterpret_cast<const char*>(glGetString(GL_RENDERER)),
        GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    if (!surface_.setVsync(config.vsync)) {
        SDL_Log("Could not set the requested swap interval: %s. Using application-configured frame pacing.", SDL_GetError());
    }
    // Texture resources remain OpenGL-specific until their later migration.
    return Texture2D::initializeSupport();
}

bool OpenGLBackend::present()
{
    if (!surface_.present()) {
        SDL_Log("OpenGL presentation failed: %s", SDL_GetError());
        return false;
    }
    return true;
}

} // namespace frostwind::detail
