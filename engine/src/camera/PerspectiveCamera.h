#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <core/Types.h>

namespace frostwind {

// CPU-only right-handed camera: +Y up, looking along view-space -Z.
// Projection uses radians and OpenGL NDC depth [-1, 1].
class PerspectiveCamera {
public:
    // Reject invalid/degenerate values with std::runtime_error, retaining state.
    void setView(const glm::vec3& position, const glm::vec3& target,
        const glm::vec3& up = {0, 1, 0});
    void setPerspective(f32 verticalFov, f32 aspectRatio, f32 nearPlane, f32 farPlane);

    const glm::mat4& view() const { return view_; }
    const glm::mat4& projection() const { return projection_; }
    glm::mat4 viewProjection() const { return projection_ * view_; }
    const glm::vec3& position() const { return position_; }
    const glm::vec3& target() const { return target_; }
    const glm::vec3& up() const { return up_; }

private:
    glm::vec3 position_{0, 0, 0};
    glm::vec3 target_{0, 0, -1};
    glm::vec3 up_{0, 1, 0};
    glm::mat4 view_{1};
    glm::mat4 projection_{1};
};

} // namespace frostwind
