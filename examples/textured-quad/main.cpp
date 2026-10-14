#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#include <memory>

#include "QuadDemo.h"

namespace {
std::unique_ptr<toybox::Application> createApplication()
{
    return std::make_unique<QuadDemo>();
}
} // namespace

SDL_AppResult SDLCALL SDL_AppInit(void** appstate, int, char**)
{
    return toybox::Application::onInit(appstate, createApplication, {"Toybox - Textured Quad", 800, 600});
}

SDL_AppResult SDLCALL SDL_AppEvent(void* appstate, SDL_Event* event)
{
    return toybox::Application::onEvent(appstate, event);
}

SDL_AppResult SDLCALL SDL_AppIterate(void* appstate)
{
    return toybox::Application::onIterate(appstate);
}

void SDLCALL SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    toybox::Application::onQuit(appstate, result);
}
