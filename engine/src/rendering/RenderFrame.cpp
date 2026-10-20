#include "RenderFrame.h"

#include <SDL3/SDL.h>

#include "GraphicsDevice.h"

namespace frostwind {

RenderFrame::~RenderFrame()
{
    if (device_) {
        device_->abandon(*this);
    }
}

bool RenderFrame::clear(Color color)
{
    if (!device_) {
        SDL_Log("Cannot clear an inactive RenderFrame.");
        return false;
    }
    return device_->clear(*this, color);
}

bool RenderFrame::draw(const GraphicsPipeline& pipeline, const Buffer& vertices,
    u32 vertexCount, u32 firstVertex, const Texture2D* texture, const void* parameters, usize parameterBytes)
{
    if (!device_) {
        SDL_Log("Cannot draw with an inactive RenderFrame.");
        return false;
    }
    return device_->draw(*this, pipeline, vertices, vertexCount, firstVertex, texture, parameters, parameterBytes);
}

} // namespace frostwind
