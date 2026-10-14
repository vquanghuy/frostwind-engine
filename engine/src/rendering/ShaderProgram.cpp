#include "ShaderProgram.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

namespace toybox {

namespace {

bool readShaderFile(const std::filesystem::path& path, std::string& source)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        SDL_Log("Could not open shader file: %s", path.u8string().c_str());
        return false;
    }
    const std::streamoff size = file.tellg();
    if (size <= 0 || size > std::numeric_limits<GLint>::max()) {
        SDL_Log("Shader file is empty, unreadable, or too large: %s", path.u8string().c_str());
        return false;
    }
    source.resize(size);
    file.seekg(0);
    if (!file.read(source.data(), size)) {
        SDL_Log("Could not read shader file: %s", path.u8string().c_str());
        return false;
    }
    return true;
}

// Compilation stages are temporary; only the linked program is retained.
class ShaderStage {
public:
    ShaderStage() = default;
    ~ShaderStage()
    {
        if (shader_) {
            glDeleteShader(shader_);
        }
    }
    ShaderStage(const ShaderStage&) = delete;
    ShaderStage& operator=(const ShaderStage&) = delete;

    bool compile(GLenum type, std::string_view source, const char* label)
    {
        shader_ = glCreateShader(type);
        if (!shader_) {
            SDL_Log("Could not create %s shader.", label);
            return false;
        }
        const char* data = source.data();
        const GLint length = source.size();
        glShaderSource(shader_, 1, &data, &length);
        glCompileShader(shader_);
        GLint success = GL_FALSE;
        glGetShaderiv(shader_, GL_COMPILE_STATUS, &success);
        if (success != GL_TRUE) {
            GLint logLength = 0;
            glGetShaderiv(shader_, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log(std::max(logLength, 1), '\0');
            glGetShaderInfoLog(shader_, logLength, nullptr, log.data());
            SDL_Log("%s shader compilation failed: %s", label, log.data());
            return false;
        }
        return true;
    }

    void attachTo(GLuint program) const { glAttachShader(program, shader_); }
    void detachFrom(GLuint program) const { glDetachShader(program, shader_); }

private:
    GLuint shader_ = 0;
};

} // namespace

ShaderProgram::~ShaderProgram()
{
    if (program_) {
        glDeleteProgram(program_);
    }
}

bool ShaderProgram::initialize(std::string_view vertexSource, std::string_view fragmentSource)
{
    if (program_) {
        SDL_Log("ShaderProgram is already initialized.");
        return false;
    }
    const usize maxLength = std::numeric_limits<GLint>::max();
    if (vertexSource.empty() || fragmentSource.empty() ||
        vertexSource.size() > maxLength || fragmentSource.size() > maxLength) {
        SDL_Log("Shader source must be nonempty and fit OpenGL's source length.");
        return false;
    }

    ShaderStage vertex;
    if (!vertex.compile(GL_VERTEX_SHADER, vertexSource, "Vertex")) {
        return false;
    }
    ShaderStage fragment;
    if (!fragment.compile(GL_FRAGMENT_SHADER, fragmentSource, "Fragment")) {
        return false;
    }

    // Keep ownership scoped until linking succeeds, including exception paths.
    ShaderProgram candidate;
    candidate.program_ = glCreateProgram();
    if (!candidate.program_) {
        SDL_Log("Could not create OpenGL program.");
        return false;
    }
    vertex.attachTo(candidate.program_);
    fragment.attachTo(candidate.program_);
    glLinkProgram(candidate.program_);
    vertex.detachFrom(candidate.program_);
    fragment.detachFrom(candidate.program_);
    GLint success = GL_FALSE;
    glGetProgramiv(candidate.program_, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        GLint length = 0;
        glGetProgramiv(candidate.program_, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(std::max(length, 1), '\0');
        glGetProgramInfoLog(candidate.program_, length, nullptr, log.data());
        SDL_Log("Shader program linking failed: %s", log.data());
        return false;
    }
    program_ = candidate.program_;
    candidate.program_ = 0;
    return true;
}

bool ShaderProgram::initializeFromFiles(const std::filesystem::path& vertexPath,
    const std::filesystem::path& fragmentPath)
{
    if (program_) {
        SDL_Log("ShaderProgram is already initialized.");
        return false;
    }
    std::string vertexSource;
    std::string fragmentSource;
    if (!readShaderFile(vertexPath, vertexSource) || !readShaderFile(fragmentPath, fragmentSource)) {
        return false;
    }
    if (!initialize(vertexSource, fragmentSource)) {
        SDL_Log("Shader program files: vertex=%s; fragment=%s",
            vertexPath.u8string().c_str(), fragmentPath.u8string().c_str());
        return false;
    }
    return true;
}

void ShaderProgram::bind() const
{
    glUseProgram(program_);
}

GLint ShaderProgram::uniformLocation(const char* name) const
{
    return glGetUniformLocation(program_, name);
}

void ShaderProgram::setInt(GLint location, int value) const
{
    glUniform1i(location, value);
}

void ShaderProgram::setVec2(GLint location, f32 x, f32 y) const
{
    glUniform2f(location, x, y);
}

} // namespace toybox
