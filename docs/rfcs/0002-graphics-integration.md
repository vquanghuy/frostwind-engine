# RFC 0002: Graphics integration

- **Status:** Implemented
- **Created:** 2026-10-13
- **Supersedes:** None
- **Superseded by:** None

## Problem

Before this RFC was implemented, the scaffold drew an SDL window surface. Terrain textures and GPU destruction masks needed a programmable graphics pipeline, with a path to the later browser experiment.

## Goals and scope

Establish native OpenGL rendering with a textured quad as the first visible result. Keep the renderer small and prepare for WebGL 2 without implementing the browser build in this milestone. Terrain carving, physics, and sprite batching follow separately.

## Proposed design

Use SDL3 to create a desktop OpenGL 3.3 Core context and present frames. Replace surface drawing with continuous rendering and VSync. Size the viewport from the window's pixel dimensions so resizing and Retina displays work correctly. Report context, loader, and shader failures clearly, and release graphics resources before destroying the context.

Generate GLAD 2 using the [online generator](https://gen.glad.sh/): C/C++, `gl` 3.3 Core, no extensions, and all optional features disabled. Use SDL_GL_GetProcAddress to load functions after the context is current; the built-in GLAD loader is unnecessary. The project owner supplied generated GLAD 2.0.8 files on 2026-10-13; they are now integrated as a local native CMake target. See the [dependency record](../../external/glad/README.mini-artillery.md).

Store only generated headers and source under `external/glad/include/` and `external/glad/src/`, alongside license information and `README.mini-artillery.md`. Record the exact generator version, settings, provenance, and local changes. Compile the generated C source as a local static CMake target for native builds, following [RFC 0001](0001-local-dependencies.md). Do not vendor the full generator or generate/download files during configuration.

Keep shared drawing, textures, and terrain masks in `engine/src/rendering/`; keep context and native/browser integration differences in `engine/src/platform/`. Add modules only as needed. Native builds use GLAD headers; browser builds will use Emscripten's GLES3 interface, which translates compatible calls to WebGL 2. WebGL 2 is based on OpenGL ES 3.0 with additional restrictions, so shared rendering must use features directly supported by WebGL 2. Keep shader logic shared where practical, with desktop GLSL and GLSL ES headers and precision declarations selected per target.

Under [RFC 0003](0003-toybox-engine-and-examples.md), engine graphics/resource classes now live in namespace `toybox` and the static engine library. `Window` retains SDL video/window/context, GLAD, VSync, and presentation ownership. The reusable application lifecycle hosts example-provided initialization, events, and rendering without selecting demo assets. `QuadDemo`/`QuadRenderer` live in the textured-quad example, whose derived resources are destroyed before the base application’s window/context. Global SDL adapters belong to the example entry point. The maintained layout and lifetime rules live in [project structure](../project-structure.md).

### Rendering support classes

`ShaderProgram` owns a linked vertex/fragment program. The name distinguishes the drawable program from an individual compiled shader stage. Use `initializeFromFiles(vertexPath, fragmentPath)` for runtime shader assets, while retaining `initialize(vertexSource, fragmentSource)` for generated sources and focused checks. Read both files completely, reject missing/unreadable/empty or oversized input, report file paths on failure, and reuse the same compilation/linking pipeline. Release temporary stages after linking. Expose program binding, uniform-location lookup, and the integer/vec2 setters currently needed by the demo. Callers cache locations and bind the program before setting uniforms. Missing uniforms return `-1`; the renderer decides which uniforms are required. A separate public shader-stage class is unnecessary for now. Relative file paths resolve against the caller’s working directory; the application passes an absolute executable-relative `assets/` directory to the demo, which selects its `shaders/` files. CMake copies shaders beside the executable on every build, including asset-only changes. The caller remains responsible for asset-path resolution; `ShaderProgram` does not locate the executable or choose assets.

`Texture2D` owns a 2D texture and initially accepts tightly packed RGBA8 bytes, with the first row at the bottom. Validate dimensions, byte count, and the device's maximum texture size before uploading. Use nearest filtering, clamp-to-edge, and no mipmaps for the initial checkerboard. The upload temporarily resets and restores pixel-unpack layout and buffer state so client-memory input is interpreted consistently. Binding selects a caller-supplied texture unit; initialization binds unit zero. `initializeFromFile(path)` loads PNG/JPEG through the pinned local stb_image 2.28 decoder. Read via `std::filesystem::path` and a binary stream, rejecting missing, unreadable, empty, incomplete, or encoded input larger than the decoder's signed-int limit. Inspect dimensions against the OpenGL device limit before full decoding. `Application::onInit` sets `stbi_set_flip_vertically_on_load(true)` once during application startup, after the window is initialized and before example assets load. File textures reuse this engine-wide bottom-left convention. Convert supported source channels to RGBA8 and use scoped ownership with `stbi_image_free` for decoded pixels. Reuse the validated RGBA8 upload path and its sampling/unpack-state contract. Log paths and decoding/upload failures. Relative paths resolve against the caller's working directory; examples supply executable-relative paths. PNG/JPEG are the only compiled decoders; HDR/16-bit preservation, sRGB formats, subregion updates, and configurable sampling remain future work.

Both classes have empty constructors, explicit fallible initialization, private handles, automatic cleanup, and disabled copying. Initialization requires a current context and loaded GLAD; destruction requires that context to remain current. Successful initialization is one-time: reject a second initialization instead of replacing a live resource. Failure cleans up temporary resources and permits retry. Add move support when a caller needs ownership transfer. These wrappers deliberately change program/texture bindings; renderers select the required bindings before drawing.

`QuadRenderer` owns these support objects and retains its vertex-array/buffer setup and selects the example-owned generated crate image. It is still a temporary demo; the wrappers can be reused by future terrain rendering.

## Alternatives

- **GLAD 1:** The original generator is usable, but GLAD 2 is the preferred starting point for this new integration. Its headers and initialization API differ; do not mix examples from the two versions.
- **SDL renderer:** Simpler for basic 2D drawing, but direct OpenGL matches the planned shader and framebuffer terrain-mask experiments.
- **A broader graphics abstraction:** Could support more backends, but adds scope before the terrain experiment needs it.

## Risks and limitations

The requested context and actual capabilities need validation on the development Mac. Desktop OpenGL success does not establish browser compatibility. Shader differences, texture/render-target formats, and browser context loss require attention during portability work.

## Validation

Run the native CMake configure and build presets, then launch and inspect the textured quad. Verify resize, Retina viewport sizing, presentation, and clean exit. Use owner-supplied captures for visible milestones in the changelog; do not capture automatically. Browser rendering remains unverified until built with Emscripten and exercised in a browser.

For support classes, exercise missing/empty shader files, paths with spaces, compilation and linking failures followed by successful retry, loading outside the repository working directory, cached uniform updates, invalid texture inputs, RGBA8 upload/readback with non-default unpack state, resource destruction, and a demo frame without OpenGL errors. Screenshots remain supplied manually by the project owner; extracting support classes does not create a new visual milestone.

## Implementation plan

1. Import the supplied generated GLAD files and their dependency record.
2. Add the native CMake target, SDL OpenGL context, and resource cleanup.
3. Render a textured quad with minimal shader and texture handling.
4. Validate the native milestone and update setup documentation and the changelog.
5. Extract reusable shader-program and 2D-texture owners and validate their error paths and demo integration.
6. Add PNG/JPEG file loading and exercise it with an example-owned texture.
7. Follow with terrain-mask rendering and circle erasure; validate browser portability later.

## Open questions

The initial quad uses local coordinates with x right and y up, and a bottom-left UV origin. The demo’s GLSL 330 sources live in `examples/textured-quad/assets/shaders/quad.vert` and `quad.frag` and are loaded from the copied runtime assets. Terrain/world coordinates and browser shader variants will be decided when those features are implemented.

## Decision

Accepted and implemented for the native scaffold on 2026-10-13 following the project owner’s instruction to proceed. GLAD import, the native Core context, resource cleanup, pixel viewport handling, and textured quad are implemented. Native configuration, build, launch, and visible checkerboard output were verified on Apple M1 Pro with an OpenGL 4.1 context. Interactive resize/Retina and exit checks remain manual. The project owner supplied the full-window milestone capture, embedded in the changelog. Terrain masks and browser rendering remain follow-up work.

Extended on 2026-10-13 with implemented `ShaderProgram` and `Texture2D` support classes, following the project owner's request. Native build and resource checks passed, including failure recovery, uniform updates, texture upload/readback, cleanup, and matching demo pixel colors. Shader-file loading is implemented; additional resource types remained deferred at that stage.

Extended on 2026-10-14 with PNG/JPEG file loading through stb_image 2.28, referenced from the owner's earlier LearnOpenGL implementation without copying its project code. The textured-quad example now loads a generated crate texture; the prior checkerboard screenshot remains historical. Native configuration/build and focused checks passed for missing/empty/corrupt/truncated files, oversized dimensions, failed-load retry, PNG/JPEG decoding, grayscale-alpha/RGBA conversion, orientation, Unicode paths, unpack-state preservation, repeated initialization rejection, and GPU cleanup. The checks ran from outside the repository working directory. A new owner-supplied screenshot is pending.
