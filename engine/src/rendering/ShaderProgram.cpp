#include "ShaderProgram.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <glad/gl.h>

#include "GraphicsChecks.h"

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

void compile(CompiledShader& shader, GLenum stage, std::string_view source)
{
    if (source.empty() || source.size() > usize(std::numeric_limits<GLint>::max())) {
        throw std::runtime_error("[ShaderProgram] Shader source is empty or exceeds the OpenGL length limit.");
    }
    shader.object = glCreateShader(stage);
    if (!shader.object) {
        throw std::runtime_error("[ShaderProgram] Could not create OpenGL shader stage.");
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
        throw std::runtime_error(std::string(stage == GL_VERTEX_SHADER ? "[ShaderProgram] Vertex" : "[ShaderProgram] Fragment") + " shader compilation failed: " + log.data());
    }
    detail::checkOpenGL("[ShaderProgram] Shader compilation");
}

void readSource(const std::filesystem::path& path, std::string& source)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    const std::streamoff size = file ? file.tellg() : std::streampos(-1);
    if (size <= 0 || size > std::numeric_limits<GLint>::max()) {
        throw std::runtime_error("[ShaderProgram] Shader file is missing, empty, or oversized: " + path.u8string());
    }
    source.resize(size);
    file.seekg(0);
    if (!file.read(source.data(), size)) {
        throw std::runtime_error("[ShaderProgram] Could not read complete shader file: " + path.u8string());
    }
}

} // namespace

ShaderProgram::~ShaderProgram() noexcept
{
    if (program_) {
        glDeleteProgram(program_);
    }
}

void ShaderProgram::initialize(std::string_view vertexSource, std::string_view fragmentSource)
{
    detail::requireOpenGL("[ShaderProgram] Shader program initialization");
    if (program_) {
        throw std::runtime_error("[ShaderProgram] Shader program initialization requires an empty owner and current context.");
    }
    CompiledShader vertex, fragment;
    compile(vertex, GL_VERTEX_SHADER, vertexSource);
    compile(fragment, GL_FRAGMENT_SHADER, fragmentSource);
    ShaderProgram candidate;
    candidate.program_ = glCreateProgram();
    if (!candidate.program_) {
        throw std::runtime_error("[ShaderProgram] Could not create OpenGL shader program.");
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
        throw std::runtime_error(std::string("[ShaderProgram] Shader program linking failed: ") + log.data());
    }
    detail::checkOpenGL("[ShaderProgram] Shader program linking");
    std::swap(program_, candidate.program_);
}

void ShaderProgram::initializeFromFiles(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
{
    std::string vertex, fragment;
    readSource(vertexPath, vertex);
    readSource(fragmentPath, fragment);
    try {
        initialize(vertex, fragment);
    } catch (const std::runtime_error& error) {
        throw std::runtime_error("[ShaderProgram] Shader program from " + vertexPath.u8string() + " and " +
            fragmentPath.u8string() + ": " + error.what());
    }
}

void ShaderProgram::use() const
{
    detail::requireOpenGL("[ShaderProgram] Shader program activation");
    if (!program_) {
        throw std::runtime_error("[ShaderProgram] ShaderProgram must be initialized before use.");
    }
    glUseProgram(program_);
    detail::checkOpenGL("[ShaderProgram] Shader program activation");
}

int ShaderProgram::uniformLocation(const char* name) const
{
    detail::requireOpenGL("[ShaderProgram] Uniform lookup");
    if (!program_) {
        throw std::runtime_error("[ShaderProgram] ShaderProgram must be initialized before use.");
    }
    if (!name || !*name) {
        throw std::runtime_error("[ShaderProgram] Uniform lookup requires a nonempty name.");
    }
    const GLint location = glGetUniformLocation(program_, name);
    detail::checkOpenGL("[ShaderProgram] Uniform lookup");
    if (location < 0) {
        throw std::runtime_error(std::string("[ShaderProgram] Required uniform is missing or inactive: ") + name);
    }
    return location;
}

void ShaderProgram::requireWritable(int location) const
{
    detail::requireOpenGL("[ShaderProgram] Uniform update");
    if (!program_) {
        throw std::runtime_error("[ShaderProgram] ShaderProgram must be initialized before use.");
    }
    if (location < 0) {
        throw std::runtime_error("[ShaderProgram] Uniform update requires a valid location from this program.");
    }
    GLint current = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);
    if (u32(current) != program_) {
        throw std::runtime_error("[ShaderProgram] Activate this shader program before updating its uniforms.");
    }
}

void ShaderProgram::setInt(int location, int value) const
{
    requireWritable(location);
    glUniform1i(location, value);
    detail::checkOpenGL("[ShaderProgram] Integer uniform update");
}

void ShaderProgram::setFloat(int location, f32 value) const
{
    requireWritable(location);
    glUniform1f(location, value);
    detail::checkOpenGL("[ShaderProgram] Float uniform update");
}

void ShaderProgram::setVec2(int location, f32 x, f32 y) const
{
    requireWritable(location);
    glUniform2f(location, x, y);
    detail::checkOpenGL("[ShaderProgram] Vec2 uniform update");
}

void ShaderProgram::setVec3(int location, f32 x, f32 y, f32 z) const
{
    requireWritable(location);
    glUniform3f(location, x, y, z);
    detail::checkOpenGL("[ShaderProgram] Vec3 uniform update");
}

void ShaderProgram::setVec4(int location, f32 x, f32 y, f32 z, f32 w) const
{
    requireWritable(location);
    glUniform4f(location, x, y, z, w);
    detail::checkOpenGL("[ShaderProgram] Vec4 uniform update");
}

void ShaderProgram::setMat4(int location, const f32* values) const
{
    requireWritable(location);
    if (!values) {
        throw std::runtime_error("[ShaderProgram] Matrix uniform update requires sixteen column-major floats.");
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, values);
    detail::checkOpenGL("[ShaderProgram] Matrix uniform update");
}

} // namespace frostwind
