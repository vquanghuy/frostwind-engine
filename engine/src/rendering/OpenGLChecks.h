#pragma once

struct SDL_GLContextState;

namespace frostwind::detail {

// Private checks shared by direct OpenGL owners; no resource registry/dispatch.
bool requireOpenGL(const char* operation);
bool requireContext(SDL_GLContextState* owner, const char* operation);
bool checkOpenGL(const char* operation);

} // namespace frostwind::detail
