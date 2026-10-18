# Project structure

Frostwind contains a reusable engine library and independent example executables in one CMake project. [RFC 0003](rfcs/0003-frostwind-engine-and-examples.md) records this boundary. Only native macOS Apple Silicon has been built and exercised; Windows/Linux SDKs are pending and browser compilation is not configured.

## Repository layout

Example directories use two-digit prefixes to sort by suggested learning progression. The prefixes apply to source directories; CMake targets, executable names, and runtime output paths retain names such as `frostwind-asteroids`.

```text
engine/
├── CMakeLists.txt
└── src/
    ├── core/Types.h
    ├── app/Application.h, Application.cpp
    ├── platform/Window.h, Window.cpp
    └── rendering/
        ├── ShaderProgram.h, ShaderProgram.cpp
        ├── Texture2D.h, Texture2D.cpp
        └── StbImage.cpp
examples/
├── 01-textured-quad/
│   ├── CMakeLists.txt
│   ├── README.md
│   ├── main.cpp
│   ├── QuadDemo.h, QuadDemo.cpp
│   ├── QuadRenderer.h, QuadRenderer.cpp
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

Headers stay beside implementations. CMake explicitly lists source files. `frostwind` exposes `engine/src/` as its include directory, allowing includes such as `<rendering/ShaderProgram.h>`; engine types use namespace `frostwind`. Examples link to `frostwind`, while the engine neither includes nor links example code. No separate public-header tree or installation/export package exists yet.

| Component | Responsibility |
| --- | --- |
| `core/Types.h` | Shared primitive aliases; no runtime initialization or ownership |
| `frostwind::Application` | Own the window, create the selected application through a factory, coordinate events/frames and FPS pacing, and translate callback exceptions into failures |
| `frostwind::Window` | Own SDL video/window/context lifetime, GLAD loading, VSync, drawable size/state, and presentation |
| `frostwind::ShaderProgram` | Own a linked vertex/fragment program, load files/source, bind, and set cached uniforms |
| `frostwind::Texture2D` | Own a texture, decode PNG/JPEG files, validate/upload RGBA8 pixels, and bind a texture unit |
| Example `main.cpp` | Supply SDL's global callbacks, application factory, and window configuration |
| `QuadDemo` | Select demo assets, initialize the renderer, handle Escape, and render each frame |
| `QuadRenderer` | Own quad buffers plus engine shader/texture objects; load the example texture and draw |
| `AsteroidsApp`, `MiniArtilleryApp` | Clear the scaffold window and handle Escape; scene/gameplay implementation remains planned |

Camera, audio, input systems, physics, terrain, and further example code are added only as implementation requires them. [Asteroids](../examples/02-asteroids/README.md) and [Mini Artillery](../examples/03-mini-artillery/README.md) each have a separate CMake target and a minimal OpenGL window scaffold alongside their planning documentation/artwork. Scene rendering and gameplay remain planned; their game rules will stay outside the engine.

## Application flow and resource lifetime

An example derives from `frostwind::Application` and implements `initialize()` and `render(width, height)`, with optional `handleEvent(event)`. Its factory returns `std::unique_ptr<frostwind::Application>`. SDL's named C callbacks remain global in the example entry point and delegate to the engine's static callbacks.

`Application::onInit` creates the derived instance, initializes its window/context, calls `Texture2D::initializeSupport()`, and then invokes example initialization. Ownership transfers to SDL's opaque state only after success; failed startup cleans up automatically and leaves null state. The factory, startup, event, and frame boundaries handle exceptions so they cannot unwind through SDL. `onQuit` takes ownership back with `unique_ptr`.

SDL owns the loop, delivering events and repeatedly invoking one frame callback. The engine queries drawable pixels, skips minimized/zero-sized windows, invokes the example's renderer, and presents. Example return values can continue, finish, or fail the application. Quit/close events are handled by the engine; Escape is a demo-specific action. There is no duplicate event polling or blocking application loop. Application-owned frame pacing measures each callback with SDL’s monotonic nanosecond clock and waits only for the unused frame budget after presentation. `ApplicationConfig` contains `window` settings and `maxFps` (60 by default, 0 disables software pacing). `WindowConfig::vsync` defaults to true; VSync may lower the achieved rate below the cap. Minimized/zero-sized windows retain a separate 16 ms idle delay. Slow frames do not accumulate catch-up work. This is a rendering limit; fixed-step simulation is future work. See [RFC 0004](rfcs/0004-frame-pacing.md).

The virtual application destructor ensures derived example members are destroyed before the base `Window`. `Window` then releases the OpenGL context, SDL window, and video subsystem in that order. Native handles are private. Example constructors must not allocate GPU resources before window initialization; destructors must tolerate partial startup. SDL's callback framework performs the final SDL_Quit.

## Rendering and assets

Rendering currently targets OpenGL 3.3 Core; the development Mac supplies a compatible 4.1 context. GLAD 2.0.8 is generated for API functions through 3.3. Alternate graphics APIs and 3D physics are planned, with no backend hierarchy introduced yet.

`ShaderProgram` and `Texture2D` are engine-owned resource types; concrete objects are owned by the renderer that uses them. Initialize/destroy them while their context is current. Their current OpenGL contracts are detailed in [RFC 0002](rfcs/0002-graphics-integration.md#rendering-support-classes). The private `StbImage.cpp` compiles the local PNG/JPEG decoder once. Application startup calls `Texture2D::initializeSupport()` before example initialization. This rendering-owned setup configures bottom-left image orientation and caches the maximum texture size for the single context. Both file loads and raw uploads reuse that limit; application code does not depend on stb_image. Example code chooses texture paths; the engine does not choose assets. Buffer/framebuffer wrappers and additional formats remain deferred.

Each example README introduces its current status and embeds its preferred concept or implemented capture. Planning artwork, older concept versions, sprite prototypes, prompts, and third-party reference attribution live in that example’s `docs/assets/`. They are separate from runtime `assets/` and are not copied beside executables. Shared branding and historical milestone images remain under `docs/assets/`; preserve their captions and bytes. Add runtime asset folders and `assets/LICENSES.md` when assets are actually imported.

Examples select and license their own runtime assets when imported. `QuadDemo` resolves the executable-relative shader directory and passes explicit paths to its renderer. The engine never selects a demo asset. When passed an asset directory, `frostwind_configure_example` copies those assets beside the executable on every build, including asset-only changes. The Asteroids and Mini Artillery scaffolds omit that argument and have no runtime assets. Separate output directories prevent examples' files from colliding.

## CMake and runtime layout

The root project owns shared local dependency setup. `engine/CMakeLists.txt` defines the static `frostwind` target; each example defines its executable. All three build presets select `frostwind-textured-quad`, `frostwind-asteroids`, and `frostwind-mini-artillery`. A build can select one with `--target <target>`. `FROSTWIND_BUILD_EXAMPLES=OFF` builds the library alone. SDL3 and GLAD include/link requirements propagate through the library target. `cmake/FrostwindExample.cmake` handles example warnings, output directories, and runtime packaging.

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
