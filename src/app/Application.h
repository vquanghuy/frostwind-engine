#pragma once

#include <SDL3/SDL.h>

#include "../platform/SDLWindow.h"
#include "../platform/OpenGLContext.h"
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

    // Destruction runs in reverse order: demo resources, context, window, SDL.
    SDLWindow window_;
    OpenGLContext context_;
    QuadRenderer demoRenderer_;
};
