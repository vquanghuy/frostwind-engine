#include "OpenGLBackend.h"

#include <initializer_list>
#include <limits>
#include <memory>

#include <SDL3/SDL.h>

#include "../../../platform/Window.h"
#include "../../internal/GraphicsBackendFactory.h"

namespace frostwind::detail {

std::unique_ptr<GraphicsBackend> createGraphicsBackend()
{
    return std::make_unique<OpenGLBackend>();
}

bool glSucceeded(const char* operation)
{
    bool success = true;
    for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError()) {
        SDL_Log("%s failed: OpenGL error 0x%x", operation, error);
        success = false;
    }
    return success;
}

OpenGLBackend::~OpenGLBackend()
{
    if (loaded_ && makeCurrent()) {
        glFinish();
        abandon();
        pipelines_.clear();
        shaders_.clear();
        textures_.clear();
        buffers_.clear();
    }
}

bool OpenGLBackend::makeCurrent() const
{
    if (!surface_.makeCurrent()) {
        SDL_Log("Making OpenGL context current failed: %s", SDL_GetError());
        return false;
    }
    return true;
}

bool OpenGLBackend::initialize(Window& window, const GraphicsConfig& config)
{
    if (!surface_.initialize(window)) {
        return false;
    }
    const int version = gladLoadGL(SDL_GL_GetProcAddress);
    if (version == 0 || !GLAD_GL_VERSION_3_3) {
        SDL_Log("GLAD could not load the required OpenGL 3.3 Core functions.");
        return false;
    }
    loaded_ = true;
    SDL_Log("OpenGL target 3.3 Core; actual context %s; renderer: %s; detected %d.%d",
        reinterpret_cast<const char*>(glGetString(GL_VERSION)),
        reinterpret_cast<const char*>(glGetString(GL_RENDERER)),
        GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    if (!surface_.setVsync(config.vsync)) {
        SDL_Log("Could not set the requested swap interval: %s. Using application-configured frame pacing.", SDL_GetError());
    }
    for (const auto channel : {SDL_GL_RED_SIZE, SDL_GL_GREEN_SIZE, SDL_GL_BLUE_SIZE, SDL_GL_ALPHA_SIZE}) {
        int bits = 0;
        if (!SDL_GL_GetAttribute(channel, &bits) || bits != 8) {
            SDL_Log("OpenGL window target must have four 8-bit color channels: %s", SDL_GetError());
            return false;
        }
    }
    GLint textureSize = 0, parameterBytes = 0, attributes = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &textureSize);
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &parameterBytes);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &attributes);
    if (!glSucceeded("Graphics limit query") || textureSize <= 0 || parameterBytes <= 0 || attributes <= 0) {
        SDL_Log("OpenGL reported invalid required graphics limits.");
        return false;
    }
    limits_.maxTextureSize = textureSize;
    limits_.maxBufferSize = std::numeric_limits<GLsizeiptr>::max();
    limits_.maxParameterBytes = parameterBytes;
    limits_.maxVertexAttributes = attributes;
    window_ = &window;
    return true;
}

u64 OpenGLBackend::nextIdentity()
{
    if (lastIdentity_ == std::numeric_limits<u64>::max()) {
        SDL_Log("Graphics resource identity space exhausted.");
        return 0;
    }
    return ++lastIdentity_;
}

void OpenGLBackend::release(ResourceKind kind, u64 identity) noexcept
{
    // A failed make-current is diagnostic; keep the registry entry so shutdown
    // can retry cleanup rather than deleting through an unrelated context.
    if (!makeCurrent()) {
        return;
    }
    usize erased = 0;
    switch (kind) {
    case ResourceKind::Buffer: erased = buffers_.erase(identity); break;
    case ResourceKind::Texture: erased = textures_.erase(identity); break;
    case ResourceKind::Shader: erased = shaders_.erase(identity); break;
    case ResourceKind::Pipeline: erased = pipelines_.erase(identity); break;
    }
    if (!erased) {
        SDL_Log("Cannot release a stale graphics resource identity.");
    }
}

} // namespace frostwind::detail
