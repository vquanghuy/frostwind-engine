# Textured Quad

A small Toybox example that draws a procedural green/teal checkerboard on a textured quad. This is the existing Mini Artillery rendering scaffold extracted into an engine consumer; it contains no artillery gameplay.

![Native OpenGL checkerboard quad](../../docs/assets/milestones/m002-opengl-quad.png)

*Captured manually on macOS Apple M1 Pro on 2026-10-13, before the Toybox rename. The window retains its original Mini Artillery title.*

## Build and run

From the repository root on macOS Apple Silicon:

```sh
cmake --preset native-debug
cmake --build --preset native-debug --parallel
./build/native/bin/toybox-textured-quad/toybox-textured-quad
```

Press Escape or close the window to exit. Resize to check that the quad remains square and centered. Retina, minimize/restore, and interactive exit checks remain manual.

## Ownership

`main.cpp` supplies SDL's required global callback adapters and creates `QuadDemo`. The engine's `toybox::Application` owns the window and coordinates events/frames. `QuadDemo` owns `QuadRenderer`, chooses the shader assets, handles Escape, and supplies rendering behavior. Example graphics resources are destroyed before the base application's window/context.

`assets/shaders/quad.vert` and `quad.frag` are copied beside the executable on each build. Shader-only edits need a build to refresh those copies. Asset lookup uses SDL_GetBasePath, so the working directory can differ from the repository. Preserve the entire output directory when relocating the executable. Shader attribution is in [assets/LICENSES.md](assets/LICENSES.md).

The shaders target desktop GLSL 330. Browser shader variants and image-file decoding remain future work. See [RFC 0002](../../docs/rfcs/0002-graphics-integration.md) for resource contracts and [RFC 0003](../../docs/rfcs/0003-toybox-engine-and-examples.md) for the engine/example boundary.
