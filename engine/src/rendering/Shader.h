#pragma once

#include <filesystem>
#include <string_view>

#include "GraphicsTypes.h"
#include "internal/ResourceHandle.h"

namespace frostwind {

class GraphicsDevice;

// One shader stage. The creating device must outlive this owner.
class Shader {
public:
    Shader() = default;
    ~Shader();
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Requires an initialized device and nonempty source. Current backend:
    // GLSL vertex/fragment stage with entry point main. Source is consumed
    // before return; one-time after success, with retry after failure.
    bool initialize(GraphicsDevice& device, const ShaderDescription& description, std::string_view source);
    // Reads the whole file; relative paths use the caller's working directory.
    // Missing/empty/oversized files and compile failures return false with logs.
    bool initializeFromFile(GraphicsDevice& device, const ShaderDescription& description, const std::filesystem::path& path);

private:
    friend class GraphicsDevice;
    GraphicsDevice* device_ = nullptr;
    detail::ShaderHandle handle_;
};

} // namespace frostwind
