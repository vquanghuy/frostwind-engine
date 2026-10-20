#include "Application.h"

#include <algorithm>
#include <exception>
#include <memory>

#include <glad/gl.h>

#include "../rendering/GraphicsChecks.h"

namespace frostwind {

SDL_AppResult SDLCALL Application::onInit(void** appstate, Factory factory, const ApplicationConfig& config)
{
    *appstate = nullptr;
    try {
        auto application = factory ? factory() : nullptr;
        if (!application) {
            SDL_Log("Application factory did not create an application.");
            return SDL_APP_FAILURE;
        }
        if (!application->window_.initialize(config.window)) {
            return SDL_APP_FAILURE;
        }
        if (!application->initialize()) {
            return SDL_APP_FAILURE;
        }
        // Round up so integer division never raises the requested FPS ceiling.
        application->frameIntervalNs_ = config.maxFps == 0 ? 0 :
            (SDL_NS_PER_SECOND + u64{config.maxFps} - 1) / config.maxFps;
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
        auto& application = *static_cast<Application*>(appstate);
        if (event->type == SDL_EVENT_WINDOW_FOCUS_LOST || event->type == SDL_EVENT_WINDOW_MINIMIZED) {
            application.active_ = false;
            application.hasUpdateTimestamp_ = false;
            application.keyboard_.releaseAll();
            application.keyboard_.clearTransitions();
        } else if (event->type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
            application.active_ = true;
            application.hasUpdateTimestamp_ = false;
            application.keyboard_.releaseAll();
            application.keyboard_.clearTransitions();
        }
        if (application.active_) {
            application.keyboard_.handleEvent(*event);
        }
        return application.handleEvent(*event);
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

SDL_AppResult Application::update(f32)
{
    return SDL_APP_CONTINUE;
}

SDL_AppResult Application::iterate()
{
    const u64 frameStart = SDL_GetTicksNS();
    if (window_.minimized()) {
        hasUpdateTimestamp_ = false;
        keyboard_.releaseAll();
        keyboard_.clearTransitions();
        SDL_Delay(16);
        return SDL_APP_CONTINUE;
    }
    int width = 0, height = 0;
    if (!window_.pixelSize(width, height)) {
        SDL_Log("Window pixel size query failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (width <= 0 || height <= 0) {
        hasUpdateTimestamp_ = false;
        keyboard_.releaseAll();
        keyboard_.clearTransitions();
        const u64 budget = frameIntervalNs_ ? frameIntervalNs_ : 16 * SDL_NS_PER_MS;
        const u64 elapsed = SDL_GetTicksNS() - frameStart;
        if (elapsed < budget) {
            SDL_DelayPrecise(budget - elapsed);
        }
        return SDL_APP_CONTINUE;
    }
    if (!window_.makeCurrent()) {
        SDL_Log("Could not prepare the OpenGL rendering context: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    detail::requireOpenGL("Viewport");
    glViewport(0, 0, width, height);
    detail::checkOpenGL("Viewport");
    if (active_) {
        const f32 elapsedNs = frameStart - previousUpdateNs_;
        const f32 deltaSeconds = hasUpdateTimestamp_ ?
            std::min(elapsedNs / SDL_NS_PER_SECOND, 0.1f) : 0.0f;
        previousUpdateNs_ = frameStart;
        hasUpdateTimestamp_ = true;
        const SDL_AppResult updateResult = update(deltaSeconds);
        keyboard_.clearTransitions();
        if (updateResult != SDL_APP_CONTINUE) {
            return updateResult;
        }
    }
    const SDL_AppResult result = render(renderer_, width, height);
    if (result != SDL_APP_CONTINUE) {
        return result;
    }
    if (!window_.present()) {
        return SDL_APP_FAILURE;
    }
    // Rendering and swap blocking consume the budget. Late frames start fresh
    // next time instead of building up a debt of catch-up frames.
    const u64 elapsed = SDL_GetTicksNS() - frameStart;
    if (elapsed < frameIntervalNs_) {
        SDL_DelayPrecise(frameIntervalNs_ - elapsed);
    }
    return SDL_APP_CONTINUE;
}

} // namespace frostwind
