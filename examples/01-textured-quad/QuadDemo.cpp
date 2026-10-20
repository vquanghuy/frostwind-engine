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
        {-1, -1, 0, 0}, {1, -1, 1, 0}, {1, 1, 1, 1}, {-1, 1, 0, 1},
    };
    constexpr frostwind::u32 indices[] = {0, 1, 2, 2, 3, 0};
    vertices_.initialize(vertices, sizeof(vertices));
    indices_.initialize(indices, 6);
    vertexArray_.initialize(vertices_, indices_, {sizeof(Vertex), {
        {0, 2, offsetof(Vertex, x)}, {1, 2, offsetof(Vertex, u)},
    }});
    texture_.initializeFromFile(assetDirectory / "textures" / "crate.png");
    program_.initializeFromFiles(assetDirectory / "shaders" / "quad.vert", assetDirectory / "shaders" / "quad.frag");
    scaleLocation_ = program_.uniformLocation("uScale");
    const int textureLocation = program_.uniformLocation("uTexture");
    program_.use();
    program_.setInt(textureLocation, 0);
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
    renderer.clear({24.0f / 255.0f, 28.0f / 255.0f, 34.0f / 255.0f, 1.0f});
    program_.use();
    program_.setVec2(scaleLocation_, side / width, side / height);
    texture_.bind(0);
    renderer.drawTriangles(vertexArray_);
    return SDL_APP_CONTINUE;
}
