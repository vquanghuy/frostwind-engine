#include "PerspectiveCamera.h"

#include <cmath>
#include <stdexcept>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>

namespace frostwind {
namespace {

bool finite(const glm::vec3& value)
{
    return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

bool finite(const glm::mat4& value)
{
    for (int column = 0; column < 4; ++column) {
        for (int row = 0; row < 4; ++row) {
            if (!std::isfinite(value[column][row])) {
                return false;
            }
        }
    }
    return true;
}

} // namespace

void PerspectiveCamera::setView(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up)
{
    const auto direction = target - position;
    const f32 directionLength = glm::length(direction);
    const f32 upLength = glm::length(up);
    if (!finite(position) || !finite(target) || !finite(up) ||
        !std::isfinite(directionLength) || !std::isfinite(upLength) ||
        directionLength <= 0 || upLength <= 0) {
        throw std::runtime_error("[PerspectiveCamera] Perspective camera requires finite, distinct eye/target and a nonzero up vector.");
    }
    const auto forward = direction / directionLength;
    const auto normalizedUp = up / upLength;
    if (glm::length(glm::cross(forward, normalizedUp)) < 0.0001f) {
        throw std::runtime_error("[PerspectiveCamera] Perspective camera up vector must not be parallel to its view direction.");
    }
    const auto candidate = glm::lookAtRH(position, target, normalizedUp);
    if (!finite(candidate)) {
        throw std::runtime_error("[PerspectiveCamera] Perspective camera view matrix is not finite.");
    }
    // Inputs can borrow this camera's own pose (for example, swapping eye and
    // target); snapshot both before committing either member.
    const auto nextPosition = position;
    const auto nextTarget = target;
    view_ = candidate;
    position_ = nextPosition;
    target_ = nextTarget;
    up_ = normalizedUp;
}

void PerspectiveCamera::setPerspective(f32 verticalFov, f32 aspectRatio, f32 nearPlane, f32 farPlane)
{
    if (!std::isfinite(verticalFov) || !std::isfinite(aspectRatio) ||
        !std::isfinite(nearPlane) || !std::isfinite(farPlane) ||
        verticalFov <= 0 || verticalFov >= glm::pi<f32>() || aspectRatio <= 0 ||
        nearPlane <= 0 || farPlane <= nearPlane) {
        throw std::runtime_error("[PerspectiveCamera] Perspective camera requires FOV in (0, pi), positive aspect/near, and far > near.");
    }
    const auto candidate = glm::perspectiveRH_NO(verticalFov, aspectRatio, nearPlane, farPlane);
    if (!finite(candidate)) {
        throw std::runtime_error("[PerspectiveCamera] Perspective camera projection matrix is not finite.");
    }
    projection_ = candidate;
}

} // namespace frostwind
