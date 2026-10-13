#pragma once

#include <SDL3/SDL.h>

class SDLWindow;

class OpenGLContext {
public:
    OpenGLContext() = default;
    ~OpenGLContext();
    OpenGLContext(const OpenGLContext&) = delete;
    OpenGLContext& operator=(const OpenGLContext&) = delete;

    // SDL requires graphics attributes to be set before window creation.
    static bool configureWindowAttributes();
    bool initialize(const SDLWindow& window);
    bool present() const;

private:
    const SDLWindow* window_ = nullptr; // Borrowed; must outlive this context.
    SDL_GLContext context_ = nullptr;
    bool vsync_ = false;
};
