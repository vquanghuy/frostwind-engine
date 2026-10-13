#include "SDLWindow.h"
#include "OpenGLContext.h"

#include <SDL3/SDL.h>

SDLWindow::~SDLWindow()
{
    if (window_) {
        SDL_DestroyWindow(window_);
    }
    if (videoInitialized_) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
    // SDL's callback framework performs the final SDL_Quit.
}

bool SDLWindow::initialize()
{
    videoInitialized_ = SDL_InitSubSystem(SDL_INIT_VIDEO);
    if (!videoInitialized_) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return false;
    }
    // This native window is currently configured for the OpenGL renderer.
    if (!OpenGLContext::configureWindowAttributes()) {
        return false;
    }
    window_ = SDL_CreateWindow("Mini Artillery", 960, 540,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window_) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return false;
    }
    return true;
}

bool SDLWindow::pixelSize(int& width, int& height) const
{
    return SDL_GetWindowSizeInPixels(window_, &width, &height);
}

bool SDLWindow::minimized() const
{
    return (SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED) != 0;
}
