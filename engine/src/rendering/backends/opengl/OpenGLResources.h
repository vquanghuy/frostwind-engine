#pragma once

#include <memory>

#include <SDL3/SDL.h>
#include <glad/gl.h>

#include "../../GraphicsTypes.h"

namespace frostwind::detail {

// Shared only inside the backend: registries own live wrapper identities;
// pipelines and active frames retain native records they reference.
struct OpenGLResourceContext {
    OpenGLResourceContext() = default;
    OpenGLResourceContext(const OpenGLResourceContext&) = delete;
    OpenGLResourceContext& operator=(const OpenGLResourceContext&) = delete;
    SDL_GLContext context = SDL_GL_GetCurrentContext();
    bool current() const;
};

struct OpenGLBuffer : OpenGLResourceContext {
    ~OpenGLBuffer();
    GLuint object = 0;
    usize byteSize = 0;
};

struct OpenGLTexture : OpenGLResourceContext {
    ~OpenGLTexture();
    GLuint object = 0;
};

struct OpenGLShader : OpenGLResourceContext {
    ~OpenGLShader();
    GLuint object = 0;
    ShaderStage stage = ShaderStage::Vertex;
};

struct OpenGLPipeline : OpenGLResourceContext {
    ~OpenGLPipeline();
    GLuint program = 0;
    GLuint vao = 0;
    GLuint parameterBuffer = 0;
    PipelineDescription description;
    std::shared_ptr<OpenGLShader> vertex;
    std::shared_ptr<OpenGLShader> fragment;
};

bool glSucceeded(const char* operation);

} // namespace frostwind::detail
