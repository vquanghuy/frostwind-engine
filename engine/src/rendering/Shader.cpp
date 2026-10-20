#include "Shader.h"

#include <fstream>
#include <limits>
#include <string>

#include <SDL3/SDL.h>

#include "GraphicsDevice.h"

namespace frostwind {

Shader::~Shader()
{
    if (device_) {
        device_->release(detail::ResourceKind::Shader, handle_.value);
    }
}

bool Shader::initialize(GraphicsDevice& device, const ShaderDescription& description, std::string_view source)
{
    return device.create(*this, description, source);
}

bool Shader::initializeFromFile(GraphicsDevice& device, const ShaderDescription& description, const std::filesystem::path& path)
{
    if (device_) {
        SDL_Log("Shader is already initialized.");
        return false;
    }
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    const std::streamoff size = file ? file.tellg() : std::streampos(-1);
    if (size <= 0 || size > std::numeric_limits<i32>::max()) {
        SDL_Log("Shader file is missing, empty, unreadable, or too large: %s", path.u8string().c_str());
        return false;
    }
    std::string source(size, '\0');
    file.seekg(0);
    if (!file.read(source.data(), size)) {
        SDL_Log("Could not read shader file: %s", path.u8string().c_str());
        return false;
    }
    if (!initialize(device, description, source)) {
        SDL_Log("Shader file: %s", path.u8string().c_str());
        return false;
    }
    return true;
}

} // namespace frostwind
