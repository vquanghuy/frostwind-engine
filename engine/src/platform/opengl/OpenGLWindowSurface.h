#pragma once

#include "../internal/WindowSurface.h"

struct SDL_Window;
struct SDL_GLContextState;

namespace frostwind::detail {

// Owns the context and borrows Window, which must outlive this object.
class OpenGLWindowSurface final : public WindowSurface {
public:
    OpenGLWindowSurface() = default;
    ~OpenGLWindowSurface() override;
    OpenGLWindowSurface(const OpenGLWindowSurface&) = delete;
    OpenGLWindowSurface& operator=(const OpenGLWindowSurface&) = delete;

    bool initialize(Window& window) override;
    bool makeCurrent() const;
    bool setVsync(bool enabled) const;
    bool present() const;

private:
    SDL_Window* window_ = nullptr;
    SDL_GLContextState* context_ = nullptr;
};

} // namespace frostwind::detail
