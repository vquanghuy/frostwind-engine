# Project structure

Frostwind contains a reusable C++17 engine library and independent examples in one CMake project. [RFC 0003](rfcs/0003-frostwind-engine-and-examples.md) records that boundary; [RFC 0006](rfcs/0006-opengl-focused-rendering.md) records the current OpenGL-focused design. Only native macOS Apple Silicon has been built and exercised. Windows/Linux SDKs and browser integration remain pending.

The [rendering guide](rendering-guide.md) explains class usage. The root [roadmap](../ROADMAP.md) records learning priorities; multi-backend architecture is preserved separately as [deferred research](rfcs/9999-multi-backend-rendering-research.md).

## Repository layout

Headers live beside implementations. Example prefixes specify suggested learning order; target names and executable output directories remain unnumbered.

```text
engine/
├── CMakeLists.txt
└── src/
    ├── core/Types.h
    ├── app/Application.h, .cpp
    ├── platform/Window.h, .cpp
    └── rendering/
        ├── Texture2D.h, .cpp
        ├── ShaderProgram.h, .cpp
        ├── Renderer.h, .cpp
        ├── VertexBuffer.h, .cpp
        ├── Image.h, .cpp
        ├── OpenGLChecks.h, .cpp       # Private context/error checks
        └── StbImage.cpp
examples/
├── 01-textured-quad/
│   ├── CMakeLists.txt
│   ├── README.md
│   ├── main.cpp
│   ├── QuadDemo.h, .cpp
│   └── assets/
│       ├── LICENSES.md
│       ├── shaders/quad.vert, quad.frag
│       └── textures/crate.png
├── 02-asteroids/                # Window scaffold, README, docs/assets/
└── 03-mini-artillery/           # Window scaffold, README, docs/assets/
```

`frostwind` exports `engine/src/` as its include root, allowing `<rendering/ShaderProgram.h>`. Engine code uses namespace `frostwind`. Examples depend on the engine; the engine never includes example behavior or chooses example assets. No installation/export package exists yet. CMake explicitly lists implementation sources.

| Component | Responsibility |
| --- | --- |
| `Application` | Own Window, coordinate SDL callbacks/rendering/FPS pacing, set the viewport, supply a persistent Renderer, and translate callback exceptions |
| `Window` | Own SDL video/window/context, set context attributes, load GLAD, request VSync, query drawable size, and present |
| `Texture2D` | Own a GL texture, validate/upload RGBA8 pixels, compose CPU image loading, and bind a texture unit |
| `ShaderProgram` | Compile/link vertex and fragment sources, activate the program, find uniforms, and write typed values |
| `VertexBuffer` | Own a VBO/VAO, upload immutable vertices, define float attribute layout, bind it, and report vertex count |
| `Renderer` | Clear the target and submit validated triangle draws using borrowed geometry and active program/textures; own no GPU resources |
| `Image` | Decode PNG/JPEG to bottom-row-first RGBA8 pixels without graphics initialization |
| Private `OpenGLChecks` helpers | Check current context/GLAD and report OpenGL errors; no dispatch or resource registry |
| `core/Types.h` | Shared primitive aliases without runtime initialization |
| Example `main.cpp` | Supply SDL callback adapters, factory, and application configuration |
| `QuadDemo` | Own resources and paths, initialize the quad, handle Escape, calculate scale, and draw |

Native graphics calls live in engine `.cpp` files. GLAD remains a private dependency and public resource headers require no native GL types or handle getters. SDL remains the application/event API. Camera, input systems, audio, physics, and terrain facilities arrive through examples that need them.

## Application flow and resource lifetime

Examples derive from `Application`, implement `initialize()` and `render(Renderer& renderer, int width, int height)`, and optionally override `handleEvent(event)`. Global SDL adapters delegate to the engine's callback methods. `ApplicationConfig` contains `WindowConfig window` and `maxFps`; VSync is `config.window.vsync`.

```text
Factory → derived Application
              ↓
Window initializes SDL → attributes → window → GL context → GLAD → VSync
              ↓
Example initialize() → GPU resources and shader uniforms
              ↓
SDL iteration → current drawable size → current context → viewport
              ↓
Example render(renderer, width, height) → renderer.clear → program/uniforms → texture → renderer.drawTriangles
              ↓ continue
Window presents → Application waits for the remaining frame budget
```

Minimized windows receive a 16 ms idle delay. Zero-size windows skip rendering and use the remaining configured budget, or 16 ms when uncapped. Drawable-query or presentation failures return failure. A rendering callback returning success/failure skips presentation; exceptions become callback failures. OpenGL operations already issued are not rolled back. Callers check operation results and propagate failure; there is no frame object that latches ignored errors.

The default cap is 60 FPS; 0 disables software pacing. VSync defaults to true and is independent of that cap. Each callback measures render/swap time and waits only for the unused budget, without catch-up frames. See [RFC 0004](rfcs/0004-frame-pacing.md). Simulation scheduling is future work.

Resource owners are non-copyable with empty constructors and fallible initialization. Partial failures clean up and permit retry; repeated successful initialization is rejected. Each GPU owner records its creating context and requires it to be current for operations and destruction. Context checks diagnose misuse; they do not switch contexts, extend context lifetime, or provide device-loss recovery. The current application uses one window/context on SDL's callback thread.

Application stores Window before Renderer, so Renderer is destroyed first. Derived example GPU members are destroyed before both. The renderer reference is borrowed for a callback and must not be retained. Application then destroys Window. Window destroys its GL context before its SDL window and video initialization. Failed startup leaves SDL's `appstate` null and cleans completed stages. Resources must not be created in constructors before Window initialization, escape the application/context lifetime, or survive context recreation. No device facade, abstract backend/surface, pipeline, resource registry, or shared frame references remain.

## Rendering and assets

Application calls `glViewport` directly during iteration and passes its persistent Renderer by reference. Renderer owns clear and draw submission; VertexBuffer owns only storage/layout/binding. No per-frame renderer allocation or separate render-command module is needed.

The baseline is OpenGL 3.3 Core with GLAD 2.0.8; the development Mac supplies a compatible 4.1 context. The current window requests RGBA8 without depth/stencil storage. Depth/culling/blending controls, indexed/instanced drawing, camera math, and richer texture options are added only when an example needs them.

`ShaderProgram` owns the linked program; temporary compiled stages are released after linking. Sources/files use explicit GLSL. Cache uniform locations after linking, call `use()`, then set integer, float, vec2/3/4, or column-major mat4 values. Locations belong to the program that returned them; missing/inactive uniforms return -1. Setters reject a missing location or a different active program and report native type/update failures. No universal parameter-block layout or shader translation is imposed.

`VertexBuffer` consumes interleaved vertex bytes and float attributes with explicit location/component count/offset/stride, exposes `bind()` and `vertexCount()`, and contains no draw operation. `Renderer::drawTriangles(vertices, count, first)` validates a nonempty triangle-list range and restores the previous VAO after submission, including failed bindings. Shader input/layout agreement is the caller's responsibility; geometry can be reused with several compatible programs. Draws use the active program and texture bindings. They do not reset every OpenGL state or validate a general pipeline description.

`Texture2D` consumes tightly packed bottom-row-first RGBA8 pixels; its upload restores the prior texture binding, pixel-unpack layout, and unpack-buffer binding. Binding selects a texture unit and the texture's nearest/clamp-to-edge sampling. It has no mipmaps. File loading queries the current context's maximum texture dimension before CPU decoding; there is no static texture-support setup. `Image` performs overflow checks and local row conversion; `StbImage.cpp` compiles the pinned PNG/JPEG decoder once.

Textured Quad uses six position/UV vertices, a `vec2 uScale`, a sampler on unit 0, and the original crate image. The example resolves its executable-relative assets; the engine accepts explicit paths. The build copies assets on each build, including asset-only edits. Each example's runtime assets and licenses stay in its `assets/`; planning concepts/provenance stay in its `docs/assets/` and are not packaged. Preserve historical milestone captures and captions under shared `docs/assets/`.

## CMake and runtime layout

The root project owns shared local dependency setup. `engine/CMakeLists.txt` defines the static `frostwind` target; each example defines its executable. All three build presets select `frostwind-textured-quad`, `frostwind-asteroids`, and `frostwind-mini-artillery`. A build can select one with `--target <target>`. `FROSTWIND_BUILD_EXAMPLES=OFF` builds the library alone. SDL3 headers/link requirements propagate through the library target. GLAD is a private implementation dependency; CMake carries its static archive to executable links without exposing its include directories to examples. `cmake/FrostwindExample.cmake` handles example warnings, output directories, and runtime packaging.

| Preset | Executable path | Status |
| --- | --- | --- |
| `native-debug` | `build/native/bin/<target>/<target>` | macOS arm64 built; runtime validation recorded in development guide |
| `windows-debug` | `build/windows/bin/<target>/Debug/<target>.exe` | SDK pending; unverified |
| `linux-debug` | `build/linux/bin/<target>/<target>` | SDK pending; unverified |

Each executable directory contains `assets/` only when that example uses runtime assets. macOS contains `Frameworks/SDL3.framework` and uses `@loader_path/Frameworks`; Windows copies `SDL3.dll` beside the executable; Linux copies `lib/` and uses `$ORIGIN/lib`. Keep each output directory intact when moving a build. Static library outputs are generated under `build/`, never vendored into `external/`.

## Platform SDKs and header lookup

```text
external/sdl3/
├── LICENSE.txt
├── README.integration.md       # Preserved dependency provenance record
├── macos-universal/SDL3.framework/ # Present; official binary and packaged headers
├── windows-x64/                   # Pending: include/SDL3/, lib/SDL3.lib, bin/SDL3.dll
└── linux-x64/                     # Pending: include/SDL3/, lib/libSDL3.so.0 + targets
```

The framework's headers are supplied through its framework search path; Windows/Linux use matching SDK include directories. Consumers include `<SDL3/SDL.h>` consistently. `cmake/SDL3.cmake` defines `SDL3::SDL3` and refuses missing SDK files instead of fetching or falling back to system packages. `cmake/GLAD.cmake` defines the native static loader target.

Preserve exact versions, licenses, provenance, framework symlinks, and header/binary matching. The official macOS universal SDL3 framework is the explicit dynamic-linking exception. Browser builds will need a separate WASM-compatible SDL3 library and GLES3 integration. See [dependency policy](../external/README.md) and [development](development.md).

## Remaining rendering work

Follow the [current roadmap](../ROADMAP.md#current-focus---opengl-scenes): a small OpenGL 3D scene, then a bounded grass experiment. Add vertex/index formats, depth storage/testing, camera matrices, lighting, and instancing as concrete examples require them. Keep resource responsibilities direct and context ownership explicit. Multi-backend compatibility is not a current design requirement.

The prior experiment is preserved at commit `e9cf616` on `archive/multi-backend-experiment`, with its analysis in RFC 9999. Windows/Linux SDK/runtime checks and browser integration remain pending. Do not create future backend hierarchies or advertise unsupported build choices.
