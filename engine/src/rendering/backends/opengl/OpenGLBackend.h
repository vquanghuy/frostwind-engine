#pragma once

#include "../../internal/GraphicsBackend.h"
#include "../../../platform/opengl/OpenGLWindowSurface.h"

namespace frostwind::detail {

class OpenGLBackend final : public GraphicsBackend {
public:
    bool initialize(Window& window, const GraphicsConfig& config) override;
    bool present() override;

private:
    OpenGLWindowSurface surface_;
};

} // namespace frostwind::detail
