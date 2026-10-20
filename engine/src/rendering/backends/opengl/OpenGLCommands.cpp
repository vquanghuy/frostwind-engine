#include "OpenGLBackend.h"

#include <SDL3/SDL.h>

#include "../../../platform/Window.h"

namespace frostwind::detail {

FrameStatus OpenGLBackend::beginFrame(FrameTarget& target)
{
    if (frameActive_) {
        SDL_Log("OpenGL frame is already active.");
        return FrameStatus::Failed;
    }
    if (!window_->pixelSize(target.width, target.height)) {
        SDL_Log("Window pixel size query failed: %s", SDL_GetError());
        return FrameStatus::Failed;
    }
    if (window_->minimized() || target.width <= 0 || target.height <= 0) {
        return FrameStatus::Unavailable;
    }
    if (!makeCurrent()) {
        return FrameStatus::Failed;
    }
    target.format = ColorFormat::RGBA8;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glViewport(0, 0, target.width, target.height);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_RASTERIZER_DISCARD);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    if (!glSucceeded("Frame acquisition")) {
        return FrameStatus::Failed;
    }
    frameActive_ = true;
    return FrameStatus::Ready;
}

bool OpenGLBackend::clear(Color color)
{
    if (!frameActive_ || !makeCurrent()) {
        return false;
    }
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_RASTERIZER_DISCARD);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
    return glSucceeded("Frame clear");
}

bool OpenGLBackend::draw(PipelineHandle pipeline, BufferHandle vertices, TextureHandle texture,
    const void* parameters, usize parameterBytes, u32 firstVertex, u32 vertexCount)
{
    const auto pipelineIt = pipelines_.find(pipeline.value);
    const auto bufferIt = buffers_.find(vertices.value);
    const auto textureIt = textures_.find(texture.value);
    if (!frameActive_ || pipelineIt == pipelines_.end() || bufferIt == buffers_.end()) {
        SDL_Log("Draw uses an inactive frame or stale pipeline/buffer identity.");
        return false;
    }
    const auto& state = pipelineIt->second;
    const auto& description = state->description;
    const usize availableVertices = bufferIt->second->byteSize / description.vertexStride;
    if (firstVertex > availableVertices || vertexCount > availableVertices - firstVertex ||
        parameterBytes != description.parameters.byteSize ||
        (!description.texture.name.empty() != (texture.value != 0)) ||
        (texture.value && textureIt == textures_.end())) {
        SDL_Log("Draw vertex range, parameter size, or texture binding does not match its pipeline.");
        return false;
    }
    if (!makeCurrent()) {
        return false;
    }
    // Retain before issuing commands, including allocation/exception paths.
    frameReferences_.push_back(state);
    frameReferences_.push_back(bufferIt->second);
    if (texture.value) {
        frameReferences_.push_back(textureIt->second);
    }
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_RASTERIZER_DISCARD);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glUseProgram(state->program);
    glBindVertexArray(state->vao);
    glBindBuffer(GL_ARRAY_BUFFER, bufferIt->second->object);
    for (const auto& attribute : description.attributes) {
        glVertexAttribPointer(attribute.location, 2, GL_FLOAT, GL_FALSE, description.vertexStride,
            reinterpret_cast<const void*>(attribute.offset));
        glEnableVertexAttribArray(attribute.location);
    }
    if (parameterBytes) {
        glBindBuffer(GL_UNIFORM_BUFFER, state->parameterBuffer);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, parameterBytes, parameters);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, state->parameterBuffer);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindSampler(0, 0);
    glBindTexture(GL_TEXTURE_2D, texture.value ? textureIt->second->object : 0);
    glDrawArrays(GL_TRIANGLES, firstVertex, vertexCount);
    glBindVertexArray(0);
    return glSucceeded("Vertex draw");
}

bool OpenGLBackend::submit()
{
    if (!frameActive_ || !makeCurrent()) {
        return false;
    }
    if (!surface_.present()) {
        SDL_Log("OpenGL presentation failed: %s", SDL_GetError());
        return false;
    }
    return true;
}

void OpenGLBackend::abandon() noexcept
{
    // Commands execute immediately in this backend; abandonment skips the swap.
    // GL retains submitted object storage as required for outstanding GPU work.
    if (makeCurrent()) {
        glUseProgram(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        frameReferences_.clear();
    }
    frameActive_ = false;
}

} // namespace frostwind::detail
