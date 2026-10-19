#include "WindowSurface.h"

#include "../Window.h"

namespace frostwind::detail {

SDL_Window* WindowSurface::sdlWindow(Window& window)
{
    return window.window_;
}

} // namespace frostwind::detail
