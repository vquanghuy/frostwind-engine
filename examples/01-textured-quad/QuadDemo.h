#pragma once

#include <app/Application.h>
#include <rendering/Buffer.h>
#include <rendering/GraphicsPipeline.h>
#include <rendering/Texture2D.h>

// Example GPU members are destroyed before the base application's device.
class QuadDemo final : public frostwind::Application {
protected:
    bool initialize() override;
    SDL_AppResult handleEvent(const SDL_Event& event) override;
    SDL_AppResult render(frostwind::RenderFrame& frame) override;

private:
    frostwind::Buffer vertices_;
    frostwind::Texture2D texture_;
    frostwind::GraphicsPipeline pipeline_;
};
