#pragma once

#include <string>

struct SDL_Window;
struct SDL_GLContextState;

namespace frostwind {

struct WindowConfig {
    std::string title = "Frostwind";
    int width = 960;
    int height = 540;
};

// Owns the native window and its OpenGL context as one lifetime.
class Window {
public:
    Window() = default;
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool initialize(const WindowConfig& config = {});
    bool pixelSize(int& width, int& height) const;
    bool minimized() const;
    bool present() const;

private:
    SDL_Window* window_ = nullptr;
    SDL_GLContextState* context_ = nullptr;
    bool videoInitialized_ = false;
    bool vsync_ = false;
};

} // namespace frostwind
