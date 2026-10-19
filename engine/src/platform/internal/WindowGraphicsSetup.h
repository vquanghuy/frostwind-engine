#pragma once

#include <SDL3/SDL_video.h>

namespace frostwind::detail {

// The compiled platform integration provides this hook. Call after SDL video
// initialization and before window creation, on the SDL callback thread.
bool prepareGraphicsWindow(SDL_WindowFlags& flags);

} // namespace frostwind::detail
