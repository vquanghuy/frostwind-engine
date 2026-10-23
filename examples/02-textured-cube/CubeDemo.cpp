#include "CubeDemo.h"

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <iterator>

#include <SDL3/SDL.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

bool CubeDemo::initialize()
{
    const char* basePath = SDL_GetBasePath();
    if (!basePath) {
        SDL_Log("[CubeDemo] Could not locate cube assets: %s", SDL_GetError());
        return false;
    }
    const auto assets = std::filesystem::u8path(basePath) / "assets";
    struct Vertex { frostwind::f32 x, y, z, u, v, nx, ny, nz; };
    // Separate vertices preserve each face's UV orientation and sharp normal.
    // Every face is counterclockwise when viewed from outside the cube.
    constexpr Vertex vertices[] = {
        {-0.5f,-0.5f, 0.5f, 0,0, 0, 0, 1}, { 0.5f,-0.5f, 0.5f, 1,0, 0, 0, 1},
        { 0.5f, 0.5f, 0.5f, 1,1, 0, 0, 1}, {-0.5f, 0.5f, 0.5f, 0,1, 0, 0, 1}, // +Z
        { 0.5f,-0.5f,-0.5f, 0,0, 0, 0, -1}, {-0.5f,-0.5f,-0.5f, 1,0, 0, 0, -1},
        {-0.5f, 0.5f,-0.5f, 1,1, 0, 0, -1}, { 0.5f, 0.5f,-0.5f, 0,1, 0, 0, -1}, // -Z
        { 0.5f,-0.5f, 0.5f, 0,0, 1, 0, 0}, { 0.5f,-0.5f,-0.5f, 1,0, 1, 0, 0},
        { 0.5f, 0.5f,-0.5f, 1,1, 1, 0, 0}, { 0.5f, 0.5f, 0.5f, 0,1, 1, 0, 0}, // +X
        {-0.5f,-0.5f,-0.5f, 0,0, -1, 0, 0}, {-0.5f,-0.5f, 0.5f, 1,0, -1, 0, 0},
        {-0.5f, 0.5f, 0.5f, 1,1, -1, 0, 0}, {-0.5f, 0.5f,-0.5f, 0,1, -1, 0, 0}, // -X
        {-0.5f, 0.5f, 0.5f, 0,0, 0, 1, 0}, { 0.5f, 0.5f, 0.5f, 1,0, 0, 1, 0},
        { 0.5f, 0.5f,-0.5f, 1,1, 0, 1, 0}, {-0.5f, 0.5f,-0.5f, 0,1, 0, 1, 0}, // +Y
        {-0.5f,-0.5f,-0.5f, 0,0, 0, -1, 0}, { 0.5f,-0.5f,-0.5f, 1,0, 0, -1, 0},
        { 0.5f,-0.5f, 0.5f, 1,1, 0, -1, 0}, {-0.5f,-0.5f, 0.5f, 0,1, 0, -1, 0}, // -Y
    };
    constexpr frostwind::u32 indices[] = {
         0, 1, 2,  2, 3, 0,   4, 5, 6,  6, 7, 4,
         8, 9,10, 10,11, 8,  12,13,14, 14,15,12,
        16,17,18, 18,19,16,  20,21,22, 22,23,20,
    };
    vertices_.initialize(vertices, sizeof(vertices));
    indices_.initialize(indices, std::size(indices));
    vertexArray_.initialize(vertices_, indices_, {sizeof(Vertex), {
        {0, 3, offsetof(Vertex, x)}, {1, 2, offsetof(Vertex, u)},
        {2, 3, offsetof(Vertex, nx)},
    }});
    texture_.initializeFromFile(assets / "textures" / "crate.png",
        {frostwind::TextureFilter::Trilinear, 16});
    program_.initializeFromFiles(assets / "shaders" / "cube.vert", assets / "shaders" / "cube.frag");
    modelLocation_ = program_.uniformLocation("uModel");
    viewLocation_ = program_.uniformLocation("uView");
    projectionLocation_ = program_.uniformLocation("uProjection");
    cameraPositionLocation_ = program_.uniformLocation("uCameraPosition");
    const int textureLocation = program_.uniformLocation("uTexture");
    const int lightDirectionLocation = program_.uniformLocation("uLightDirection");
    const int lightColorLocation = program_.uniformLocation("uLightColor");
    const int ambientColorLocation = program_.uniformLocation("uAmbientColor");
    const int specularColorLocation = program_.uniformLocation("uSpecularColor");
    const int shininessLocation = program_.uniformLocation("uShininess");
    program_.use();
    program_.setInt(textureLocation, 0);
    // A shallow light angle makes a visible reflection sweep across the side
    // faces from the initial orbit view. Values remain example-owned policy.
    program_.setVec3(lightDirectionLocation, 0.4f, -0.1f, -0.6f);
    program_.setVec3(lightColorLocation, 0.8f, 0.8f, 0.8f);
    program_.setVec3(ambientColorLocation, 0.192f, 0.192f, 0.192f);
    program_.setVec3(specularColorLocation, 0.8f, 0.8f, 0.8f);
    program_.setFloat(shininessLocation, 16);
    resetScene();
    SDL_Log("[CubeDemo] Textured Cube: arrows orbit, W/S move closer/farther, Space pauses rotation, R resets, Escape quits.");
    return true;
}

SDL_AppResult CubeDemo::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

void CubeDemo::resetScene()
{
    frostwind::OrbitCameraConfig cameraConfig;
    cameraConfig.yaw = 0.6f;
    cameraConfig.pitch = 0.35f;
    cameraConfig.distance = 4;
    cameraConfig.minDistance = 1.5f;
    cameraConfig.maxDistance = 12;
    cameraController_.reset(camera_, cameraConfig);
    rotation_ = 0;
    rotating_ = true;
}

SDL_AppResult CubeDemo::update(frostwind::f32 deltaSeconds)
{
    const auto& keys = keyboard();
    if (keys.wasPressed(SDL_SCANCODE_R)) {
        resetScene();
        return SDL_APP_CONTINUE;
    }
    if (keys.wasPressed(SDL_SCANCODE_SPACE)) {
        rotating_ = !rotating_;
    }
    const frostwind::f32 horizontal = keys.isDown(SDL_SCANCODE_RIGHT) - keys.isDown(SDL_SCANCODE_LEFT);
    const frostwind::f32 vertical = keys.isDown(SDL_SCANCODE_UP) - keys.isDown(SDL_SCANCODE_DOWN);
    const frostwind::f32 travel = keys.isDown(SDL_SCANCODE_S) - keys.isDown(SDL_SCANCODE_W);
    cameraController_.update(camera_, {horizontal, vertical, travel}, deltaSeconds);
    if (rotating_) {
        rotation_ = std::remainder(rotation_ + deltaSeconds * 0.6f, glm::two_pi<frostwind::f32>());
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult CubeDemo::render(frostwind::Renderer& renderer, int width, int height)
{
    const frostwind::f32 drawableWidth = width;
    camera_.setPerspective(glm::radians(50.0f), drawableWidth / height, 0.1f, 100.0f);
    // T * R * S acts on column vectors right-to-left. View and projection
    // remain separate uniforms so each transform is easy to experiment with.
    const auto translation = glm::translate(glm::mat4{1}, glm::vec3{0, 0, 0});
    const auto rotation = glm::rotate(glm::mat4{1}, rotation_, glm::vec3{0, 1, 0});
    const auto scale = glm::scale(glm::mat4{1}, glm::vec3{1});
    const auto model = translation * rotation * scale;
    renderer.setDepthTest(true);
    renderer.setBackFaceCulling(true);
    renderer.clear({0.035f, 0.045f, 0.065f, 1});
    renderer.clearDepth();
    program_.use();
    program_.setMat4(modelLocation_, glm::value_ptr(model));
    program_.setMat4(viewLocation_, glm::value_ptr(camera_.view()));
    program_.setMat4(projectionLocation_, glm::value_ptr(camera_.projection()));
    const auto& cameraPosition = camera_.position();
    program_.setVec3(cameraPositionLocation_, cameraPosition.x, cameraPosition.y, cameraPosition.z);
    texture_.bind();
    renderer.drawTriangles(vertexArray_);
    return SDL_APP_CONTINUE;
}
