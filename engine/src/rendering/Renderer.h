#pragma once

#include <core/Types.h>

namespace frostwind {

class VertexArray;

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
    // Clear depth to 1 with writes enabled, independently of scissor state.
    // The window must have requested a depth buffer.
    void clearDepth() const;
    // Establish conventional 3D state: LESS depth comparison and depth writes;
    // back-face culling uses counterclockwise front faces. State persists.
    void setDepthTest(bool enabled) const;
    void setBackFaceCulling(bool enabled) const;
    // Draws the entire configured geometry using stored counts: indexed when
    // an IndexBuffer is associated, sequential otherwise. Requires a nonempty
    // complete triangle list and an active shader program. Borrows the array,
    // validates bounds/context, and restores the previous VAO on failure too.
    void drawTriangles(const VertexArray& vertexArray) const;
};

} // namespace frostwind
