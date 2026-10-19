#pragma once

#include <memory>

namespace frostwind::detail {

class GraphicsBackend;

// Defined by the single implementation linked into the engine.
std::unique_ptr<GraphicsBackend> createGraphicsBackend();

} // namespace frostwind::detail
