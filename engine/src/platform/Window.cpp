#include "Window.h"

#include <utility>

#include <SDL3/SDL.h>
#include <glad/gl.h>

namespace frostwind {

Window::~Window()
{
    if (context_ && !SDL_GL_DestroyContext(context_)) {
        SDL_Log("OpenGL context destruction failed: %s", SDL_GetError());
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
    if (window_ || config.width <= 0 || config.height <= 0) {
        SDL_Log("Window initialization requires an empty owner and positive dimensions.");
        return false;
    }
    Window candidate;
    candidate.videoInitialized_ = SDL_InitSubSystem(SDL_INIT_VIDEO);
    if (!candidate.videoInitialized_) {
        SDL_Log("SDL video initialization failed: %s", SDL_GetError());
        return false;
    }
    // SDL requires context attributes before window creation.
    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG) ||
        !SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) ||
        !SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8) ||
        !SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8) ||
        !SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8) ||
        !SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8) ||
        !SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0) ||
        !SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0)) {
        SDL_Log("OpenGL attributes failed: %s", SDL_GetError());
        return false;
    }
    candidate.window_ = SDL_CreateWindow(config.title.c_str(), config.width, config.height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!candidate.window_) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return false;
    }
    candidate.context_ = SDL_GL_CreateContext(candidate.window_);
    if (!candidate.context_ || !candidate.makeCurrent()) {
        SDL_Log("OpenGL context creation/activation failed: %s", SDL_GetError());
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
    if (!SDL_GL_SetSwapInterval(config.vsync ? 1 : 0)) {
        SDL_Log("Could not set the requested swap interval: %s. Software frame pacing remains independent.", SDL_GetError());
    }
    std::swap(window_, candidate.window_);
    std::swap(context_, candidate.context_);
    std::swap(videoInitialized_, candidate.videoInitialized_);
    return true;
}

bool Window::pixelSize(int& width, int& height) const
{
    return window_ && SDL_GetWindowSizeInPixels(window_, &width, &height);
}

bool Window::minimized() const
{
    return window_ && (SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED) != 0;
}

bool Window::makeCurrent() const
{
    return context_ && SDL_GL_MakeCurrent(window_, context_);
}

bool Window::present() const
{
    if (!makeCurrent() || !SDL_GL_SwapWindow(window_)) {
        SDL_Log("OpenGL presentation failed: %s", SDL_GetError());
        return false;
    }
    return true;
}

} // namespace frostwind
