#pragma once

#include <app/Application.h>
#include <rendering/Renderer.h>
#include <rendering/ShaderProgram.h>
#include <rendering/Texture2D.h>
#include <rendering/VertexBuffer.h>

// Derived GPU owners are destroyed before Application's Window/context.
class QuadDemo final : public frostwind::Application {
protected:
    bool initialize() override;
    SDL_AppResult handleEvent(const SDL_Event& event) override;
    SDL_AppResult render(frostwind::Renderer& renderer, int width, int height) override;

private:
    frostwind::VertexBuffer vertices_;
    frostwind::Texture2D texture_;
    frostwind::ShaderProgram program_;
    int scaleLocation_ = -1;
};
