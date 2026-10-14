#include "QuadDemo.h"

#include <filesystem>

#include <SDL3/SDL.h>

bool QuadDemo::initialize()
{
    const char* basePath = SDL_GetBasePath();
    if (!basePath) {
        SDL_Log("Could not locate example assets: %s", SDL_GetError());
        return false;
    }
    if (!renderer_.initialize(std::filesystem::u8path(basePath) / "assets")) {
        return false;
    }
    SDL_Log("Toybox textured quad initialized. Press Escape or close the window to quit.");
    return true;
}

SDL_AppResult QuadDemo::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult QuadDemo::render(int width, int height)
{
    renderer_.draw(width, height);
    return SDL_APP_CONTINUE;
}
