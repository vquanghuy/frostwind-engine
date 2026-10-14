#pragma once

#include <memory>

#include <SDL3/SDL.h>

#include "../platform/Window.h"

namespace frostwind {

// Derived applications supply behavior; their members die before Window.
class Application {
public:
    using Factory = std::unique_ptr<Application> (*)();

    Application() = default;
    virtual ~Application() = default;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    static SDL_AppResult SDLCALL onInit(void** appstate, Factory factory, const WindowConfig& config = {});
    static SDL_AppResult SDLCALL onEvent(void* appstate, SDL_Event* event);
    static SDL_AppResult SDLCALL onIterate(void* appstate);
    static void SDLCALL onQuit(void* appstate, SDL_AppResult result);

protected:
    virtual bool initialize() = 0;
    virtual SDL_AppResult handleEvent(const SDL_Event& event);
    virtual SDL_AppResult render(int width, int height) = 0;

private:
    SDL_AppResult iterate();
    Window window_;
};

} // namespace frostwind
