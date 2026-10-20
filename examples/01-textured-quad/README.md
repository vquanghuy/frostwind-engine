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

The [rendering usage guide](../../docs/rendering-guide.md) walks through this example's resource creation, parameter layout, drawing, and lifetime with diagrams.

## Ownership

`main.cpp` supplies SDL's global callback adapters and creates `QuadDemo`. The example keeps resource initialization, Escape handling, and drawing together:

```text
QuadDemo : Application
├── Buffer
├── Texture2D
├── GraphicsPipeline
├── initialize() → create resources
├── handleEvent() → Escape handling
└── render(frame) → clear, calculate scale, draw quad
```

[QuadDemo.h](QuadDemo.h) declares the resource owners; [QuadDemo.cpp](QuadDemo.cpp) implements the callbacks. The engine's `Application` owns the window and graphics device, acquires a frame, and submits/presents when rendering returns continue. `QuadDemo` creates its resources through `graphics()` and draws six vertices through the borrowed `RenderFrame`. The pipeline retains shader resources after local shader owners leave initialization. Example GPU members are destroyed before the base device/context, then the window. Native graphics calls/types stay inside the engine. See [RFC 9999](../../docs/rfcs/9999-multi-backend-rendering-research.md).

`assets/shaders/quad.vert`, `quad.frag`, and `assets/textures/crate.png` are copied beside the executable on each build. Asset-only edits need a build to refresh those copies. Asset lookup uses SDL_GetBasePath, so the working directory can differ from the repository. Preserve the entire output directory when relocating the executable. Shader and texture attribution is in [assets/LICENSES.md](assets/LICENSES.md).

The shaders target desktop GLSL 330. Vertex scale is supplied through a 16-byte `QuadParameters` block; the fragment texture uses engine slot 0. Native block/sampler locations are resolved inside the backend. PNG/JPEG files are decoded to RGBA8 with a bottom-left UV origin. The orange plate is at the upper left and the arrow points up in the crate image; use them to inspect orientation. Sampling remains nearest/clamp-to-edge without mipmaps. Browser shader variants remain future work. See [RFC 9999](../../docs/rfcs/9999-multi-backend-rendering-research.md) for current resource contracts and [RFC 0003](../../docs/rfcs/0003-frostwind-engine-and-examples.md) for the engine/example boundary.
