#include "OrbitCameraController.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include <glm/gtc/constants.hpp>

#include "PerspectiveCamera.h"

namespace frostwind {
namespace {

void applyView(PerspectiveCamera& camera, const OrbitCameraConfig& state)
{
    const glm::vec3 offset{
        state.distance * std::cos(state.pitch) * std::sin(state.yaw),
        state.distance * std::sin(state.pitch),
        state.distance * std::cos(state.pitch) * std::cos(state.yaw),
    };
    camera.setView(state.target + offset, state.target);
}

bool validAxis(f32 axis)
{
    return std::isfinite(axis) && axis >= -1 && axis <= 1;
}

} // namespace

void OrbitCameraController::reset(PerspectiveCamera& camera, const OrbitCameraConfig& config)
{
    if (!std::isfinite(config.target.x) || !std::isfinite(config.target.y) || !std::isfinite(config.target.z) ||
        !std::isfinite(config.yaw) || !std::isfinite(config.pitch) || !std::isfinite(config.distance) ||
        !std::isfinite(config.minDistance) || !std::isfinite(config.maxDistance) ||
        !std::isfinite(config.pitchLimit) || !std::isfinite(config.angularSpeed) || !std::isfinite(config.distanceSpeed) ||
        config.minDistance <= 0 || config.maxDistance < config.minDistance ||
        config.distance < config.minDistance || config.distance > config.maxDistance ||
        config.pitchLimit <= 0 || config.pitchLimit > 1.56f || std::abs(config.pitch) > config.pitchLimit ||
        config.angularSpeed < 0 || config.distanceSpeed < 0) {
        throw std::runtime_error("[OrbitCameraController] Orbit camera requires finite values, positive distance bounds, pitch within a safe limit, and nonnegative speeds.");
    }
    auto candidate = config;
    candidate.yaw = std::remainder(candidate.yaw, glm::two_pi<f32>());
    applyView(camera, candidate);
    state_ = candidate;
}

void OrbitCameraController::update(PerspectiveCamera& camera, const OrbitCameraInput& input, f32 deltaSeconds)
{
    if (!validAxis(input.horizontal) || !validAxis(input.vertical) || !validAxis(input.distance) ||
        !std::isfinite(deltaSeconds) || deltaSeconds < 0) {
        throw std::runtime_error("[OrbitCameraController] Orbit camera requires input axes in [-1, 1] and finite nonnegative elapsed seconds.");
    }
    auto candidate = state_;
    candidate.yaw += input.horizontal * candidate.angularSpeed * deltaSeconds;
    candidate.pitch += input.vertical * candidate.angularSpeed * deltaSeconds;
    candidate.distance += input.distance * candidate.distanceSpeed * deltaSeconds;
    if (!std::isfinite(candidate.yaw) || !std::isfinite(candidate.pitch) || !std::isfinite(candidate.distance)) {
        throw std::runtime_error("[OrbitCameraController] Orbit camera movement is not finite.");
    }
    candidate.yaw = std::remainder(candidate.yaw, glm::two_pi<f32>());
    candidate.pitch = std::clamp(candidate.pitch, -candidate.pitchLimit, candidate.pitchLimit);
    candidate.distance = std::clamp(candidate.distance, candidate.minDistance, candidate.maxDistance);
    applyView(camera, candidate);
    state_ = candidate;
}

} // namespace frostwind
