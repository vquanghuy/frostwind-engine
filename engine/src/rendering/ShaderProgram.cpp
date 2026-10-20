#include "ShaderProgram.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>
#include <glad/gl.h>

#include "OpenGLChecks.h"

namespace frostwind {
namespace {

struct CompiledShader {
    GLuint object = 0;
    ~CompiledShader()
    {
        if (object) {
            glDeleteShader(object);
        }
    }
};

bool compile(CompiledShader& shader, GLenum stage, std::string_view source)
{
    if (source.empty() || source.size() > usize(std::numeric_limits<GLint>::max())) {
        SDL_Log("Shader source is empty or exceeds the OpenGL length limit.");
        return false;
    }
    shader.object = glCreateShader(stage);
    if (!shader.object) {
        SDL_Log("Could not create OpenGL shader stage.");
        return false;
    }
    const char* data = source.data();
    const GLint length = source.size();
    glShaderSource(shader.object, 1, &data, &length);
    glCompileShader(shader.object);
    GLint compiled = GL_FALSE;
    glGetShaderiv(shader.object, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        GLint logLength = 0;
        glGetShaderiv(shader.object, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(std::max(logLength, 1), '\0');
        glGetShaderInfoLog(shader.object, log.size(), nullptr, log.data());
        SDL_Log("%s shader compilation failed: %s", stage == GL_VERTEX_SHADER ? "Vertex" : "Fragment", log.data());
        return false;
    }
    return detail::checkOpenGL("Shader compilation");
}

bool readSource(const std::filesystem::path& path, std::string& source)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    const std::streamoff size = file ? file.tellg() : std::streampos(-1);
    if (size <= 0 || size > std::numeric_limits<GLint>::max()) {
        SDL_Log("Shader file is missing, empty, or oversized: %s", path.u8string().c_str());
        return false;
    }
    source.resize(size);
    file.seekg(0);
    if (!file.read(source.data(), size)) {
        SDL_Log("Could not read complete shader file: %s", path.u8string().c_str());
        return false;
    }
    return true;
}

} // namespace

ShaderProgram::~ShaderProgram()
{
    if (program_ && detail::requireContext(context_, "Shader program destruction")) {
        glDeleteProgram(program_);
    }
}

bool ShaderProgram::initialize(std::string_view vertexSource, std::string_view fragmentSource)
{
    if (program_ || !detail::requireOpenGL("Shader program initialization")) {
        SDL_Log("Shader program initialization requires an empty owner and current context.");
        return false;
    }
    CompiledShader vertex, fragment;
    if (!compile(vertex, GL_VERTEX_SHADER, vertexSource) || !compile(fragment, GL_FRAGMENT_SHADER, fragmentSource)) {
        return false;
    }
    ShaderProgram candidate;
    candidate.context_ = SDL_GL_GetCurrentContext();
    candidate.program_ = glCreateProgram();
    if (!candidate.program_) {
        SDL_Log("Could not create OpenGL shader program.");
        return false;
    }
    glAttachShader(candidate.program_, vertex.object);
    glAttachShader(candidate.program_, fragment.object);
    glLinkProgram(candidate.program_);
    glDetachShader(candidate.program_, vertex.object);
    glDetachShader(candidate.program_, fragment.object);
    GLint linked = GL_FALSE;
    glGetProgramiv(candidate.program_, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        GLint logLength = 0;
        glGetProgramiv(candidate.program_, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(std::max(logLength, 1), '\0');
        glGetProgramInfoLog(candidate.program_, log.size(), nullptr, log.data());
        SDL_Log("Shader program linking failed: %s", log.data());
        return false;
    }
    if (!detail::checkOpenGL("Shader program linking")) {
        return false;
    }
    std::swap(program_, candidate.program_);
    std::swap(context_, candidate.context_);
    return true;
}

bool ShaderProgram::initializeFromFiles(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
{
    if (program_ || !detail::requireOpenGL("Shader file loading")) {
        return false;
    }
    std::string vertex, fragment;
    if (!readSource(vertexPath, vertex) || !readSource(fragmentPath, fragment)) {
        return false;
    }
    if (!initialize(vertex, fragment)) {
        SDL_Log("Shader program failed for %s and %s", vertexPath.u8string().c_str(), fragmentPath.u8string().c_str());
        return false;
    }
    return true;
}

bool ShaderProgram::use() const
{
    if (!program_ || !detail::requireContext(context_, "Shader program activation")) {
        return false;
    }
    glUseProgram(program_);
    return detail::checkOpenGL("Shader program activation");
}

int ShaderProgram::uniformLocation(const char* name) const
{
    if (!program_ || !name || !*name || !detail::requireContext(context_, "Uniform lookup")) {
        return -1;
    }
    const GLint location = glGetUniformLocation(program_, name);
    return detail::checkOpenGL("Uniform lookup") ? location : -1;
}

bool ShaderProgram::canWrite(int location) const
{
    if (!program_ || location < 0 || !detail::requireContext(context_, "Uniform update")) {
        return false;
    }
    GLint current = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);
    if (u32(current) != program_) {
        SDL_Log("Activate this shader program before updating its uniforms.");
        return false;
    }
    return true;
}

bool ShaderProgram::setInt(int location, int value) const
{
    if (!canWrite(location)) {
        return false;
    }
    glUniform1i(location, value);
    return detail::checkOpenGL("Integer uniform update");
}

bool ShaderProgram::setFloat(int location, f32 value) const
{
    if (!canWrite(location)) {
        return false;
    }
    glUniform1f(location, value);
    return detail::checkOpenGL("Float uniform update");
}

bool ShaderProgram::setVec2(int location, f32 x, f32 y) const
{
    if (!canWrite(location)) {
        return false;
    }
    glUniform2f(location, x, y);
    return detail::checkOpenGL("Vec2 uniform update");
}

bool ShaderProgram::setVec3(int location, f32 x, f32 y, f32 z) const
{
    if (!canWrite(location)) {
        return false;
    }
    glUniform3f(location, x, y, z);
    return detail::checkOpenGL("Vec3 uniform update");
}

bool ShaderProgram::setVec4(int location, f32 x, f32 y, f32 z, f32 w) const
{
    if (!canWrite(location)) {
        return false;
    }
    glUniform4f(location, x, y, z, w);
    return detail::checkOpenGL("Vec4 uniform update");
}

bool ShaderProgram::setMat4(int location, const f32* values) const
{
    if (!values || !canWrite(location)) {
        return false;
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, values);
    return detail::checkOpenGL("Matrix uniform update");
}

} // namespace frostwind
