#include "Window.h"

#include <SDL3/SDL.h>
#include <glad/gl.h>

namespace frostwind {

Window::~Window()
{
    if (context_) {
        SDL_GL_DestroyContext(context_);
    }
    if (window_) {
        SDL_DestroyWindow(window_);
    }
    if (videoInitialized_) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
    // SDL's callback framework performs the final SDL_Quit.
}

bool Window::initialize(const WindowConfig& config)
{
    videoInitialized_ = SDL_InitSubSystem(SDL_INIT_VIDEO);
    if (!videoInitialized_) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return false;
    }
    // SDL requires graphics attributes before window creation.
    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG) ||
        !SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)) {
        SDL_Log("OpenGL attributes failed: %s", SDL_GetError());
        return false;
    }

    window_ = SDL_CreateWindow(config.title.c_str(), config.width, config.height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window_) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return false;
    }
    context_ = SDL_GL_CreateContext(window_);
    if (!context_ || !SDL_GL_MakeCurrent(window_, context_)) {
        SDL_Log("OpenGL context creation failed: %s", SDL_GetError());
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

    vsync_ = SDL_GL_SetSwapInterval(1);
    if (!vsync_) {
        SDL_Log("VSync unavailable: %s. Using a small frame delay.", SDL_GetError());
    }
    return true;
}

bool Window::pixelSize(int& width, int& height) const
{
    return SDL_GetWindowSizeInPixels(window_, &width, &height);
}

bool Window::minimized() const
{
    return (SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED) != 0;
}

bool Window::present() const
{
    if (!SDL_GL_SwapWindow(window_)) {
        return false;
    }
    if (!vsync_) {
        SDL_Delay(16);
    }
    return true;
}

} // namespace frostwind
