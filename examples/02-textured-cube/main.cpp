#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#include <memory>

#include "CubeDemo.h"

namespace {
std::unique_ptr<frostwind::Application> createApplication()
{
    return std::make_unique<CubeDemo>();
}
} // namespace

SDL_AppResult SDLCALL SDL_AppInit(void** appstate, int, char**)
{
    frostwind::ApplicationConfig config;
    config.window = {"Frostwind - Textured Cube", 960, 540};
    config.window.depthBuffer = true;
    config.window.msaaSamples = 4;
    config.maxFps = 120;
    return frostwind::Application::onInit(appstate, createApplication, config);
}

SDL_AppResult SDLCALL SDL_AppEvent(void* appstate, SDL_Event* event)
{
    return frostwind::Application::onEvent(appstate, event);
}

SDL_AppResult SDLCALL SDL_AppIterate(void* appstate)
{
    return frostwind::Application::onIterate(appstate);
}

void SDLCALL SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    frostwind::Application::onQuit(appstate, result);
}
