# Project structure

The root [README](../README.md#project-structure) summarizes the planned modules. This document records application responsibilities, resource ownership, and dependency packaging. Only the macOS SDK is supplied and validated; Windows and Linux paths describe the prepared integration for later verification.

## Implemented application structure

```text
src/
├── main.cpp                      # SDL callback-mode entry point
├── app/
│   ├── Application.h
│   └── Application.cpp           # Initialization, events, and frame coordination
├── platform/
│   ├── Window.h
│   └── Window.cpp                # SDL video/window, OpenGL context, and presentation
└── rendering/
    ├── QuadRenderer.h
    └── QuadRenderer.cpp          # Temporary checkerboard demo
```

Headers live beside their implementation files. CMake explicitly lists each compiled source. The `app/` directory currently needs only `Application.h` and `Application.cpp`; fixed-step scheduling or configuration helpers will be added when needed.

| Component | Responsibility |
| --- | --- |
| `main.cpp` | Enable SDL's generated entry point and callback mode |
| `Application` | Own the window and demo, handle lifecycle/events, and coordinate one frame |
| `Window` | Own SDL video initialization, the native window and OpenGL context; load GLAD, configure VSync, query size/state, and present frames |
| `QuadRenderer` | Own and draw the temporary demo's shader, texture, vertex array, and buffer |

`Application` owns `Window` and the temporary `QuadRenderer`, in that declaration order. `Window` initializes SDL's video subsystem, configures graphics attributes, creates the SDL window and OpenGL context, loads GLAD, and sets VSync. The demo then creates its graphics resources. Constructors start empty; fallible initialization reports errors, and destructors release resources after partial initialization.

Destruction runs in reverse: demo resources first, then `Window` releases its context, SDL window, and video subsystem. `Window` balances its subsystem initialization with SDL_QuitSubSystem; SDL's callback framework performs the final SDL_Quit. Resource owners disable copying. Native handles remain private to `Window`, with no public handle getter or friend access. Project classes currently use no project namespace.

`main.cpp` only selects SDL callback mode and includes SDL's entry-point header. Static callbacks on `Application` own allocation, event handling, per-frame work, and deletion. SDL requires named C callback entry points; thin adapters live beside their static implementations in `Application.cpp`.

`onInit` creates the application with `std::make_unique` and transfers ownership to SDL's opaque callback state only after initialization succeeds. Failed initialization cleans up automatically and leaves that state null; standard exceptions become SDL initialization failures. `onQuit` takes ownership back with `std::unique_ptr`, which releases the application and its resources on scope exit.

### Main loop

SDL owns the loop. After initialization, it pumps events into `Application::onEvent` and repeatedly calls `Application::onIterate`. That callback executes one frame through `iterate()`: read drawable dimensions, skip minimized/zero-sized windows, draw, and present. Returning success or failure ends the loop and invokes `onQuit`. There is no application-owned `while` loop or manual event polling. Fixed simulation updates will be added within the frame iteration when physics is introduced; browser callback integration remains unverified.

`Window` bundles window and context lifetime for the single-window OpenGL experiment. A separate context abstraction is unnecessary until independent context management is needed. Platform/API-specific code stays in `platform/`; drawing stays in `rendering/`.

The desktop renderer targets OpenGL 3.3 Core. The actual context may be newer: the development Mac supplies OpenGL 4.1. GLAD 2.0.8 was generated for the 3.3 API; its runtime version result reports the actual context, not the generated API limit. Browser rendering will need its own Emscripten integration and validation.

## Planned rendering resources

`QuadRenderer` is a disposable graphics demo, not the foundation of the terrain renderer. It currently owns its raw OpenGL resources. Replace it as terrain rendering is implemented.

Add `Texture`, `ShaderProgram`, `Buffer`, and `Framebuffer` resource wrappers under `src/rendering/` only when needed. Each wrapper owns its graphics handle and releases it automatically; copying is disabled, and moving can transfer ownership when required. Renderers own the resources they use, and all resources must be destroyed before the context. A future `TerrainRenderer` can own its terrain/mask textures, shader, and framebuffer. Shader and image files belong under `assets/`, separate from the C++ wrappers.

Gameplay, terrain geometry, and simulation remain independent of graphics resources. Browser context setup and shader variants will be added during portability work; the current implementation is native only. See [RFC 0002](rfcs/0002-graphics-integration.md) for the graphics decision.

## Platform SDK layout

```text
external/sdl3/
├── LICENSE.txt
├── README.mini-artillery.md
├── macos-universal/                 # Present: official upstream framework
│   └── SDL3.framework/
│       ├── Headers -> Versions/Current/Headers
│       ├── Resources -> Versions/Current/Resources
│       ├── SDL3 -> Versions/Current/SDL3
│       └── Versions/
├── windows-x64/                     # SDK pending; MSVC x64 only
│   ├── include/SDL3/               # Matching public headers
│   ├── lib/SDL3.lib                # DLL import library, not a static library
│   └── bin/SDL3.dll                # Runtime library
└── linux-x64/                       # SDK pending; distribution to be selected
    ├── include/SDL3/               # Matching public headers
    └── lib/
        ├── libSDL3.so.0            # Runtime SONAME; may be a symlink
        └── <versioned binary>      # Required if the SONAME path is a symlink
```

Windows and Linux SDK directories will be created when actual dependency files are imported. Use SDL3 3.4.18 consistently and preserve licenses. Record each package's source, checksum, architecture, ABI requirements, and any local modifications in the dependency record. No upstream source tree is required for normal application builds.

## Header lookup

Application source uses the same include on every target:

```cpp
#include <SDL3/SDL.h>
```

| Platform | Compiler search configuration | Header location |
| --- | --- | --- |
| macOS | Framework search path supplied by the imported framework target | `SDL3.framework/Headers/SDL.h` |
| Windows | Include directory `windows-x64/include` | `include/SDL3/SDL.h` |
| Linux | Include directory `linux-x64/include` | `include/SDL3/SDL.h` |

`cmake/SDL3.cmake` selects the platform SDK and exposes `SDL3::SDL3`. Linking that target supplies the platform's header and library settings. CMake-aware editor tools can use those same settings for code completion.

## Build and runtime layout

| Preset | Build directory | Runtime dependency placement | Status |
| --- | --- | --- | --- |
| `native-debug` | `build/native/` | `Frameworks/SDL3.framework` beside the executable | macOS arm64 validated |
| `windows-debug` | `build/windows/` | `SDL3.dll` beside `Debug/mini-artillery.exe` | SDK pending; unverified |
| `linux-debug` | `build/linux/` | `lib/` beside the executable | SDK pending; unverified |

Presets are visible only on their matching host OS. Windows selects Visual Studio 2022 and x64; Linux selects Unix Makefiles. macOS runtime lookup uses `@loader_path/Frameworks`, and the prepared Linux path uses `$ORIGIN/lib`. Keep the copied runtime dependency directories with the executable.

CMake does not download SDKs or fall back to system SDL installations. Missing dependency files produce an error describing the expected paths. Browser support requires its own WASM-compatible SDL library and is not configured by these native branches.

See [development instructions](development.md) for build commands and deferred platform checks.
