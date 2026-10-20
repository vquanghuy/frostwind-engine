# Project structure

Frostwind contains a reusable engine library and independent example executables in one CMake project. [RFC 0003](rfcs/0003-frostwind-engine-and-examples.md) records this boundary. Only native macOS Apple Silicon has been built and exercised; Windows/Linux SDKs are pending and browser compilation is not configured.

The layout, ownership, and callback descriptions below document the current implementation. The [remaining rendering boundary work](#remaining-rendering-boundary-work) at the end summarizes extensions following the implemented OpenGL extraction. The root [roadmap](../ROADMAP.md) records intended sequencing and future experiments.

For task-based rendering examples, class usage, and lifecycle diagrams, see the [rendering guide](rendering-guide.md). This document describes source organization and ownership.

## Repository layout

Example directories use two-digit prefixes to sort by suggested learning progression. The prefixes apply to source directories; CMake targets, executable names, and runtime output paths retain names such as `frostwind-asteroids`.

```text
engine/
├── CMakeLists.txt
└── src/
    ├── core/Types.h
    ├── app/Application.h, Application.cpp
    ├── platform/
    │   ├── Window.h, Window.cpp
    │   ├── internal/WindowGraphicsSetup.h
    │   ├── internal/WindowSurface.h, WindowSurface.cpp
    │   └── opengl/OpenGLWindowSurface.h, OpenGLWindowSurface.cpp
    └── rendering/
        ├── GraphicsDevice.h, GraphicsDevice.cpp
        ├── GraphicsTypes.h
        ├── RenderFrame.h, RenderFrame.cpp
        ├── Buffer.h, Buffer.cpp
        ├── Shader.h, Shader.cpp
        ├── GraphicsPipeline.h, GraphicsPipeline.cpp
        ├── Texture2D.h, Texture2D.cpp
        ├── Image.h, Image.cpp
        ├── StbImage.cpp
        ├── internal/GraphicsBackend.h, GraphicsBackendFactory.h, ResourceHandle.h
        └── backends/opengl/
            ├── OpenGLBackend.h, OpenGLBackend.cpp
            ├── OpenGLResources.h, OpenGLResources.cpp
            └── OpenGLCommands.cpp
examples/
├── 01-textured-quad/
│   ├── CMakeLists.txt
│   ├── README.md
│   ├── main.cpp
│   ├── QuadDemo.h, QuadDemo.cpp
│   └── assets/
│       ├── LICENSES.md
│       ├── shaders/quad.vert, quad.frag
│       └── textures/crate.png
├── 02-asteroids/               # Minimal window scaffold; scene/gameplay planned
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── README.md
│   └── docs/assets/            # Concepts, references, prototypes, provenance
└── 03-mini-artillery/          # Minimal window scaffold; terrain/gameplay planned
    ├── CMakeLists.txt
    ├── main.cpp
    ├── README.md
    └── docs/assets/            # Concept/reference images and provenance
```

Headers stay beside implementations. CMake explicitly lists source files. `frostwind` exposes `engine/src/` as its include directory, allowing includes such as `<rendering/RenderFrame.h>`; engine types use namespace `frostwind`. Examples link to `frostwind`, while the engine neither includes nor links example code. No separate public-header tree or installation/export package exists yet.

| Component | Responsibility |
| --- | --- |
| `core/Types.h` | Shared primitive aliases; no runtime initialization or ownership |
| `frostwind::Application` | Own the window and graphics device, create the selected application through a factory, coordinate events/frames and FPS pacing, and translate callback exceptions into failures |
| `frostwind::Window` | Own SDL video/window lifetime and report drawable size/state; call the compiled private setup hook before window creation |
| `frostwind::GraphicsDevice` | Own the compiled backend, validate device/resource/frame identity and descriptions, expose limits, and acquire/submit frames |
| `frostwind::RenderFrame` | Borrow the device for one frame; expose target extent/format, clear, and draw; abandon unfinished work on destruction |
| Private `OpenGLBackend` | Initialize the context, GLAD, VSync, and limits; own native resource registries, apply drawing state, retain frame resources, and present |
| Private `OpenGLWindowSurface` | Prepare OpenGL attributes/flags, own the SDL OpenGL context, and access the private window for context creation and swapping |
| Private abstract `WindowSurface` | Define surface initialization and provide protected SDL window access through the window's single graphics friend |
| `frostwind::Buffer` | Own an opaque identity for immutable vertex bytes uploaded through a device |
| `frostwind::Shader` | Own an opaque identity for a compiled vertex or fragment stage; load explicit source/files |
| `frostwind::GraphicsPipeline` | Own validated shader, vertex layout, resource-binding, and drawing-state configuration |
| `frostwind::Texture2D` | Own an opaque RGBA8 texture identity; compose CPU image loading with device upload |
| `frostwind::Image` | Decode PNG/JPEG into bottom-row-first RGBA8 pixels without a graphics context |
| Example `main.cpp` | Supply SDL's global callbacks, application factory, and window configuration |
| `QuadDemo` | Own engine buffer/texture/pipeline resources, select assets, initialize geometry/shaders, handle Escape, and clear/scale/draw the quad each frame |
| `AsteroidsApp`, `MiniArtilleryApp` | Clear the scaffold window and handle Escape; scene/gameplay implementation remains planned |

Camera, audio, input systems, physics, terrain, and further example code are added only as implementation requires them. [Asteroids](../examples/02-asteroids/README.md) and [Mini Artillery](../examples/03-mini-artillery/README.md) each have a separate CMake target and a minimal OpenGL window scaffold alongside their planning documentation/artwork. Scene rendering and gameplay remain planned; their game rules will stay outside the engine.

## Application flow and resource lifetime

An example derives from `frostwind::Application` and implements `initialize()` and `render(RenderFrame&)`, with optional `handleEvent(event)`. Its factory returns `std::unique_ptr<frostwind::Application>`. SDL's named C callbacks remain global in the example entry point and delegate to the engine's static callbacks.

`Application::onInit` creates the derived instance and initializes its window, then graphics device, then example resources. `Window` calls the compiled setup hook after SDL video initialization and before window creation. `GraphicsDevice` creates the private OpenGL backend, whose surface owns the context; the backend loads GLAD and queries device limits before example initialization. The protected `graphics()` accessor supplies that device for resource initialization. Ownership transfers to SDL's opaque state only after success; failed startup cleans up automatically and leaves null state. The factory, startup, event, and frame boundaries handle exceptions so they cannot unwind through SDL. `onQuit` takes ownership back with `unique_ptr`.

SDL owns the loop, delivering events and repeatedly invoking one frame callback. The engine skips minimized windows and acquires a stack-scoped `RenderFrame` through `GraphicsDevice`. The backend reports ready, temporarily unavailable, or failed, with the latest drawable extent for a ready frame. The example borrows that frame and requests clears/draws; continuing callbacks submit/present once. Early success/failure or exceptions abandon without presenting. Unavailable frames receive a budgeted wait, with a 16 ms fallback when software pacing is disabled. Example return values can continue, finish, or fail the application. Quit/close events are handled by the engine; Escape is a demo-specific action. There is no duplicate event polling or blocking application loop. Application-owned frame pacing measures each callback with SDL’s monotonic nanosecond clock and waits only for the unused frame budget after presentation. `ApplicationConfig` contains `window` and `graphics` settings plus `maxFps` (60 by default, 0 disables software pacing). `GraphicsConfig::vsync` defaults to true; VSync may lower the achieved rate below the cap. Minimized windows retain a separate 16 ms idle delay; zero-sized/unavailable targets follow the unavailable-frame budget. Slow frames do not accumulate catch-up work. This is a rendering limit; fixed-step simulation is future work. See [RFC 0004](rfcs/0004-frame-pacing.md).

The virtual application destructor ensures derived example members are destroyed before the base `GraphicsDevice`, then `Window`. The device/backend releases its context through `OpenGLWindowSurface`; `Window` subsequently releases the SDL window and video subsystem. Native handles are private. `Window` grants graphics friendship once to the abstract `WindowSurface` base; derived surfaces use its protected SDL access instead of gaining their own friendship or a public handle getter. Failed window/device initialization cleans up scoped candidates, and a successful second initialization is rejected. Example constructors must not allocate GPU resources before graphics initialization; destructors must tolerate partial startup. SDL's callback framework performs the final SDL_Quit.

## Rendering and assets

Rendering targets OpenGL 3.3 Core; the development Mac supplies a compatible 4.1 context. GLAD 2.0.8 is generated for functions through 3.3 and is private to the engine target. There is one compiled `GraphicsBackend` implementation, `OpenGLBackend`. Examples and public rendering headers contain no native graphics calls or types. Alternate APIs and browser builds remain planned.

`Buffer`, `Shader`, `Texture2D`, and `GraphicsPipeline` are non-copyable scoped owners borrowing their creating device and holding typed, opaque identities. Successful initialization is one-time; failed initialization cleans up and permits retry. Identity values are never reused during a device's lifetime; backend registries reject stale identities. The facade rejects resources from a different device. Uploads consume caller bytes before returning. Native resource records are privately shared by registries, pipeline dependencies, and active frames; public resource owners do not share ownership of the device. OpenGL deletion occurs with the creating context current, and its native deletion semantics protect outstanding GPU work. Shutdown drains work before destroying the context.

`RenderFrame::draw` takes complete bindings: pipeline, vertex buffer, vertex count/first vertex, optional texture, and optional parameter bytes. The initial contract supports non-indexed triangle lists, Float2 attributes, one RGBA8 window target, opaque drawing without depth/culling, and an optional vertex parameter block plus fragment texture at engine slot 0. Pipeline descriptions map shader declaration names to those slots; the backend resolves native locations, verifies active vertex attributes and resource layout, and keeps native lookups private. Parameter size must match the shader block and be a multiple of 16 bytes. Parameter bytes are copied/consumed by each draw. The quad uses six position/UV vertices and a 16-byte scale block. An untextured triangle can use the same draw operation without parameter/texture bindings.

A `RenderFrame` is non-copyable and non-movable. It is acquired once and becomes inactive after submission or scope abandonment; nested acquisition, reuse, drawing after completion, and submitting through another device fail. A failed clear/draw prevents presentation even if its result is ignored. Examples must not retain frame references. OpenGL executes commands immediately, so abandonment skips presentation and releases references without undoing issued commands; future explicit backends must implement their own safe abandonment and retirement.

`Image` owns CPU pixels independently of a graphics device. It inspects PNG/JPEG dimensions against a caller-supplied limit before decoding, checks sizes for overflow, and preserves bottom-left UV orientation through local row conversion. `Texture2D::initializeFromFile` supplies its device's maximum texture dimension, then uploads RGBA8. Nearest filtering, clamp-to-edge sampling, and no mipmaps preserve the existing quad appearance. The private `StbImage.cpp` compiles the pinned decoder once. No static texture-support initialization or public uniform-location API remains. Shader assets still use explicit GLSL source; native shader tooling for other backends is future work.

Each example README introduces its current status and embeds its preferred concept or implemented capture. Planning artwork, older concept versions, sprite prototypes, prompts, and third-party reference attribution live in that example’s `docs/assets/`. They are separate from runtime `assets/` and are not copied beside executables. Shared branding and historical milestone images remain under `docs/assets/`; preserve their captions and bytes. Add runtime asset folders and `assets/LICENSES.md` when assets are actually imported.

Examples select and license their own runtime assets when imported. `QuadDemo` resolves the executable-relative asset directory and passes explicit paths to engine shader/texture initialization. The engine never selects a demo asset. When passed an asset directory, `frostwind_configure_example` copies those assets beside the executable on every build, including asset-only changes. The Asteroids and Mini Artillery scaffolds omit that argument and have no runtime assets. Separate output directories prevent examples' files from colliding.

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

## Remaining rendering boundary work

[RFC 9999](rfcs/9999-multi-backend-rendering-research.md) retains the multi-backend architecture and pipeline comparison as deferred research. The existing OpenGL foundation remains implemented: window/device/surface ownership, scoped frames, resource identities, CPU images, shaders/pipelines, and migration of all three examples. The [README rendering tree](../README.md#rendering-layout) shows the current source organization.

```text
Example application : Application
├── Buffer / Texture2D / GraphicsPipeline owners
└── base Application
    ├── Window
    └── GraphicsDevice
        └── unique_ptr<GraphicsBackend>
            └── OpenGLBackend
                ├── OpenGLWindowSurface : WindowSurface
                └── native resource registries and frame references

Drawing: example → RenderFrame → GraphicsDevice → GraphicsBackend → OpenGL
```

Window ownership uses composition: no `WindowOpenGL`/`WindowMetal` hierarchy exists. Future `MetalBackend` and `VulkanBackend` implementations will implement the private backend contract, with their own private surface integration. Public windows/resources need no per-API subclasses.

The [current learning plan](../ROADMAP.md#current-focus---opengl-scenes) focuses on a small OpenGL 3D scene and then a bounded grass experiment, adding camera/input/math support as needed. Asteroids camera/sprite work remains separately selectable. Keep the implemented boundary provisional and pause speculative portability expansion. Indexed drawing, additional vertex formats, depth targets, passes, and richer bindings are added when needed. Before implementing Metal, settle shared coordinate conventions and specify shader artifacts, device/surface ownership, in-flight work, resize/recreation, and resource retirement in its RFC. A second backend may revise the shared interface and must migrate the existing examples alongside any change.

Only OpenGL is compiled today; backend selection remains a private factory and platform hook with one linked implementation. Add build selection when a second real backend exists. Windows/Linux SDK/runtime verification and browser integration remain pending. Do not create empty future-backend directories or advertise unsupported build choices.
