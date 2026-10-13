#pragma once

#include <filesystem>
#include <string_view>

#include <glad/gl.h>

// Owns a linked vertex/fragment program. The context must outlive this object.
class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram();
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    bool initialize(std::string_view vertexSource, std::string_view fragmentSource);
    // Relative paths are resolved against the caller's working directory.
    bool initializeFromFiles(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
    void bind() const;
    GLint uniformLocation(const char* name) const;

    // Bind this program before setting its cached uniform locations.
    void setInt(GLint location, int value) const;
    void setVec2(GLint location, float x, float y) const;

private:
    GLuint program_ = 0;
};
