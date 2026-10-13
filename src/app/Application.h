#pragma once

#include <SDL3/SDL.h>

#include "../platform/Window.h"
#include "../rendering/QuadRenderer.h"

class Application {
public:
    static SDL_AppResult SDLCALL onInit(void** appstate, int argc, char** argv);
    static SDL_AppResult SDLCALL onEvent(void* appstate, SDL_Event* event);
    static SDL_AppResult SDLCALL onIterate(void* appstate);
    static void SDLCALL onQuit(void* appstate, SDL_AppResult result);

private:
    bool initialize();
    SDL_AppResult handleEvent(const SDL_Event& event);
    SDL_AppResult iterate();

    // Destroy demo resources before Window releases its context and SDL state.
    Window window_;
    QuadRenderer demoRenderer_;
};
