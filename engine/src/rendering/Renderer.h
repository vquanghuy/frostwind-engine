#pragma once

#include <core/Types.h>

namespace frostwind {

class VertexBuffer;

struct Color {
    f32 r = 0;
    f32 g = 0;
    f32 b = 0;
    f32 a = 1;
};

// Concrete immediate OpenGL renderer. Owns no GPU resources and needs no
// initialization. Application supplies the same instance to each callback.
// Operations throw std::runtime_error on invalid input/context or GL failures.
class Renderer {
public:
    // Clears the full color target, independently of scissor/color write masks.
    // Requires a current initialized context.
    void clear(Color color) const;
    // Borrows geometry and uses the active shader and texture bindings.
    // Validates a nonempty triangle-list range and the buffer's creating context;
    // restores the previous VAO after drawing. No native handles are exposed.
    void drawTriangles(const VertexBuffer& vertices, u32 vertexCount, u32 firstVertex = 0) const;
};

} // namespace frostwind
