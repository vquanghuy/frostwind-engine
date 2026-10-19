#include "Window.h"

#include <SDL3/SDL.h>

#include "internal/WindowGraphicsSetup.h"

namespace frostwind {

Window::~Window()
{
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
    if (window_) {
        SDL_Log("Window is already initialized.");
        return false;
    }

    Window candidate;
    candidate.videoInitialized_ = SDL_InitSubSystem(SDL_INIT_VIDEO);
    if (!candidate.videoInitialized_) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return false;
    }
    SDL_WindowFlags graphicsFlags = 0;
    if (!detail::prepareGraphicsWindow(graphicsFlags)) {
        return false;
    }

    candidate.window_ = SDL_CreateWindow(config.title.c_str(), config.width, config.height,
        graphicsFlags | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!candidate.window_) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return false;
    }

    window_ = candidate.window_;
    videoInitialized_ = candidate.videoInitialized_;
    candidate.window_ = nullptr;
    candidate.videoInitialized_ = false;
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

} // namespace frostwind
