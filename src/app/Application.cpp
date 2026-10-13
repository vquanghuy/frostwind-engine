#include "Application.h"

// Declare SDL's named callbacks without generating another platform entry point.
#define SDL_MAIN_USE_CALLBACKS 1
#define SDL_MAIN_NOIMPL 1
#include <SDL3/SDL_main.h>

#include <exception>
#include <memory>

SDL_AppResult SDLCALL Application::onInit(void** appstate, int, char**)
{
    *appstate = nullptr;
    try {
        auto application = std::make_unique<Application>();
        if (!application->initialize()) {
            return SDL_APP_FAILURE;
        }
        // SDL carries the state until onQuit takes ownership back.
        *appstate = application.release();
        return SDL_APP_CONTINUE;
    } catch (const std::exception& error) {
        SDL_Log("Application initialization failed: %s", error.what());
        return SDL_APP_FAILURE;
    }
}

SDL_AppResult SDLCALL Application::onEvent(void* appstate, SDL_Event* event)
{
    return static_cast<Application*>(appstate)->handleEvent(*event);
}

SDL_AppResult SDLCALL Application::onIterate(void* appstate)
{
    return static_cast<Application*>(appstate)->iterate();
}

void SDLCALL Application::onQuit(void* appstate, SDL_AppResult)
{
    const auto application = std::unique_ptr<Application>(static_cast<Application*>(appstate));
}

bool Application::initialize()
{
    if (!window_.initialize() ||
        !demoRenderer_.initialize()) {
        return false;
    }
    SDL_Log("Textured quad initialized. Press Escape or close the window to quit.");
    return true;
}

SDL_AppResult Application::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_QUIT ||
        event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED ||
        (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult Application::iterate()
{
    int width = 0;
    int height = 0;
    if (!window_.pixelSize(width, height)) {
        SDL_Log("Window pixel size query failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (window_.minimized() || width <= 0 || height <= 0) {
        SDL_Delay(16);
        return SDL_APP_CONTINUE;
    }
    demoRenderer_.draw(width, height);
    if (!window_.present()) {
        SDL_Log("OpenGL presentation failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

// SDL requires these named C entry points; all behavior belongs to Application.
SDL_AppResult SDLCALL SDL_AppInit(void** appstate, int argc, char** argv)
{
    return Application::onInit(appstate, argc, argv);
}

SDL_AppResult SDLCALL SDL_AppEvent(void* appstate, SDL_Event* event)
{
    return Application::onEvent(appstate, event);
}

SDL_AppResult SDLCALL SDL_AppIterate(void* appstate)
{
    return Application::onIterate(appstate);
}

void SDLCALL SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    Application::onQuit(appstate, result);
}
