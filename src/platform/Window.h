#pragma once

struct SDL_Window;
struct SDL_GLContextState;

// Owns the native window and its OpenGL context as one lifetime.
class Window {
public:
    Window() = default;
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool initialize();
    bool pixelSize(int& width, int& height) const;
    bool minimized() const;
    bool present() const;

private:
    SDL_Window* window_ = nullptr;
    SDL_GLContextState* context_ = nullptr;
    bool videoInitialized_ = false;
    bool vsync_ = false;
};
