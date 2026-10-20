#pragma once

struct SDL_GLContextState;

namespace frostwind::detail {

// Private checks throw std::runtime_error; SDL callback boundaries report errors.
void requireOpenGL(const char* operation);
void requireContext(SDL_GLContextState* owner, const char* operation);
void checkOpenGL(const char* operation);
// Destructors must never throw, including during initialization failure cleanup.
bool canDestroy(SDL_GLContextState* owner, const char* operation) noexcept;

} // namespace frostwind::detail
