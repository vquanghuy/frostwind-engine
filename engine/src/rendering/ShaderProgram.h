#pragma once

#include <filesystem>
#include <string_view>

#include <core/Types.h>

namespace frostwind {

// Owns a linked OpenGL vertex/fragment program. Its creating context must be
// current for operations/destruction and outlive it. No public shader stages.
// Initialization/use throw std::runtime_error with operation diagnostics.
// Destruction never throws.
class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram() noexcept;
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    // Sources contain GLSL main entry points. One-time after success; retry on
    // failure. Temporary compiled stages are released after linking.
    void initialize(std::string_view vertexSource, std::string_view fragmentSource);
    void initializeFromFiles(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
    void use() const;
    // Cache locations after linking. Missing/inactive required uniforms throw.
    int uniformLocation(const char* name) const;
    // Activate this program before writing its locations. Invalid/missing
    // locations throw rather than silently skipping required parameters.
    void setInt(int location, int value) const;
    void setFloat(int location, f32 value) const;
    void setVec2(int location, f32 x, f32 y) const;
    void setVec3(int location, f32 x, f32 y, f32 z) const;
    void setVec4(int location, f32 x, f32 y, f32 z, f32 w) const;
    // Sixteen column-major floats; no GLM dependency or implicit transpose.
    void setMat4(int location, const f32* values) const;

private:
    void requireWritable(int location) const;
    u32 program_ = 0;
};

} // namespace frostwind
