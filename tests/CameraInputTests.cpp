#include <camera/PerspectiveCamera.h>
#include <input/Keyboard.h>

#include <cmath>
#include <cstdio>
#include <exception>
#include <limits>
#include <stdexcept>

#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/vec4.hpp>

namespace {

void require(bool condition, const char* message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool near(float a, float b)
{
    return std::abs(a - b) < 0.0001f;
}

template<class Action>
void rejects(Action action)
{
    bool rejected = false;
    try {
        action();
    } catch (const std::runtime_error&) {
        rejected = true;
    }
    require(rejected, "Invalid camera state was accepted.");
}

void cameraChecks()
{
    frostwind::PerspectiveCamera camera;
    camera.setView({0, 0, 3}, {0, 0, 0});
    const auto origin = camera.view() * glm::vec4{0, 0, 0, 1};
    require(near(origin.x, 0) && near(origin.y, 0) && near(origin.z, -3), "View translation sign.");
    camera.setPerspective(glm::half_pi<float>(), 2, 1, 10);
    const auto project = [&](const glm::vec4& point) {
        const auto clip = camera.projection() * point;
        return glm::vec3{clip} / clip.w;
    };
    const auto corner = project({2, 1, -1, 1});
    require(near(corner.x, 1) && near(corner.y, 1) && near(corner.z, -1), "Near frustum corner/NDC depth.");
    require(near(project({0, 0, -10, 1}).z, 1), "Far plane must map to OpenGL +1 depth.");
    require(near(project({1, 0, -2, 1}).x, project({1, 0, -1, 1}).x / 2), "Perspective foreshortening.");
    const float wideX = project({1, 0, -2, 1}).x;
    camera.setPerspective(glm::half_pi<float>(), 1, 1, 10);
    require(near(project({1, 0, -2, 1}).x, wideX * 2), "Resize aspect ratio.");
    camera.setView({3, 0, 0}, {0, 0, 0});
    const auto side = camera.view() * glm::vec4{0, 0, 0, 1};
    require(near(side.x, 0) && near(side.z, -3), "Orbit side view.");
    const auto combined = camera.viewProjection() * glm::vec4{0.2f, 0.3f, 0.4f, 1};
    const auto separate = camera.projection() * camera.view() * glm::vec4{0.2f, 0.3f, 0.4f, 1};
    require(glm::length(combined - separate) < 0.0001f, "View/projection multiplication order.");
    const auto oldView = camera.view();
    const auto oldProjection = camera.projection();
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float infinity = std::numeric_limits<float>::infinity();
    rejects([&] { camera.setView({0, 0, 0}, {0, 0, 0}); });
    rejects([&] { camera.setView({nan, 0, 3}, {0, 0, 0}); });
    rejects([&] { camera.setView({0, 0, 3}, {0, 0, 0}, {0, 0, 0}); });
    rejects([&] { camera.setView({0, 0, 3}, {0, 0, 0}, {0, 0, 1}); });
    rejects([&] { camera.setPerspective(0, 1, 1, 10); });
    rejects([&] { camera.setPerspective(glm::pi<float>(), 1, 1, 10); });
    rejects([&] { camera.setPerspective(1, 0, 1, 10); });
    rejects([&] { camera.setPerspective(1, 1, 0, 10); });
    rejects([&] { camera.setPerspective(1, 1, 10, 1); });
    rejects([&] { camera.setPerspective(1, infinity, 1, 10); });
    rejects([&] { camera.setPerspective(nan, 1, 1, 10); });
    require(camera.view() == oldView && camera.projection() == oldProjection, "Rejected setters changed camera state.");
}

void keyboardChecks()
{
    frostwind::Keyboard keys;
    const auto send = [&](Uint32 type, SDL_Scancode key, bool repeat = false) {
        SDL_Event event{};
        event.type = type;
        event.key.scancode = key;
        event.key.repeat = repeat;
        keys.handleEvent(event);
    };
    send(SDL_EVENT_KEY_DOWN, SDL_SCANCODE_W);
    require(keys.isDown(SDL_SCANCODE_W) && keys.wasPressed(SDL_SCANCODE_W), "Key press missing.");
    keys.clearTransitions();
    require(keys.isDown(SDL_SCANCODE_W) && !keys.wasPressed(SDL_SCANCODE_W), "Held key/edge consumption.");
    send(SDL_EVENT_KEY_DOWN, SDL_SCANCODE_W, true);
    send(SDL_EVENT_KEY_DOWN, SDL_SCANCODE_W);
    require(!keys.wasPressed(SDL_SCANCODE_W), "Repeat/duplicate created a new press.");
    send(SDL_EVENT_KEY_DOWN, SDL_SCANCODE_A);
    send(SDL_EVENT_KEY_UP, SDL_SCANCODE_A);
    require(!keys.isDown(SDL_SCANCODE_A) && keys.wasPressed(SDL_SCANCODE_A) && keys.wasReleased(SDL_SCANCODE_A),
        "Down/up between updates lost an edge.");
    keys.releaseAll();
    require(!keys.isDown(SDL_SCANCODE_W) && keys.wasReleased(SDL_SCANCODE_W), "Focus-style release stuck a key.");
    keys.clearTransitions();
    require(!keys.wasReleased(SDL_SCANCODE_W), "Release edge was not consumed.");
    send(SDL_EVENT_KEY_DOWN, SDL_SCANCODE_SPACE, true);
    require(!keys.isDown(SDL_SCANCODE_SPACE), "A repeat reacquired an inactive key.");
    for (auto key : {SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_COUNT, static_cast<SDL_Scancode>(-1)}) {
        send(SDL_EVENT_KEY_DOWN, key);
        require(!keys.isDown(key) && !keys.wasPressed(key) && !keys.wasReleased(key), "Invalid scancode.");
    }
}

} // namespace

int main()
{
    try {
        cameraChecks();
        keyboardChecks();
        std::puts("Camera projection/view and keyboard transition checks passed.");
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
