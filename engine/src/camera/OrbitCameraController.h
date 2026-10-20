#pragma once

#include <glm/vec3.hpp>

#include <core/Types.h>

namespace frostwind {

class PerspectiveCamera;

struct OrbitCameraConfig {
    glm::vec3 target{0, 0, 0};
    f32 yaw = 0;
    f32 pitch = 0;
    f32 distance = 3;
    f32 minDistance = 0.1f;
    f32 maxDistance = 100;
    f32 pitchLimit = 1.4f; // In (0, 1.56], staying clear of the +Y-up pole.
    f32 angularSpeed = 1; // Radians per second at full input.
    f32 distanceSpeed = 2; // World units per second at full input.
};

// Signed axes in [-1, 1]. Key bindings and input devices belong to the caller.
struct OrbitCameraInput {
    f32 horizontal = 0;
    f32 vertical = 0;
    f32 distance = 0; // Positive moves farther from the target.
};

// CPU-only +Y-up orbit movement. Composes with a PerspectiveCamera without
// owning it, retaining references, or changing its projection. Yaw 0 faces
// the target from +Z; positive yaw moves the eye toward +X.
class OrbitCameraController {
public:
    // Reset config/pose and apply the view immediately. Angles are radians.
    // Invalid config/input or unrepresentable views throw std::runtime_error
    // without changing the camera or controller state.
    void reset(PerspectiveCamera& camera, const OrbitCameraConfig& config = {});
    void update(PerspectiveCamera& camera, const OrbitCameraInput& input, f32 deltaSeconds);

private:
    OrbitCameraConfig state_;
};

} // namespace frostwind
