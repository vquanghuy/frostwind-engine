#pragma once

struct SDL_Window;

namespace frostwind {
class Window;
}

namespace frostwind::detail {

// Private integration boundary: Window grants native access only to this base.
// Derived surfaces borrow the window and must be destroyed before it.
class WindowSurface {
public:
    virtual ~WindowSurface() = default;
    WindowSurface(const WindowSurface&) = delete;
    WindowSurface& operator=(const WindowSurface&) = delete;

    virtual bool initialize(Window& window) = 0;

protected:
    WindowSurface() = default;
    static SDL_Window* sdlWindow(Window& window);
};

} // namespace frostwind::detail
