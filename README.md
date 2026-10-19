![Frostwind](docs/assets/frostwind-logo.png)

An experimental C++ game engine for **2D and 3D games**, developed through practical examples. Planned capabilities include 2D/3D physics integrations and multiple graphics APIs.

> **Status:** A native macOS OpenGL scaffold is implemented: window/context management, application lifecycle callbacks, shader programs with file loading, and RGBA8 textures with PNG/JPEG loading. The textured-quad example exercises these features. Camera, audio, physics, terrain destruction, and additional graphics backends remain planned.

Frostwind grew out of Mini Artillery, a destructible-terrain experiment, and was initially named Toybox. The engine now lives separately from its examples in this repository; existing releases and milestone screenshots retain their original history. See the [changelog](CHANGELOG.md) and [RFC 0003](docs/rfcs/0003-frostwind-engine-and-examples.md).

## Engine and examples

The static `frostwind` library supplies reusable facilities. Examples depend on the engine and own their application behavior and runtime assets; the engine never depends on an example.

Example directories use two-digit prefixes for a suggested learning order: Textured Quad, Asteroids, then Mini Artillery. Each example builds independently; CMake targets and executable names omit the prefixes.

| Example | Status |
| --- | --- |
| [Textured Quad](examples/01-textured-quad/README.md) | Implemented: shader/image-file loading and quad rendering |
| [Asteroids](examples/02-asteroids/README.md) | Runnable window scaffold; 2D camera/input/sprites, lighting, and gameplay planned |
| [Mini Artillery](examples/03-mini-artillery/README.md) | Runnable window scaffold; tanks, projectiles, and destructible terrain planned |

Asteroids and Mini Artillery each have a minimal executable that opens a plain OpenGL window, plus planning READMEs and concept/reference artwork. Their READMEs embed the preferred concepts and link older versions and provenance. Asteroids will own ship/arena rules, bullets, asteroid behavior, and scoring. Mini Artillery will own aiming, weapons, damage, turns, match rules, and game UI. Reusable terrain carving and collision rebuilding can belong to the engine. Other 2D/3D examples will be added as features need them; no gameplay is implemented yet.

## Technical approach

| Area | Choice and status |
| --- | --- |
| Language and build | C++17 and CMake; static engine library, separate example executables |
| Native development | macOS on Apple Silicon; Windows x64 and Linux x86_64 scaffolds remain unverified |
| Graphics | OpenGL 3.3 Core baseline, GLAD 2.0.8; SDL3 window/context integration |
| SDL | 3.4.18 official macOS universal dynamic framework |
| Rendering resources | ShaderProgram with source/file loading; Texture2D with in-memory RGBA8 upload and PNG/JPEG loading |
| Math and debug UI | GLM and Dear ImGui planned |
| Image loading | stb_image 2.28; PNG/JPEG decoding |
| Audio | SDL3 built-in audio planned; SDL3_mixer if richer playback is needed |
| 2D physics and terrain | Box2D 3.x and Clipper2 planned |
| 3D physics | Planned; library selection deferred |
| Rendering architecture | Proposed in draft RFC 0006: concrete SDL Window, public GraphicsDevice facade, private abstract GraphicsBackend; extraction not implemented |
| Additional graphics APIs | Planned: Metal on macOS and Vulkan on Windows/Linux, after extracting the existing OpenGL implementation |
| Browser | Planned Emscripten/WebAssembly and WebGL 2 through its GLES3 interface; not configured |

OpenGL and the current 2D example are the starting point, not limits on Frostwind's scope. Introduce new systems through concrete examples rather than building unused abstractions.

The [engine roadmap](ROADMAP.md) records the intended learning sequence: extract the existing OpenGL backend, develop 2D/3D fundamentals, then explore Metal, Vulkan, and optional advanced rendering. [Draft RFC 0006](docs/rfcs/0006-rendering-backend-boundary.md) proposes the first rendering boundary and migration of the existing examples; those changes remain unimplemented.

## Project structure

### Current layout

This tree shows the implemented repository. The rendering boundary described below remains a proposal.

```text
frostwind-engine/
├── CMakeLists.txt              # Shared project and local dependencies
├── CMakePresets.json
├── cmake/                      # SDK integration and example packaging
├── engine/
│   ├── CMakeLists.txt          # Static frostwind library
│   └── src/
│       ├── core/               # Primitive type aliases
│       ├── app/                # Application lifecycle and frame coordination
│       ├── platform/           # Window and OpenGL context ownership
│       └── rendering/          # ShaderProgram and Texture2D
├── examples/
│   ├── 01-textured-quad/       # Implemented code, CMake target, and runtime assets
│   ├── 02-asteroids/           # Window scaffold, CMake target, and planning artwork
│   └── 03-mini-artillery/      # Window scaffold, CMake target, and planning artwork
├── external/                   # Pinned dependencies and license records
├── docs/                       # Structure, development, conventions, and RFCs
├── ROADMAP.md                  # Learning phases and future experiments
├── CHANGELOG.md
├── LICENSE
└── build/                      # Ignored generated outputs
```

Engine code uses namespace `frostwind`; example code stays outside it. Headers live beside their `.cpp` files. Add camera, input, audio, terrain, physics, further examples, and tests when needed. The [structure guide](docs/project-structure.md) records ownership, callbacks, runtime assets, and platform SDK layout.

### Proposed rendering layout

[Draft RFC 0006](docs/rfcs/0006-rendering-backend-boundary.md) proposes the following engine organization for the first OpenGL extraction. These new files/directories are not implemented yet; create them through the staged migration of all three examples.

```text
engine/src/
├── core/Types.h
├── app/
│   └── Application.h, Application.cpp
├── platform/
│   ├── Window.h, Window.cpp                    # Concrete SDL window owner
│   ├── internal/WindowGraphicsSetup.h          # Private setup-hook declaration
│   └── opengl/
│       └── OpenGLWindowSurface.h, .cpp         # Context and SDL integration
└── rendering/
    ├── GraphicsDevice.h, .cpp                  # Concrete public facade
    ├── GraphicsTypes.h                        # Descriptions and configuration
    ├── RenderFrame.h, .cpp                     # Scoped drawing interface
    ├── Buffer.h, .cpp
    ├── Texture2D.h, .cpp
    ├── Shader.h, .cpp
    ├── GraphicsPipeline.h, .cpp
    ├── Image.h, .cpp                          # CPU image loading
    ├── StbImage.cpp
    ├── internal/
    │   ├── GraphicsBackend.h                  # Private abstract interface
    │   ├── GraphicsBackendFactory.h
    │   └── ResourceHandle.h
    └── backends/opengl/
        ├── OpenGLBackend.h, .cpp              # Concrete backend implementation
        ├── OpenGLResources.cpp
        └── OpenGLCommands.cpp
```

`Application` would own a concrete `Window` and `GraphicsDevice`. The device would own a private `GraphicsBackend` implementation; `OpenGLBackend` would implement it first. `Window` would retain SDL window state, while graphics context ownership and presentation would move behind the device. Examples would draw through `RenderFrame` and engine resource types, without OpenGL calls or native handles.

Later `MetalBackend` and `VulkanBackend` implementations would inherit from the same private backend interface. They would not require per-API public window subclasses. Keep those implementations and their platform integration out of the tree until work begins. The [proposed ownership summary](docs/project-structure.md#proposed-rendering-boundary) and RFC describe startup, resource lifetime, shader bindings, and the migration in more detail.

## Build and run

On an Apple Silicon Mac with Xcode or Command Line Tools and CMake 3.22 or newer:

```sh
cmake --preset native-debug
cmake --build --preset native-debug --parallel
./build/native/bin/frostwind-textured-quad/frostwind-textured-quad
```

The preset builds all three examples. Textured Quad opens a resizable window with a centered quad displaying a generated crate texture. Its assets are resolved relative to the executable, independently of the working directory.

Run either window scaffold separately:

```sh
./build/native/bin/frostwind-asteroids/frostwind-asteroids
./build/native/bin/frostwind-mini-artillery/frostwind-mini-artillery
```

Asteroids and Mini Artillery currently display plain backgrounds. Press Escape or close any example window to exit.

Each example's output directory contains its SDL framework and any runtime assets it uses. The two new scaffolds have no runtime assets; planning artwork is not packaged. Keep each output directory together when moving a build. CMake compiles the engine and GLAD from local source and copies the official SDL3 framework; it does not rebuild SDL3 or download dependencies. See [development instructions](docs/development.md) for individual targets, engine-only builds, IDE setup, and remaining platform checks.

Third-party files remain pinned under `external/`, shared by the engine and examples. Preserve licenses and provenance; see the [dependency policy](external/README.md) and [RFC 0001](docs/rfcs/0001-local-dependencies.md).

## Asset creation tools

Useful tools for preparing example artwork:

| Tool | Use |
| --- | --- |
| [Agent Sprite Forge](https://github.com/0x0funky/agent-sprite-forge) | AI-assisted generation and processing of 2D sprites, sprite sheets, transparent PNG frames, and animations |
| [NormalMap-Online](https://cpetry.github.io/NormalMap-Online/) | Browser-based normal-map generation from height maps, with adjustable settings and a live 3D preview |

These are optional asset-authoring tools. Record runtime asset sources, tools, modifications, and licenses in the owning example's `assets/LICENSES.md`. For normal maps, prefer authored height maps over color-image brightness, which also contains paint colors and baked lighting. Asteroids' normal-map lighting experiment remains planned; see the [prototype workflow in RFC 0005](docs/rfcs/0005-camera-keyboard-and-sprites.md#normal-mapped-sprite-lighting-prototype).

## Documentation and license

Start with the [documentation index](docs/README.md). Follow the [coding conventions](docs/coding-conventions.md), and record significant decisions in [RFCs](docs/rfcs/README.md).

Project code is licensed under the [MIT License](LICENSE). Third-party libraries and assets retain their own licenses. Examples keep asset attribution with their assets; the [textured-quad record](examples/01-textured-quad/assets/LICENSES.md) covers its original shaders. Each planned example records concept/reference provenance under its `docs/assets/`; [shared branding and historical milestone captures](docs/assets/README.md) stay in central documentation.
