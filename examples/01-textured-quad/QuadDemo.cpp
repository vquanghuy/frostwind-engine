#include "QuadDemo.h"

#include <algorithm>
#include <cstddef>
#include <filesystem>

#include <SDL3/SDL.h>

#include <core/Types.h>
#include <rendering/GraphicsDevice.h>
#include <rendering/RenderFrame.h>
#include <rendering/Shader.h>

bool QuadDemo::initialize()
{
    const char* basePath = SDL_GetBasePath();
    if (!basePath) {
        SDL_Log("Could not locate example assets: %s", SDL_GetError());
        return false;
    }
    const auto assetDirectory = std::filesystem::u8path(basePath) / "assets";
    auto& device = graphics();
    // Local coordinates: x right, y up; UV origin at the bottom left.
    struct Vertex { frostwind::f32 x, y, u, v; };
    constexpr Vertex vertices[] = {
        {-1, -1, 0, 0}, {1, -1, 1, 0}, {1, 1, 1, 1},
        {-1, -1, 0, 0}, {1, 1, 1, 1}, {-1, 1, 0, 1},
    };
    if (!vertices_.initialize(device, vertices, sizeof(vertices)) ||
        !texture_.initializeFromFile(device, assetDirectory / "textures" / "crate.png")) {
        return false;
    }
    frostwind::Shader vertex;
    frostwind::Shader fragment;
    if (!vertex.initializeFromFile(device, {frostwind::ShaderStage::Vertex}, assetDirectory / "shaders" / "quad.vert") ||
        !fragment.initializeFromFile(device, {frostwind::ShaderStage::Fragment}, assetDirectory / "shaders" / "quad.frag")) {
        return false;
    }
    frostwind::PipelineDescription description;
    description.vertexStride = sizeof(Vertex);
    description.attributes = {
        {0, frostwind::VertexFormat::Float2, offsetof(Vertex, x)},
        {1, frostwind::VertexFormat::Float2, offsetof(Vertex, u)},
    };
    description.parameters = {"QuadParameters", 16};
    description.texture = {"uTexture"};
    description.targetFormat = frostwind::ColorFormat::RGBA8;
    description.topology = frostwind::PrimitiveTopology::TriangleList;
    description.blend = frostwind::BlendMode::Opaque;
    description.depth = frostwind::DepthMode::Disabled;
    description.cull = frostwind::CullMode::None;
    // Pipeline retains the required shader resources after these locals die.
    if (!pipeline_.initialize(device, description, vertex, fragment)) {
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

SDL_AppResult QuadDemo::render(frostwind::RenderFrame& frame)
{
    if (!frame.clear({24.0f / 255.0f, 28.0f / 255.0f, 34.0f / 255.0f, 1.0f})) {
        return SDL_APP_FAILURE;
    }
    const auto& target = frame.target();
    const frostwind::f32 side = std::min(target.width, target.height) * 0.66f;
    struct alignas(16) Parameters {
        frostwind::f32 scaleX, scaleY;
        frostwind::f32 padding[2] = {};
    };
    static_assert(sizeof(Parameters) == 16, "Match the shader's parameter block.");
    const Parameters parameters{side / target.width, side / target.height};
    return frame.draw(pipeline_, vertices_, 6, 0, &texture_, &parameters, sizeof(parameters))
        ? SDL_APP_CONTINUE : SDL_APP_FAILURE;
}
