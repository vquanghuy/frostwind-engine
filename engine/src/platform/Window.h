#pragma once

#include <string>

struct SDL_Window;
struct SDL_GLContextState;

namespace frostwind {

struct WindowConfig {
    std::string title = "Frostwind";
    int width = 960;
    int height = 540;
    bool vsync = true;
};

// Owns SDL video, its window, and an OpenGL context. GPU owners must die first.
class Window {
public:
    Window() = default;
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // One-time after success. Failed startup cleans up and permits retry.
    bool initialize(const WindowConfig& config = {});
    bool pixelSize(int& width, int& height) const;
    bool minimized() const;
    bool makeCurrent() const;
    bool present() const;

private:
    SDL_Window* window_ = nullptr;
    SDL_GLContextState* context_ = nullptr;
    bool videoInitialized_ = false;
};

} // namespace frostwind
