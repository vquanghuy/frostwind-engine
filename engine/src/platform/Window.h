#pragma once

#include <string>

struct SDL_Window;

namespace frostwind {

namespace detail {
class WindowSurface;
}

struct WindowConfig {
    std::string title = "Frostwind";
    int width = 960;
    int height = 540;
};

// Owns SDL video/window lifetime. Graphics resources must be destroyed first.
class Window {
public:
    Window() = default;
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool initialize(const WindowConfig& config = {});
    bool pixelSize(int& width, int& height) const;
    bool minimized() const;

private:
    friend class detail::WindowSurface;

    SDL_Window* window_ = nullptr;
    bool videoInitialized_ = false;
};

} // namespace frostwind
