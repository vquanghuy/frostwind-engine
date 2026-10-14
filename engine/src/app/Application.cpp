#include "Application.h"

#include <exception>
#include <memory>

#include "../rendering/Texture2D.h"

namespace frostwind {

SDL_AppResult SDLCALL Application::onInit(void** appstate, Factory factory, const WindowConfig& config)
{
    *appstate = nullptr;
    try {
        auto application = factory ? factory() : nullptr;
        if (!application) {
            SDL_Log("Application factory did not create an application.");
            return SDL_APP_FAILURE;
        }
        if (!application->window_.initialize(config)) {
            return SDL_APP_FAILURE;
        }
        if (!Texture2D::initializeSupport() || !application->initialize()) {
            return SDL_APP_FAILURE;
        }
        // SDL carries the state until onQuit takes ownership back.
        *appstate = application.release();
        return SDL_APP_CONTINUE;
    } catch (const std::exception& error) {
        SDL_Log("Application initialization failed: %s", error.what());
    } catch (...) {
        SDL_Log("Application initialization failed with an unknown exception.");
    }
    return SDL_APP_FAILURE;
}

SDL_AppResult SDLCALL Application::onEvent(void* appstate, SDL_Event* event)
{
    try {
        if (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            return SDL_APP_SUCCESS;
        }
        return static_cast<Application*>(appstate)->handleEvent(*event);
    } catch (const std::exception& error) {
        SDL_Log("Application event failed: %s", error.what());
    } catch (...) {
        SDL_Log("Application event failed with an unknown exception.");
    }
    return SDL_APP_FAILURE;
}

SDL_AppResult SDLCALL Application::onIterate(void* appstate)
{
    try {
        return static_cast<Application*>(appstate)->iterate();
    } catch (const std::exception& error) {
        SDL_Log("Application frame failed: %s", error.what());
    } catch (...) {
        SDL_Log("Application frame failed with an unknown exception.");
    }
    return SDL_APP_FAILURE;
}

void SDLCALL Application::onQuit(void* appstate, SDL_AppResult)
{
    const auto application = std::unique_ptr<Application>(static_cast<Application*>(appstate));
}

SDL_AppResult Application::handleEvent(const SDL_Event&)
{
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
    const SDL_AppResult result = render(width, height);
    if (result != SDL_APP_CONTINUE) {
        return result;
    }
    if (!window_.present()) {
        SDL_Log("OpenGL presentation failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

} // namespace frostwind
