#include "QuadDemo.h"

#include <algorithm>
#include <cstddef>
#include <filesystem>

#include <SDL3/SDL.h>

#include <core/Types.h>

bool QuadDemo::initialize()
{
    const char* basePath = SDL_GetBasePath();
    if (!basePath) {
        SDL_Log("Could not locate example assets: %s", SDL_GetError());
        return false;
    }
    const auto assetDirectory = std::filesystem::u8path(basePath) / "assets";
    // Local coordinates: x right, y up; UV origin at the bottom left.
    struct Vertex { frostwind::f32 x, y, u, v; };
    constexpr Vertex vertices[] = {
        {-1, -1, 0, 0}, {1, -1, 1, 0}, {1, 1, 1, 1},
        {-1, -1, 0, 0}, {1, 1, 1, 1}, {-1, 1, 0, 1},
    };
    if (!vertices_.initialize(vertices, sizeof(vertices), sizeof(Vertex), {
            {0, 2, offsetof(Vertex, x)}, {1, 2, offsetof(Vertex, u)},
        }) ||
        !texture_.initializeFromFile(assetDirectory / "textures" / "crate.png") ||
        !program_.initializeFromFiles(assetDirectory / "shaders" / "quad.vert", assetDirectory / "shaders" / "quad.frag")) {
        return false;
    }
    scaleLocation_ = program_.uniformLocation("uScale");
    const int textureLocation = program_.uniformLocation("uTexture");
    if (scaleLocation_ < 0 || textureLocation < 0 || !program_.use() || !program_.setInt(textureLocation, 0)) {
        SDL_Log("Quad shader requires uScale and uTexture uniforms.");
        return false;
    }
    SDL_Log("Frostwind textured quad initialized. Press Escape or close the window to quit.");
    return true;
}

SDL_AppResult QuadDemo::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult QuadDemo::render(frostwind::Renderer& renderer, int width, int height)
{
    const frostwind::f32 side = std::min(width, height) * 0.66f;
    if (!renderer.clear({24.0f / 255.0f, 28.0f / 255.0f, 34.0f / 255.0f, 1.0f})) {
        SDL_Log("Quad background clear failed.");
        return SDL_APP_FAILURE;
    }
    if (!program_.use()) {
        SDL_Log("Quad shader activation failed.");
        return SDL_APP_FAILURE;
    }
    if (!program_.setVec2(scaleLocation_, side / width, side / height)) {
        SDL_Log("Quad scale uniform update failed.");
        return SDL_APP_FAILURE;
    }
    if (!texture_.bind(0)) {
        SDL_Log("Quad texture binding failed.");
        return SDL_APP_FAILURE;
    }
    if (!renderer.drawTriangles(vertices_, 6)) {
        SDL_Log("Quad draw failed.");
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}
