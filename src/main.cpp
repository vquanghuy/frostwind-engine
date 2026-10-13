#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

SDL_AppResult SDL_AppInit(void** appstate, int, char**)
{
    // An empty window only needs updates when events arrive.
    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "waitevent");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Mini Artillery", 960, 540, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    *appstate = window;
    SDL_Log("Mini Artillery window created. Press Escape or close the window to quit.");
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void*, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT ||
        event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED ||
        (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE)) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    SDL_Window* window = static_cast<SDL_Window*>(appstate);
    SDL_Surface* surface = SDL_GetWindowSurface(window);
    if (!surface ||
        !SDL_FillSurfaceRect(surface, nullptr, SDL_MapSurfaceRGB(surface, 24, 28, 34)) ||
        !SDL_UpdateWindowSurface(window)) {
        SDL_Log("Window update failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult)
{
    SDL_DestroyWindow(static_cast<SDL_Window*>(appstate));
    // SDL's main callback framework calls SDL_Quit after this callback.
}
