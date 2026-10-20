# Textured Quad

A small Frostwind example that loads a generated wooden-crate PNG and draws it on a textured quad. This is the existing Mini Artillery rendering scaffold extracted into an engine consumer; it contains no artillery gameplay.

![Frostwind quad displaying the loaded crate texture](../../docs/assets/milestones/m004-frostwind-textured-quad.png)

*Captured manually on macOS Apple M1 Pro on 2026-10-14. The generated crate PNG is loaded from the example’s runtime assets. Earlier Mini Artillery and Toybox captures remain in the [changelog](../../CHANGELOG.md).*

## Build and run

From the repository root on macOS Apple Silicon:

```sh
cmake --preset native-debug
cmake --build --preset native-debug --parallel
./build/native/bin/frostwind-textured-quad/frostwind-textured-quad
```

Press Escape or close the window to exit. Resize to check that the quad remains square and centered. Retina, minimize/restore, and interactive exit checks remain manual.

The [rendering usage guide](../../docs/rendering-guide.md) walks through this example's resource creation, shader uniforms, drawing, and lifetime with diagrams.

## Ownership

`main.cpp` supplies SDL's global callback adapters and creates `QuadDemo`. The example keeps resource initialization, Escape handling, and drawing together:

```text
QuadDemo : Application
├── VertexBuffer
├── IndexBuffer
├── VertexArray → borrows buffers
├── Texture2D
├── ShaderProgram
├── initialize() → create resources
├── handleEvent() → Escape handling
└── render(renderer, width, height) → clear, calculate scale, activate, bind, draw
```

[QuadDemo.h](QuadDemo.h) declares the resource owners; [QuadDemo.cpp](QuadDemo.cpp) implements the callbacks. `Application` owns Window and one persistent Renderer, which it passes by reference to each render callback. Window initializes OpenGL/GLAD and presents after a continuing callback. `QuadDemo` loads a linked program, texture, and four-vertex buffer plus six-index buffer, then configures a VertexArray with their layout/associations. It caches uniform locations, activates the program, updates scale, binds the texture, and asks Renderer to draw the borrowed vertices through their indices. VertexBuffer owns bytes, IndexBuffer owns immutable u32 indices, and VertexArray owns the VAO/layout; Renderer draws the configured array. The array is declared after its borrowed buffers and is destroyed before them. Derived GPU members are destroyed before the base Window/context. Native graphics calls stay inside the engine. See [RFC 0006](../../docs/rfcs/0006-opengl-focused-rendering.md).

`assets/shaders/quad.vert`, `quad.frag`, and `assets/textures/crate.png` are copied beside the executable on each build. Asset-only edits need a build to refresh those copies. Asset lookup uses SDL_GetBasePath, so the working directory can differ from the repository. Preserve the entire output directory when relocating the executable. Shader and texture attribution is in [assets/LICENSES.md](assets/LICENSES.md).

The shaders target desktop GLSL 330. Vertex scale uses a regular `vec2 uScale`; `uTexture` selects texture unit 0. There is no padded parameter block or pipeline description. PNG/JPEG files decode to RGBA8 with a bottom-left UV origin. The orange plate is at the upper left and the arrow points up in the crate image; use them to inspect orientation. Sampling remains nearest/clamp-to-edge without mipmaps. Browser shader variants remain future work. [RFC 0003](../../docs/rfcs/0003-frostwind-engine-and-examples.md) records the engine/example boundary.

Rendering and resource calls throw on failure. `Application` catches and logs exceptions at the SDL callback boundary and skips presentation, so `QuadDemo` uses a direct sequence of engine calls without per-operation checks.

The quad uses indices `0, 1, 2, 2, 3, 0`. `drawTriangles(vertexArray_)` draws two triangles from four shared corners using the stored index count. Renderer detects the associated IndexBuffer automatically; no count, offset, or draw-mode argument is required.
