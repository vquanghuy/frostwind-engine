#include <camera/OrbitCameraController.h>
#include <camera/PerspectiveCamera.h>

#include <cmath>
#include <cstdio>
#include <exception>
#include <limits>
#include <stdexcept>

#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>

namespace {

void require(bool condition, const char* message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

bool near(const glm::vec3& a, const glm::vec3& b)
{
    return glm::length(a - b) < 0.0001f;
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
    require(rejected, "Invalid orbit input/config was accepted.");
}

void checks()
{
    frostwind::PerspectiveCamera camera;
    camera.setPerspective(1, 2, 0.1f, 100);
    const auto projection = camera.projection();
    frostwind::OrbitCameraController controller;
    frostwind::OrbitCameraConfig config;
    config.distance = 4;
    config.minDistance = 1.5f;
    config.maxDistance = 12;
    controller.reset(camera, config);
    require(near(camera.position(), {0, 0, 4}) && near(camera.target(), {0, 0, 0}), "Default orbit pose.");
    controller.update(camera, {1, 0, 0}, glm::half_pi<float>());
    require(near(camera.position(), {4, 0, 0}), "Quarter orbit direction/radian units.");
    controller.reset(camera, config);
    controller.update(camera, {0, 0, -1}, 100);
    require(near(camera.position(), {0, 0, 1.5f}), "Minimum distance clamp.");
    controller.update(camera, {0, 0, 1}, 100);
    require(near(camera.position(), {0, 0, 12}), "Maximum distance clamp.");
    controller.update(camera, {0, 1, 0}, 100);
    require(near(camera.position(), {0, 12 * std::sin(1.4f), 12 * std::cos(1.4f)}), "Upper pitch clamp.");
    controller.update(camera, {0, -1, 0}, 100);
    require(near(camera.position(), {0, -12 * std::sin(1.4f), 12 * std::cos(1.4f)}), "Lower pitch clamp.");
    require(camera.projection() == projection, "Movement changed the camera projection.");

    config.target = {10, -2, 8};
    controller.reset(camera, config);
    require(near(camera.position(), {10, -2, 12}) && near(camera.target(), config.target), "Orbit target translation.");
    const auto stationary = camera.view();
    controller.update(camera, {1, 1, 1}, 0);
    require(camera.view() == stationary, "Zero delta moved the camera.");
    controller.update(camera, {0, 0, 0}, 10);
    require(camera.view() == stationary, "Zero input moved the camera.");
    controller.update(camera, {1, 0, 0}, glm::two_pi<float>() * 100);
    require(near(camera.position(), {10, -2, 12}), "Full-turn wrapping changed the pose.");

    config.target = {0, 0, 0};
    config.yaw = 0.6f;
    config.pitch = 0.35f;
    for (int fps : {30, 60, 120}) {
        controller.reset(camera, config);
        const float seconds = 1.0f / fps;
        for (int i = 0; i < fps; ++i) {
            controller.update(camera, {0.4f, 0.2f, 0.25f}, seconds);
        }
        const glm::vec3 expected{
            4.5f * std::cos(0.55f) * std::sin(1.0f),
            4.5f * std::sin(0.55f),
            4.5f * std::cos(0.55f) * std::cos(1.0f),
        };
        require(near(camera.position(), expected), "Orbit speed depends on update frequency.");
    }

    controller.reset(camera, config);
    const auto savedView = camera.view();
    const auto savedPosition = camera.position();
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float infinity = std::numeric_limits<float>::infinity();
    rejects([&] { controller.update(camera, {nan, 0, 0}, 1); });
    rejects([&] { controller.update(camera, {2, 0, 0}, 1); });
    rejects([&] { controller.update(camera, {0, 0, 0}, -1); });
    rejects([&] { controller.update(camera, {0, 0, 0}, infinity); });
    rejects([&] { controller.update(camera, {0, 0, 1}, std::numeric_limits<float>::max()); });
    for (int invalid = 0; invalid < 8; ++invalid) {
        auto bad = config;
        if (invalid == 0) { bad.pitchLimit = glm::half_pi<float>(); }
        if (invalid == 1) { bad.distance = 0; }
        if (invalid == 2) { bad.minDistance = 0; }
        if (invalid == 3) { bad.maxDistance = 1; }
        if (invalid == 4) { bad.target.x = nan; }
        if (invalid == 5) { bad.angularSpeed = -1; }
        if (invalid == 6) { bad.pitch = 2; }
        if (invalid == 7) { bad.target = {1e30f, 1e30f, 1e30f}; }
        rejects([&] { controller.reset(camera, bad); });
    }
    require(camera.view() == savedView && camera.position() == savedPosition, "Invalid operation changed the camera pose.");
    controller.update(camera, {}, 0);
    require(camera.view() == savedView, "Invalid operation changed the controller state.");

    camera.setView({0, 0, 3}, {1, 2, 0}, {0, 2, 0});
    require(near(camera.up(), {0, 1, 0}), "Stored up vector must be normalized.");
    const auto oldPosition = camera.position();
    const auto oldTarget = camera.target();
    camera.setView(camera.target(), camera.position());
    require(camera.position() == oldTarget && camera.target() == oldPosition, "Aliased pose setters corrupted pose state.");
}

} // namespace

int main()
{
    try {
        checks();
        std::puts("Orbit pose, limits, reset, timing, projection independence, and invalid-state checks passed.");
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
