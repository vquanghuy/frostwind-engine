#include "OpenGLBackend.h"

#include <algorithm>
#include <string>
#include <vector>

#include <SDL3/SDL.h>

namespace frostwind::detail {

bool OpenGLResourceContext::current() const
{
    if (SDL_GL_GetCurrentContext() != context) {
        SDL_Log("Skipping native resource deletion without its owning context; context destruction will reclaim its storage.");
        return false;
    }
    return true;
}

OpenGLBuffer::~OpenGLBuffer()
{
    if (object && current()) {
        glDeleteBuffers(1, &object);
    }
}

OpenGLTexture::~OpenGLTexture()
{
    if (object && current()) {
        glDeleteTextures(1, &object);
    }
}

OpenGLShader::~OpenGLShader()
{
    if (object && current()) {
        glDeleteShader(object);
    }
}

OpenGLPipeline::~OpenGLPipeline()
{
    if (!current()) {
        return;
    }
    if (parameterBuffer) {
        glDeleteBuffers(1, &parameterBuffer);
    }
    if (vao) {
        glDeleteVertexArrays(1, &vao);
    }
    if (program) {
        glDeleteProgram(program);
    }
}

bool OpenGLBackend::createBuffer(const void* data, usize byteCount, BufferHandle& result)
{
    if (!makeCurrent()) {
        return false;
    }
    auto resource = std::make_shared<OpenGLBuffer>();
    glGenBuffers(1, &resource->object);
    if (!resource->object) {
        SDL_Log("Could not create OpenGL vertex buffer.");
        return false;
    }
    glBindBuffer(GL_ARRAY_BUFFER, resource->object);
    glBufferData(GL_ARRAY_BUFFER, byteCount, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (!glSucceeded("Vertex buffer upload")) {
        return false;
    }
    resource->byteSize = byteCount;
    const u64 identity = nextIdentity();
    if (!identity) {
        return false;
    }
    buffers_.emplace(identity, std::move(resource));
    result = {identity};
    return true;
}

bool OpenGLBackend::createTexture(int width, int height, const byte* pixels, TextureHandle& result)
{
    if (!makeCurrent()) {
        return false;
    }
    auto resource = std::make_shared<OpenGLTexture>();
    glGenTextures(1, &resource->object);
    if (!resource->object) {
        SDL_Log("Could not create OpenGL texture.");
        return false;
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, resource->object);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLint alignment = 0, rowLength = 0, skipPixels = 0, skipRows = 0, unpackBuffer = 0;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowLength);
    glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skipPixels);
    glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skipRows);
    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &unpackBuffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, skipPixels);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, skipRows);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpackBuffer);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (!glSucceeded("Texture upload")) {
        return false;
    }
    const u64 identity = nextIdentity();
    if (!identity) {
        return false;
    }
    textures_.emplace(identity, std::move(resource));
    result = {identity};
    return true;
}

bool OpenGLBackend::createShader(const ShaderDescription& description, std::string_view source, ShaderHandle& result)
{
    if (description.format != ShaderFormat::GLSL || description.entryPoint != "main" ||
        (description.stage != ShaderStage::Vertex && description.stage != ShaderStage::Fragment)) {
        SDL_Log("OpenGL supports GLSL vertex/fragment stages with the main entry point.");
        return false;
    }
    if (!makeCurrent()) {
        return false;
    }
    auto resource = std::make_shared<OpenGLShader>();
    resource->stage = description.stage;
    resource->object = glCreateShader(description.stage == ShaderStage::Vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
    if (!resource->object) {
        SDL_Log("Could not create OpenGL shader stage.");
        return false;
    }
    const char* data = source.data();
    const GLint length = source.size();
    glShaderSource(resource->object, 1, &data, &length);
    glCompileShader(resource->object);
    GLint compiled = GL_FALSE;
    glGetShaderiv(resource->object, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        GLint logLength = 0;
        glGetShaderiv(resource->object, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(std::max(logLength, 1), '\0');
        glGetShaderInfoLog(resource->object, logLength, nullptr, log.data());
        SDL_Log("Shader compilation failed: %s", log.data());
        return false;
    }
    if (!glSucceeded("Shader compilation")) {
        return false;
    }
    const u64 identity = nextIdentity();
    if (!identity) {
        return false;
    }
    shaders_.emplace(identity, std::move(resource));
    result = {identity};
    return true;
}

namespace {

bool validateLayout(GLuint program, const PipelineDescription& description)
{
    GLint attributeCount = 0, nameLength = 0;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &attributeCount);
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &nameLength);
    if (usize(attributeCount) != description.attributes.size()) {
        SDL_Log("Pipeline vertex layout does not match active shader attributes.");
        return false;
    }
    std::vector<char> name(std::max(nameLength, 1));
    for (GLint i = 0; i < attributeCount; ++i) {
        GLint count = 0;
        GLenum type = 0;
        glGetActiveAttrib(program, i, name.size(), nullptr, &count, &type, name.data());
        const GLint location = glGetAttribLocation(program, name.data());
        const auto found = std::find_if(description.attributes.begin(), description.attributes.end(),
            [location](const VertexAttribute& attribute) { return location >= 0 && attribute.location == u32(location); });
        if (found == description.attributes.end() || count != 1 || type != GL_FLOAT_VEC2) {
            SDL_Log("Pipeline shader attribute is missing or is not a single Float2: %s", name.data());
            return false;
        }
    }
    GLint blocks = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &blocks);
    if (blocks != (description.parameters.byteSize == 0 ? 0 : 1)) {
        SDL_Log("Pipeline parameter declarations do not match active shader blocks.");
        return false;
    }
    if (blocks) {
        const GLuint block = glGetUniformBlockIndex(program, description.parameters.name.c_str());
        if (block == GL_INVALID_INDEX) {
            SDL_Log("Pipeline parameter block is missing: %s", description.parameters.name.c_str());
            return false;
        }
        GLint size = 0, vertex = 0, fragment = 0;
        glGetActiveUniformBlockiv(program, block, GL_UNIFORM_BLOCK_DATA_SIZE, &size);
        glGetActiveUniformBlockiv(program, block, GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER, &vertex);
        glGetActiveUniformBlockiv(program, block, GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER, &fragment);
        if (usize(size) != description.parameters.byteSize || vertex != GL_TRUE || fragment != GL_FALSE) {
            SDL_Log("Pipeline parameter block size or vertex-stage visibility does not match its description.");
            return false;
        }
        glUniformBlockBinding(program, block, 0);
    }
    GLint uniforms = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniforms);
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &nameLength);
    name.resize(std::max(nameLength, 1));
    bool samplerFound = false;
    for (GLint i = 0; i < uniforms; ++i) {
        const GLuint index = i;
        GLint block = -1;
        glGetActiveUniformsiv(program, 1, &index, GL_UNIFORM_BLOCK_INDEX, &block);
        if (block >= 0) {
            continue;
        }
        GLint count = 0;
        GLenum type = 0;
        glGetActiveUniform(program, index, name.size(), nullptr, &count, &type, name.data());
        if (description.texture.name != name.data() || type != GL_SAMPLER_2D || count != 1 || samplerFound) {
            SDL_Log("Unsupported or undeclared standalone shader uniform: %s", name.data());
            return false;
        }
        const GLint location = glGetUniformLocation(program, name.data());
        if (location < 0) {
            SDL_Log("Pipeline texture sampler is missing.");
            return false;
        }
        glUniform1i(location, 0);
        samplerFound = true;
    }
    if (samplerFound != !description.texture.name.empty()) {
        SDL_Log("Pipeline texture description does not match the shader sampler.");
        return false;
    }
    return true;
}

} // namespace

bool OpenGLBackend::createPipeline(const PipelineDescription& description, ShaderHandle vertex, ShaderHandle fragment, PipelineHandle& result)
{
    const auto vertexIt = shaders_.find(vertex.value);
    const auto fragmentIt = shaders_.find(fragment.value);
    if (vertexIt == shaders_.end() || fragmentIt == shaders_.end() ||
        vertexIt->second->stage != ShaderStage::Vertex || fragmentIt->second->stage != ShaderStage::Fragment) {
        SDL_Log("Pipeline requires live vertex and fragment shader identities of the correct stages.");
        return false;
    }
    if (!makeCurrent()) {
        return false;
    }
    auto resource = std::make_shared<OpenGLPipeline>();
    resource->description = description;
    resource->vertex = vertexIt->second;
    resource->fragment = fragmentIt->second;
    resource->program = glCreateProgram();
    if (!resource->program) {
        SDL_Log("Could not create OpenGL program.");
        return false;
    }
    glAttachShader(resource->program, resource->vertex->object);
    glAttachShader(resource->program, resource->fragment->object);
    glLinkProgram(resource->program);
    glDetachShader(resource->program, resource->vertex->object);
    glDetachShader(resource->program, resource->fragment->object);
    GLint linked = GL_FALSE;
    glGetProgramiv(resource->program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        GLint logLength = 0;
        glGetProgramiv(resource->program, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(std::max(logLength, 1), '\0');
        glGetProgramInfoLog(resource->program, logLength, nullptr, log.data());
        SDL_Log("Pipeline shader linking failed: %s", log.data());
        return false;
    }
    glUseProgram(resource->program);
    const bool layoutValid = validateLayout(resource->program, description);
    glUseProgram(0);
    if (!layoutValid) {
        return false;
    }
    glGenVertexArrays(1, &resource->vao);
    if (!resource->vao) {
        SDL_Log("Could not create OpenGL vertex array.");
        return false;
    }
    if (description.parameters.byteSize) {
        glGenBuffers(1, &resource->parameterBuffer);
        if (!resource->parameterBuffer) {
            SDL_Log("Could not create OpenGL parameter buffer.");
            return false;
        }
        glBindBuffer(GL_UNIFORM_BUFFER, resource->parameterBuffer);
        glBufferData(GL_UNIFORM_BUFFER, description.parameters.byteSize, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    if (!glSucceeded("Pipeline initialization")) {
        return false;
    }
    const u64 identity = nextIdentity();
    if (!identity) {
        return false;
    }
    pipelines_.emplace(identity, std::move(resource));
    result = {identity};
    return true;
}

} // namespace frostwind::detail
