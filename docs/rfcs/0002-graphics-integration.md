# RFC 0002: Graphics integration

- **Status:** Implemented
- **Created:** 2026-10-13
- **Supersedes:** None
- **Superseded by:** None

## Problem

The current scaffold draws an SDL window surface. Terrain textures and GPU destruction masks need a programmable graphics pipeline, with a path to the later browser experiment.

## Goals and scope

Establish native OpenGL rendering with a textured quad as the first visible result. Keep the renderer small and prepare for WebGL 2 without implementing the browser build in this milestone. Terrain carving, physics, and sprite batching follow separately.

## Proposed design

Use SDL3 to create a desktop OpenGL 3.3 Core context and present frames. Replace surface drawing with continuous rendering and VSync. Size the viewport from the window's pixel dimensions so resizing and Retina displays work correctly. Report context, loader, and shader failures clearly, and release graphics resources before destroying the context.

Generate GLAD 2 using the [online generator](https://gen.glad.sh/): C/C++, `gl` 3.3 Core, no extensions, and all optional features disabled. Use SDL_GL_GetProcAddress to load functions after the context is current; the built-in GLAD loader is unnecessary. The project owner supplied generated GLAD 2.0.8 files on 2026-10-13; they are now integrated as a local native CMake target. See the [dependency record](../../external/glad/README.mini-artillery.md).

Store only generated headers and source under `external/glad/include/` and `external/glad/src/`, alongside license information and `README.mini-artillery.md`. Record the exact generator version, settings, provenance, and local changes. Compile the generated C source as a local static CMake target for native builds, following [RFC 0001](0001-local-dependencies.md). Do not vendor the full generator or generate/download files during configuration.

Keep shared drawing, textures, and terrain masks in `src/rendering/`; keep context and native/browser integration differences in `src/platform/`. Add modules only as needed. Native builds use GLAD headers; browser builds will use Emscripten's GLES3 interface, which translates compatible calls to WebGL 2. WebGL 2 is based on OpenGL ES 3.0 with additional restrictions, so shared rendering must use features directly supported by WebGL 2. Keep shader logic shared where practical, with desktop GLSL and GLSL ES headers and precision declarations selected per target.

Following the project owner's structure review, `Application` owns `SDLWindow` and `OpenGLContext`, followed by the temporary demo. `SDLWindow` manages SDL video initialization and its private native window. Declaration order ensures demo resources are destroyed before the context, window, and SDL subsystem. Static callbacks in `Application` coordinate lifecycle and frame work; named SDL adapters live in the same implementation file. `main.cpp` only selects SDL callback mode, with SDL driving event delivery and repeated frame callbacks. Project classes use no project namespace. `QuadRenderer` remains a temporary demo; resource wrappers and terrain rendering will be added as needed. The maintained layout and lifetime rules live in [project structure](../project-structure.md).

## Alternatives

- **GLAD 1:** The original generator is usable, but GLAD 2 is the preferred starting point for this new integration. Its headers and initialization API differ; do not mix examples from the two versions.
- **SDL renderer:** Simpler for basic 2D drawing, but direct OpenGL matches the planned shader and framebuffer terrain-mask experiments.
- **A broader graphics abstraction:** Could support more backends, but adds scope before the terrain experiment needs it.

## Risks and limitations

The requested context and actual capabilities need validation on the development Mac. Desktop OpenGL success does not establish browser compatibility. Shader differences, texture/render-target formats, and browser context loss require attention during portability work.

## Validation

Run the native CMake configure and build presets, then launch and inspect the textured quad. Verify resize, Retina viewport sizing, presentation, and clean exit. Capture the visible milestone for the changelog. Browser rendering remains unverified until built with Emscripten and exercised in a browser.

## Implementation plan

1. Import the supplied generated GLAD files and their dependency record.
2. Add the native CMake target, SDL OpenGL context, and resource cleanup.
3. Render a textured quad with minimal shader and texture handling.
4. Validate the native milestone and update setup documentation and the changelog.
5. Follow with terrain-mask rendering and circle erasure; validate browser portability later.

## Open questions

The initial quad uses local coordinates with x right and y up, and a bottom-left UV origin. GLSL 330 shaders are embedded in the renderer for this small native experiment. Terrain/world coordinates and browser shader variants will be decided when those features are implemented.

## Decision

Accepted and implemented for the native scaffold on 2026-10-13 following the project owner’s instruction to proceed. GLAD import, the native Core context, resource cleanup, pixel viewport handling, and textured quad are implemented. Native configuration, build, launch, and visible checkerboard output were verified on Apple M1 Pro with an OpenGL 4.1 context. Interactive resize/Retina and exit checks remain manual. The project owner supplied the full-window milestone capture, embedded in the changelog. Terrain masks and browser rendering remain follow-up work.
