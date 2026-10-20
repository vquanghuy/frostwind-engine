#pragma once

#include <filesystem>
#include <string_view>

#include <core/Types.h>

struct SDL_GLContextState;

namespace frostwind {

// Owns a linked OpenGL vertex/fragment program. Its creating context must be
// current for operations/destruction and outlive it. No public shader stages.
class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram();
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    // Sources contain GLSL main entry points. One-time after success; retry on
    // failure. Temporary compiled stages are released after linking.
    bool initialize(std::string_view vertexSource, std::string_view fragmentSource);
    bool initializeFromFiles(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
    bool use() const;
    // Cache locations after linking. Missing/inactive uniforms return -1.
    int uniformLocation(const char* name) const;
    // Activate this program before writing its locations. Invalid/missing
    // locations fail rather than silently skipping required parameters.
    bool setInt(int location, int value) const;
    bool setFloat(int location, f32 value) const;
    bool setVec2(int location, f32 x, f32 y) const;
    bool setVec3(int location, f32 x, f32 y, f32 z) const;
    bool setVec4(int location, f32 x, f32 y, f32 z, f32 w) const;
    // Sixteen column-major floats; no GLM dependency or implicit transpose.
    bool setMat4(int location, const f32* values) const;

private:
    bool canWrite(int location) const;
    u32 program_ = 0;
    SDL_GLContextState* context_ = nullptr;
};

} // namespace frostwind
