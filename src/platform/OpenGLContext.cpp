#include "OpenGLContext.h"
#include "SDLWindow.h"

#include <glad/gl.h>

OpenGLContext::~OpenGLContext()
{
    if (context_) {
        SDL_GL_DestroyContext(context_);
    }
}

bool OpenGLContext::configureWindowAttributes()
{
    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG) ||
        !SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)) {
        SDL_Log("OpenGL attributes failed: %s", SDL_GetError());
        return false;
    }

    return true;
}

bool OpenGLContext::initialize(const SDLWindow& window)
{
    window_ = &window;
    context_ = SDL_GL_CreateContext(window_->window_);
    if (!context_ || !SDL_GL_MakeCurrent(window_->window_, context_)) {
        SDL_Log("OpenGL context creation failed: %s", SDL_GetError());
        return false;
    }

    const int version = gladLoadGL(SDL_GL_GetProcAddress);
    if (version == 0 || !GLAD_GL_VERSION_3_3) {
        SDL_Log("GLAD could not load the required OpenGL 3.3 Core functions.");
        return false;
    }
    SDL_Log("OpenGL %s; renderer: %s; GLAD loaded %d.%d",
        reinterpret_cast<const char*>(glGetString(GL_VERSION)),
        reinterpret_cast<const char*>(glGetString(GL_RENDERER)),
        GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    vsync_ = SDL_GL_SetSwapInterval(1);
    if (!vsync_) {
        SDL_Log("VSync unavailable: %s. Using a small frame delay.", SDL_GetError());
    }
    return true;
}

bool OpenGLContext::present() const
{
    if (!SDL_GL_SwapWindow(window_->window_)) {
        return false;
    }
    if (!vsync_) {
        SDL_Delay(16);
    }
    return true;
}
