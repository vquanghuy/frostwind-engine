#pragma once

#include <app/Application.h>
#include <camera/PerspectiveCamera.h>
#include <camera/OrbitCameraController.h>
#include <rendering/IndexBuffer.h>
#include <rendering/ShaderProgram.h>
#include <rendering/Texture2D.h>
#include <rendering/VertexArray.h>
#include <rendering/VertexBuffer.h>

// Scene, input bindings, and model animation belong to this example.
class CubeDemo final : public frostwind::Application {
protected:
    bool initialize() override;
    SDL_AppResult handleEvent(const SDL_Event& event) override;
    SDL_AppResult update(frostwind::f32 deltaSeconds) override;
    SDL_AppResult render(frostwind::Renderer& renderer, int width, int height) override;

private:
    void resetScene();

    frostwind::VertexBuffer vertices_;
    frostwind::IndexBuffer indices_;
    frostwind::VertexArray vertexArray_;
    frostwind::Texture2D texture_;
    frostwind::ShaderProgram program_;
    frostwind::PerspectiveCamera camera_;
    frostwind::OrbitCameraController cameraController_;
    int modelLocation_ = -1;
    int viewLocation_ = -1;
    int projectionLocation_ = -1;
    int cameraPositionLocation_ = -1;
    frostwind::f32 rotation_ = 0;
    bool rotating_ = true;
};
