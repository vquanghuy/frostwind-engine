#pragma once

struct SDL_Window;
class OpenGLContext;

class SDLWindow {
public:
    SDLWindow() = default;
    ~SDLWindow();
    SDLWindow(const SDLWindow&) = delete;
    SDLWindow& operator=(const SDLWindow&) = delete;

    bool initialize();
    bool pixelSize(int& width, int& height) const;
    bool minimized() const;

private:
    // Native access is restricted to the platform context implementation.
    friend class OpenGLContext;
    SDL_Window* window_ = nullptr;
    bool videoInitialized_ = false;
};
