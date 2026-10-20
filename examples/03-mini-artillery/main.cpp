#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

#include <memory>

#include <SDL3/SDL.h>

#include <app/Application.h>

namespace {
class MiniArtilleryApp final : public frostwind::Application {
protected:
    bool initialize() override
    {
        SDL_Log("Frostwind Mini Artillery scaffold initialized. Terrain and gameplay are pending. Press Escape or close the window to quit.");
        return true;
    }

    SDL_AppResult handleEvent(const SDL_Event& event) override
    {
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
            return SDL_APP_SUCCESS;
        }
        return SDL_APP_CONTINUE;
    }

    SDL_AppResult render(frostwind::RenderFrame& frame) override
    {
        return frame.clear({0.06f, 0.08f, 0.05f, 1.0f}) ? SDL_APP_CONTINUE : SDL_APP_FAILURE;
    }
};

std::unique_ptr<frostwind::Application> createApplication()
{
    return std::make_unique<MiniArtilleryApp>();
}
} // namespace

SDL_AppResult SDLCALL SDL_AppInit(void** appstate, int, char**)
{
    frostwind::ApplicationConfig config;
    config.window = {"Frostwind - Mini Artillery", 960, 540};
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
