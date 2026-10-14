#pragma once

#include <app/Application.h>

#include "QuadRenderer.h"

class QuadDemo final : public frostwind::Application {
protected:
    bool initialize() override;
    SDL_AppResult handleEvent(const SDL_Event& event) override;
    SDL_AppResult render(int width, int height) override;

private:
    QuadRenderer renderer_;
};
