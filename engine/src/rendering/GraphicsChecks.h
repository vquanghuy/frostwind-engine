#pragma once

namespace frostwind::detail {

// Private checks throw std::runtime_error; SDL callback boundaries report errors.
void requireOpenGL(const char* operation);
void checkOpenGL(const char* operation);

} // namespace frostwind::detail
