# RFC 0006: Rendering backend boundary

- **Status:** Draft
- **Created:** 2026-10-18
- **Supersedes:** None
- **Superseded by:** None

## Problem

Frostwind currently has a working OpenGL 3.3 Core implementation, rather than an API-independent renderer. `Window` owns the SDL window, OpenGL context, GLAD loading, VSync, and presentation. `Application` initializes OpenGL texture support and presents through the window. `ShaderProgram` exposes OpenGL uniform locations, while `QuadRenderer` and both window scaffolds issue OpenGL calls directly.

The learning direction is to start with this implementation, establish a small rendering boundary, and later explore Metal on macOS and Vulkan on Windows/Linux. Building a complete abstraction for those APIs before using them would require speculative interfaces. Leaving OpenGL details throughout the examples would make later migrations larger.

This proposal evolves the graphics ownership in [RFC 0002](0002-graphics-integration.md), preserves the engine/example boundary in [RFC 0003](0003-frostwind-engine-and-examples.md), and supports the sequence in the [engine roadmap](../../ROADMAP.md). It does not change the implemented architecture until code lands.

## Goals and scope

- Make OpenGL the first concrete backend behind a small engine rendering interface.
- Separate SDL window ownership from graphics context/device ownership and presentation.
- Migrate all three existing examples together, retaining their purpose, assets, controls, and visible behavior.
- Keep API types and calls inside engine implementation files and private integration headers.
- Keep CPU image data, geometry, camera math, and future simulation independent of a graphics context.
- Leave the interface open to revision when a second backend demonstrates new requirements.

The initial implementation does not add Metal, Vulkan, runtime backend switching, a render graph, an editor, a general material system, ray tracing, mesh shaders, or voxel gameplay. Browser compilation remains pending. No empty backend directories or unsupported configuration choices are introduced.

## Proposed design

### Dependency flow and ownership

```text
Examples: Textured Quad / Asteroids / Mini Artillery
                         ↓
             Frostwind rendering interface
                         ↓
                  OpenGL backend

Later implementations of the same boundary:
    Metal on macOS
    Vulkan on Windows/Linux
```

Examples continue to derive from `frostwind::Application`, select their own assets, and supply SDL callback adapters. Their renderers own engine resource objects and describe drawing operations. They do not include GLAD or backend headers. Engine rendering code never selects example assets or includes example code.

| Component | Proposed responsibility |
| --- | --- |
| `Application` | Own the window and graphics device; coordinate startup, events, frame acquisition, submission/presentation, and existing FPS pacing |
| `Window` | Own SDL video/window lifetime, drawable size, and window state; privately cooperate with graphics integration |
| `GraphicsDevice` | Own the selected graphics implementation, context/device, resource creation support, limits, VSync policy, and presentation |
| `RenderFrame` | Provide scoped access to the current render target and recording of clears, bindings, and draws |
| Resource owners | Own engine buffers, textures, shader stages, and graphics pipelines with private native representations |
| CPU image loader | Decode PNG/JPEG and validate pixel data without querying OpenGL |
| Backend implementation | Translate engine descriptions to API operations and enforce native lifetime/state rules |

`Window` must be created with flags appropriate to the selected implementation. Backend-specific window/surface integration stays private to `platform/` and rendering internals; no public native-window or native-GPU handle getter is added. `GraphicsDevice` owns the graphics side of that integration, and its lifetime ends before the SDL window.

### Intended source layout

This is a design sketch, not a request to create every file immediately. Keep headers beside implementations and add types only when the quad migration needs their responsibility.

```text
engine/src/
├── core/Types.h
├── app/
│   └── Application.h, Application.cpp
├── platform/
│   └── Window.h, Window.cpp
└── rendering/
    ├── GraphicsDevice.h, GraphicsDevice.cpp
    ├── RenderFrame.h, RenderFrame.cpp
    ├── Buffer.h, Buffer.cpp
    ├── Texture2D.h, Texture2D.cpp
    ├── Shader.h, Shader.cpp
    ├── GraphicsPipeline.h, GraphicsPipeline.cpp
    ├── Image.h, Image.cpp
    ├── StbImage.cpp
    └── backends/
        └── opengl/
            ├── OpenGLDevice.h, OpenGLDevice.cpp
            ├── OpenGLResources.h, OpenGLResources.cpp
            └── OpenGLCommands.h, OpenGLCommands.cpp
```

Use namespace `frostwind` and the existing primitive aliases. Public rendering headers contain engine descriptions and opaque resource ownership, without `GLuint`, `GLint`, `Vk*`, or Objective-C types. Private backend headers are implementation details even though the current include root remains `engine/src/`.

Compile one implementation per build initially. Opaque implementations or another small private dispatch mechanism are sufficient; do not introduce a plugin registry or a virtual class for every API function. Metal/Vulkan source and CMake helpers follow when their implementations begin. Final filenames and dispatch details should follow the first working extraction.

### Resource and drawing model

The quad establishes the initial requirements: a vertex buffer, a vertex layout, vertex/fragment shader stages, a texture with defined sampling, a graphics pipeline, a small per-draw parameter block, a clear operation, and a draw operation.

Describe relevant state explicitly: topology, vertex inputs, resource bindings, color/depth formats, blending, depth behavior, and culling. Examples should not depend on previously bound OpenGL state. A graphics pipeline describes the state a draw needs; the OpenGL implementation may apply that state when drawing without pretending OpenGL has a native pipeline object equivalent to other APIs.

Replace public uniform-location lookup and texture-unit manipulation with engine parameter/resource bindings. Define the minimum layout needed by the quad, including CPU/shader packing and when parameter bytes are copied. Do not expose a renamed version of every `gl*` function. Shader stages and pipeline state replace the existing `ShaderProgram` contract as examples migrate.

`Texture2D` retains scoped GPU ownership, while decoding moves behind an independent CPU image loader. Compile the pinned stb_image implementation once and keep decoder configuration private. Device limits belong to the graphics device rather than a static OpenGL texture-support initializer. Preserve missing/corrupt/oversized-file diagnostics and overflow-safe pixel-size validation, including dimension checks before full decoding against caller-supplied limits.

For the first OpenGL extraction, preserve the current bottom-left image/UV convention, RGBA8 upload behavior, nearest filtering, and clamp-to-edge sampling. Document engine coordinate, depth, and image conventions explicitly before another backend is implemented. Any conversion belongs at a defined boundary, with matching shader changes and validation; an OpenGL-specific coordinate choice must not silently become a requirement of every native API.

Examples still own shader sources and their license records. Retain GLSL source/file loading for the OpenGL implementation. A later shader artifact pipeline may compile backend-specific assets into `build/` and package them beside each example. Do not assume one shader binary or identical shader stages will work on every API, and do not choose new compiler dependencies during this extraction.

### Application flow and lifetime

Preserve SDL's callback-driven loop and the exception handling in the C callback adapters. The intended startup order is window, graphics device, then derived-example initialization. Derived examples receive protected access to engine rendering operations, without native handles.

The frame sequence becomes:

1. Query drawable/window state and retain the minimized/zero-size idle behavior.
2. Acquire a frame with a ready, temporarily unavailable, or failed result.
3. Pass a ready frame and its drawable extent to the example's rendering callback.
4. Submit/present a completed frame, or safely abandon it if rendering fails, exits, or throws.
5. Apply the existing application frame budget after submission/presentation.

An unavailable frame must not cause an unpaced busy loop. Resize and presentation-target recreation are graphics responsibilities coordinated with current window dimensions. The frame's actual target extent is authoritative for viewport-dependent rendering. Keep [RFC 0004](0004-frame-pacing.md)'s independent software cap and VSync behavior; move presentation settings to graphics configuration when their owner changes.

Declare the base application's window before its graphics device so destruction releases the graphics device first. Derived example resources are destroyed before both. Resource owners must tolerate partial initialization and have non-throwing cleanup.

Later explicit APIs also need protection against GPU work still using a resource after its C++ owner is destroyed. The backend must retire native resources only after the relevant work completes, and drain pending work before destroying its device and presentation objects. Waiting in the base application destructor alone is insufficient because derived resource members have already been destroyed. Frame abandonment and deferred retirement are interface constraints; implement only the machinery required by the current backend, then validate asynchronous handling with the second backend.

### Existing examples and incremental migration

| Existing code | Migration |
| --- | --- |
| `Window` owns OpenGL context/loading/presentation | Move graphics ownership to the device and private integration code |
| `Application` calls `Texture2D::initializeSupport()` and `Window::present()` | Initialize graphics support through the device and use its frame lifecycle |
| `QuadRenderer` owns VAO/VBO and OpenGL uniform locations | Own engine resources and submit the same geometry through the shared interface |
| Asteroids and Mini Artillery call `glClear` | Request their existing background clears through a frame |
| Examples include GLAD | Remove those includes and consume the engine rendering interface |

Update affected examples in the same completed change as each interface change. Preserve source directory numbering, target/executable names, executable-relative asset selection, framework packaging, Escape/close behavior, and historical captures. There is one current example structure; examples do not remain indefinitely on an abandoned interface. Temporary implementation scaffolding is removed once the replacement passes validation.

Retain OpenGL as the only implemented backend throughout this extraction. Its GLAD dependency becomes an implementation dependency; verify the include/link propagation required by the static library without exposing GLAD through public rendering headers. Preserve pinned versions and local dependency records under [RFC 0001](0001-local-dependencies.md).

### Relationship to other planned work

[RFC 0005](0005-camera-keyboard-and-sprites.md) remains the Asteroids feature proposal. Its OpenGL uniform setters, direct viewport calls, and projection assumptions need reconciliation with this boundary before that rendering work is implemented. Input state and camera math remain independent responsibilities; this proposal does not implement them or change the planned controls/art direction.

The longer-term backend direction is Metal on macOS and Vulkan on Windows/Linux. The second implementation validates and may revise this interface. Exact API/OS baselines, shader tooling, and resource-binding strategy belong to that implementation's RFC.

Ray tracing and mesh shading are optional future capabilities, introduced through focused examples and separate capability/limit checks. Distinguish acceleration structures, ray queries, ray-tracing pipelines, and mesh/task stages where the APIs differ. Do not promise equivalent features, dedicated hardware acceleration, or performance across backends. The existing OpenGL 3.3 baseline remains a conventional rendering path. Browser graphics require a separate portability decision and validation; this proposal establishes no Metal/Vulkan browser path.

## Alternatives

- **Continue direct OpenGL calls throughout examples:** Lowest immediate effort, but increases API coupling and later migration work.
- **Implement Metal/Vulkan before extracting OpenGL:** Makes backend differences concrete early, but combines learning new APIs with replacing working platform/resource behavior. Deferred in favor of smaller steps.
- **Design a complete rendering hardware interface now:** Could anticipate more features, but would add unexercised abstractions. Prefer revising a small interface against real examples.
- **Adopt an existing abstraction such as SDL GPU:** Reduces backend maintenance and remains a possible future decision. The selected learning direction is to own a small backend boundary and later investigate native advanced features.

## Risks and limitations

An interface exercised only by OpenGL may retain hidden binding, synchronization, or shader assumptions. The second backend is an architectural check, not a mechanical translation task. Moving context ownership can break startup failure cleanup or resource destruction. Changing image conventions, shader bindings, or target formats can alter otherwise unchanged examples.

Windows/Linux SDKs and runtime verification remain pending. Browser builds and an automated test suite are not established. This document does not establish new platform support or authorize dependency/version changes.

## Validation

- Configure/build the native engine, all three examples, and a fresh engine-only configuration.
- Exercise Textured Quad with its original texture, geometry, filtering, orientation, scaling, and executable-relative asset lookup, including launch from outside the repository.
- Exercise both scaffold backgrounds, resizing/Retina drawable sizing, minimization/restoration, Escape/close results, VSync, and software pacing.
- Check factory/window/device/resource initialization failures, invalid shader/image inputs, callback exit/failure/exception paths, abandoned frames, partial initialization, and resource cleanup before context/device/window destruction.
- Inspect resource uploads and a rendered frame through focused GPU readback checks where practical; keep temporary outputs under ignored `build/`. Screenshots remain owner-supplied.
- Search for GLAD includes and direct OpenGL calls in examples/public rendering headers and ensure they are confined to implementation code. Verify no engine dependency on example assets/code.
- With the second backend, repeat the example/lifecycle checks and exercise resize/recreation, pending uploads, multiple frames in flight, and resource retirement under native validation tools. Document justified image tolerances rather than assuming byte-identical rendering across APIs.

Report checks actually executed and remaining manual/platform checks. These are implementation acceptance criteria, not results from creating this draft.

## Implementation plan

1. Establish the current examples' behavior and failure/lifetime checks as the migration baseline.
2. Separate window and OpenGL graphics ownership while preserving working callbacks, packaging, and frame pacing.
3. Extract the minimum resource/frame/pipeline operations needed by Textured Quad; migrate all affected examples together and remove obsolete paths.
4. Validate the extraction and update implemented architecture, development instructions, dependency guidance, and `AGENTS.md` to match the code.
5. Develop camera/sprite and then small 3D examples along the roadmap, extending the interface only for demonstrated requirements.
6. Propose and implement Metal, then Vulkan on available test systems; revise the common interface and migrate existing examples with each change.

## Open questions

- What is the smallest resource-binding and per-draw parameter layout that makes the quad explicit and can be evaluated with Metal later?
- Which private representation best preserves simple scoped ownership with one compiled backend?
- Which frame callback signature and frame-abandonment rules make exit/error handling clearest?
- Which coordinate/depth conventions should be shared with future 2D/3D cameras before adding Metal?
- Once Metal works, should OpenGL remain a maintained reference backend or become a deliberately retired learning implementation? Preserve history and record that decision separately.

## Decision

Draft created on 2026-10-18 to record the agreed learning direction: extract the working OpenGL implementation first, preserve the examples, and use later Metal/Vulkan work to improve the boundary. Detailed interfaces remain proposed, and no backend migration is implemented by this RFC. RFCs 0002 and 0003 retain their current status and historical decisions.
