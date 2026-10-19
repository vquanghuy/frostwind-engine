#include "OpenGLWindowSurface.h"

#include <SDL3/SDL.h>

#include "../internal/WindowGraphicsSetup.h"

namespace frostwind::detail {

bool prepareGraphicsWindow(SDL_WindowFlags& flags)
{
    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG) ||
        !SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)) {
        SDL_Log("OpenGL attributes failed: %s", SDL_GetError());
        return false;
    }
    flags = SDL_WINDOW_OPENGL;
    return true;
}

OpenGLWindowSurface::~OpenGLWindowSurface()
{
    if (context_ && !SDL_GL_DestroyContext(context_)) {
        SDL_Log("OpenGL context destruction failed: %s", SDL_GetError());
    }
}

bool OpenGLWindowSurface::initialize(Window& window)
{
    if (context_) {
        SDL_Log("OpenGL window surface is already initialized.");
        return false;
    }
    SDL_Window* nativeWindow = sdlWindow(window);
    if (!nativeWindow) {
        SDL_Log("OpenGL context creation requires an initialized window.");
        return false;
    }

    OpenGLWindowSurface candidate;
    candidate.context_ = SDL_GL_CreateContext(nativeWindow);
    if (!candidate.context_) {
        SDL_Log("OpenGL context creation failed: %s", SDL_GetError());
        return false;
    }
    if (!SDL_GL_MakeCurrent(nativeWindow, candidate.context_)) {
        SDL_Log("Making OpenGL context current failed: %s", SDL_GetError());
        return false;
    }
    window_ = nativeWindow;
    context_ = candidate.context_;
    candidate.context_ = nullptr;
    return true;
}

bool OpenGLWindowSurface::setVsync(bool enabled) const
{
    return SDL_GL_SetSwapInterval(enabled ? 1 : 0);
}

bool OpenGLWindowSurface::present() const
{
    return SDL_GL_SwapWindow(window_);
}

} // namespace frostwind::detail
