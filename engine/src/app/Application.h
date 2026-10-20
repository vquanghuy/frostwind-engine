#pragma once

#include <memory>

#include <SDL3/SDL.h>
#include <core/Types.h>

#include "../platform/Window.h"
#include "../rendering/Renderer.h"

namespace frostwind {

struct ApplicationConfig {
    WindowConfig window;
    // Zero disables software pacing; VSync is configured separately.
    u32 maxFps = 60;
};

// Derived GPU members die before the base Window and its OpenGL context.
class Application {
public:
    using Factory = std::unique_ptr<Application> (*)();

    Application() = default;
    virtual ~Application() = default;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    static SDL_AppResult SDLCALL onInit(void** appstate, Factory factory, const ApplicationConfig& config = {});
    static SDL_AppResult SDLCALL onEvent(void* appstate, SDL_Event* event);
    static SDL_AppResult SDLCALL onIterate(void* appstate);
    static void SDLCALL onQuit(void* appstate, SDL_AppResult result);

protected:
    // Called after Window has initialized SDL, the context, and GLAD.
    virtual bool initialize() = 0;
    virtual SDL_AppResult handleEvent(const SDL_Event& event);
    // Borrow the persistent renderer during this callback; do not retain it.
    // Dimensions are drawable pixels. Continue presents; exit/failure skips swap.
    virtual SDL_AppResult render(Renderer& renderer, int width, int height) = 0;

private:
    SDL_AppResult iterate();
    Window window_;
    Renderer renderer_;
    u64 frameIntervalNs_ = 0;
};

} // namespace frostwind
