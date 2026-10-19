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

### Architecture decisions

The first extraction chooses composition for window/device ownership and one private polymorphic boundary for GPU operations. These are proposed decisions, rather than an unspecified collection of backend classes.

| Type | Concrete or abstract? | Visibility | Ownership and purpose |
| --- | --- | --- | --- |
| `Application` | Existing abstract application base | Public | Owns one `Window` and one `GraphicsDevice`; examples implement application behavior |
| `Window` | Concrete, non-polymorphic | Public | Owns SDL video/window lifetime and reports size/state |
| `GraphicsDevice` | Concrete facade | Public | Owns one selected backend and validates access to rendering operations |
| `GraphicsBackend` | Abstract, with a virtual destructor | Private | Defines the native graphics implementation contract |
| `OpenGLBackend` | Concrete, final implementation of `GraphicsBackend` | Private | Owns OpenGL graphics state and implements that contract |
| `RenderFrame` | Concrete, scoped facade | Public | Borrows the device for one active frame; examples use it to draw |
| `Buffer`, `Texture2D`, `Shader`, `GraphicsPipeline` | Concrete resource owners | Public | Own opaque resource identities associated with one device |
| `OpenGLWindowSurface` | Concrete integration helper | Private | Owns the SDL OpenGL context and handles make-current/swap operations |
| `Image` | Ordinary CPU data and loading operations | Public | Owns decoded pixels, independently of a GPU |

Later `MetalBackend` and `VulkanBackend` implementations inherit from `GraphicsBackend`. They do not inherit from `Window` or `GraphicsDevice`. There are no per-API public resource subclasses such as `Texture2DMetal`, and examples do not downcast engine objects.

```text
Example application : Application
├── example renderer
│   └── Buffer / Texture2D / Shader / GraphicsPipeline
└── base Application
    ├── Window                         concrete SDL window owner
    └── GraphicsDevice                 concrete public facade
        └── unique_ptr<GraphicsBackend>
            └── OpenGLBackend          implemented first
                ├── OpenGLWindowSurface
                └── private GPU resources and frame state

Calls: GraphicsDevice → GraphicsBackend contract → selected implementation

Private inheritance hierarchy:
GraphicsBackend (abstract)
├── OpenGLBackend                       first implementation
├── MetalBackend                        later, no stub now
└── VulkanBackend                       later, no stub now
```

The inheritance hierarchy provides implementation dispatch. The ownership tree above it provides lifetime. `GraphicsDevice` owns a backend through the abstract base; it does not inherit from that base. The concrete backends do not inherit from one another.

Examples continue to derive from `frostwind::Application`, select their own assets, and supply global SDL callback adapters. The engine does not choose example assets or include example code. All initial window and rendering operations remain on SDL's callback thread; this RFC does not introduce a renderer thread or multiple windows/devices.

### Why Window stays concrete

An SDL window still has a title, size, focus state, and close events regardless of the selected graphics API. A `WindowOpenGL`/`WindowMetal` hierarchy would make native graphics creation and presentation part of the window abstraction again, coupling them to otherwise shared window behavior.

Keep these responsibilities separate:

- `Window` owns the `SDL_Window` and SDL video initialization. Its normal interface deals with window state and dimensions. It no longer owns a context, GLAD, VSync, or `present()`.
- `GraphicsDevice` owns graphics initialization, rendering resources, frame acquisition, and submission/presentation. `GraphicsConfig` owns VSync; `WindowConfig` keeps title and size.
- Private platform integration bridges the two lifetimes. Different setup flags or surface APIs require different bridge implementations, not public window subclasses.

If the project later replaces SDL with another window system, evaluate a window abstraction at that point. This proposal addresses graphics API variation while retaining SDL.

### Window creation and private platform integration

Window setup must happen before graphics-device initialization, but the compiled backend determines its requirements. Resolve that ordering at build time:

1. CMake selects one backend and its matching platform integration translation units.
2. `Window::initialize` initializes SDL video, calls the private `prepareGraphicsWindow` hook, and creates the SDL window with the returned flags.
3. The OpenGL hook sets the existing requested context attributes before window creation and selects `SDL_WINDOW_OPENGL`. This moves API-specific setup out of the generic window implementation.
4. `GraphicsDevice::initialize(Window&, GraphicsConfig)` creates the compiled backend. `OpenGLBackend` initializes its `OpenGLWindowSurface`, makes the context current, loads GLAD, checks the required OpenGL version, and configures presentation.
5. Once the device is ready, `Application` calls the example's `initialize()`.

The hook has exactly one linked implementation. It is a private platform operation, not a runtime registry or a virtual method on `Window`. `Window` retains ownership of the window after creation even though the selected integration determines the graphics flags.

`OpenGLWindowSurface` is a narrowly scoped friend of `Window` for SDL graphics integration. It accesses the private window only to create/use its context and swap buffers. No `Window::nativeHandle()` getter is introduced. The context belongs to the helper, the helper belongs to `OpenGLBackend`, and the backend borrows the window for its lifetime. Generic rendering code and examples cannot obtain the SDL window through this path.

Later integrations follow the same ordering but can have different creation dependencies:

| Implementation | Window setup | Graphics-side ownership after window creation |
| --- | --- | --- |
| OpenGL, first | SDL OpenGL attributes and `SDL_WINDOW_OPENGL` | Context in `OpenGLWindowSurface`; GLAD/state/resources in `OpenGLBackend` |
| Metal, later | Metal-compatible SDL window setup | Private view/layer integration and Metal device/queue/presentation resources |
| Vulkan, later | `SDL_WINDOW_VULKAN` | Instance, SDL-created surface, device/queues, and swapchain with dependency-aware teardown |

SDL documents the [OpenGL attribute ordering](https://wiki.libsdl.org/SDL3/SDL_GL_SetAttribute), [Vulkan window/instance requirements](https://wiki.libsdl.org/SDL3/SDL_Vulkan_CreateSurface), and [Metal view creation](https://wiki.libsdl.org/SDL3/SDL_Metal_CreateView). A Metal view does not assign its GPU device automatically. The later backend RFCs must define their concrete integration owners and destruction order; they cannot assume an OpenGL context and swap call map directly to those APIs.

### Public facade and private backend contract

`GraphicsDevice` is the engine-facing facade, not an abstract base for examples to implement. It owns `std::unique_ptr<detail::GraphicsBackend>`, with the backend forward-declared and destruction implemented out of line. A private `createGraphicsBackend` factory returns the implementation compiled into the engine. OpenGL is the only factory result in the first extraction.

The facade keeps engine-level checks in one place: initialization state, resource/device identity, valid resource descriptions, and frame sequencing. The abstract backend implements native creation, state, drawing, and presentation. It has a virtual destructor so deletion through the owning base pointer releases the actual implementation. Native failure diagnostics come from the backend and propagate through the facade to application callback results.

The initial contract covers operation groups exercised by the existing examples:

| Operation group | Contract |
| --- | --- |
| Initialization and limits | Initialize against an existing window, report the backend and supported limits, clean up partial startup |
| Resource creation | Create buffers, RGBA8 textures, vertex/fragment shader stages, and graphics pipelines from engine descriptions |
| Resource release | Relinquish an opaque resource identity and perform native deletion when safe |
| Frame acquisition | Return ready, temporarily unavailable, or failed, with the actual target extent when ready |
| Frame commands | Clear the target, select pipeline/resources/parameters, and draw vertices |
| Frame completion | Submit/present once, or abandon safely; drain outstanding work during shutdown |

Use ordinary concrete resource wrappers rather than a virtual class for every resource or API call. One private interface makes the native implementation replaceable without exposing polymorphism to the examples. Virtual dispatch is an implementation choice for this boundary, not a promise of runtime backend switching. The second backend may revise the contract where its implementation demonstrates a missing concept.

### Intended source layout and build selection

The following layout identifies ownership boundaries. Group related operations in the listed implementation files; do not split every operation into a new class or create future backend placeholders.

```text
engine/src/
├── core/Types.h
├── app/
│   └── Application.h, Application.cpp
├── platform/
│   ├── Window.h, Window.cpp
│   ├── internal/
│   │   └── WindowGraphicsSetup.h        private setup-hook declaration
│   └── opengl/
│       └── OpenGLWindowSurface.h, OpenGLWindowSurface.cpp
└── rendering/
    ├── GraphicsDevice.h, GraphicsDevice.cpp
    ├── GraphicsTypes.h                  engine descriptions/configuration
    ├── RenderFrame.h, RenderFrame.cpp
    ├── Buffer.h, Buffer.cpp
    ├── Texture2D.h, Texture2D.cpp
    ├── Shader.h, Shader.cpp
    ├── GraphicsPipeline.h, GraphicsPipeline.cpp
    ├── Image.h, Image.cpp
    ├── StbImage.cpp
    ├── internal/
    │   ├── GraphicsBackend.h            abstract implementation contract
    │   ├── GraphicsBackendFactory.h     private factory declaration
    │   └── ResourceHandle.h             typed private resource identities
    └── backends/
        └── opengl/
            ├── OpenGLBackend.h, OpenGLBackend.cpp
            ├── OpenGLResources.cpp
            └── OpenGLCommands.cpp
```

The OpenGL translation units provide the setup hook and factory definitions. `OpenGLResources.cpp` and `OpenGLCommands.cpp` group implementations declared in the private backend header; they do not introduce additional abstract layers. Private helpers can be added beside them when their size/responsibility warrants it. Use namespace `frostwind`, with implementation contracts in `frostwind::detail`, and retain the existing primitive aliases and header conventions.

Examples include the public rendering headers. They do not include `internal/`, `backends/`, or platform integration headers. Public rendering headers expose no `GLuint`, `GLint`, `Vk*`, or Objective-C types. The physical include root stays `engine/src/`; enforce the intended boundary through dependency checks and review rather than claiming these files are inaccessible to the compiler.

Keep the static `frostwind` target and explicit CMake source lists. The first extraction compiles only OpenGL, retains existing presets/output directories, and makes GLAD an implementation dependency with static-link requirements propagated as needed. Add a backend cache selection only when a second real implementation exists; reject unsupported choices at configure time. Compile matching platform/backend sources together. Shader compilation products and any backend-specific intermediate outputs belong under `build/`.

The initial scope and later additions are distinct:

| First OpenGL extraction | Later, when exercised |
| --- | --- |
| Concrete `Window`, public device/frame/resource facades, one private abstract backend, one OpenGL implementation | Additional concrete backends and their platform bridges |
| Existing window color target, quad vertex data, parameter block, one texture, explicit drawing state | Indexed meshes, depth targets, configurable passes, richer resource bindings |
| One window, one device, one active frame on the callback thread | Additional concurrency or windowing only when an example needs it |
| Existing GLSL and PNG/JPEG asset workflow | Native shader artifact tooling and additional formats |
| Limits required to validate implemented operations | Capability/limit interfaces for compute, mesh shading, and ray tracing as those features arrive |

### Resource ownership and the first drawing contract

Resource wrappers are non-copyable scoped owners. Initialization remains explicitly fallible. Each wrapper borrows its creating `GraphicsDevice` and holds a private typed identity; native objects live in backend-owned resource storage. This is not shared ownership of the graphics device. Examples must destroy resources before that device, as the application member order guarantees for normal example members.

Resource identities include enough information to reject stale entries, and the facade checks device ownership and resource type before use. Binding a texture from another device or a released buffer is an error. Native storage is selected entirely inside the backend, without casts or native handles in example code. Moving a wrapper can transfer its identity when an actual caller needs move support.

Uploads consume/copy caller bytes before returning success, so decoded pixels and temporary vertex data can be released afterward. A future asynchronous implementation must retain its staging data until GPU work completes rather than borrowing the caller's pointer. A successfully created pipeline retains the native shader state it needs independently of the caller's shader wrappers.

The initial draw contract deliberately matches Textured Quad:

| Description | Initial quad contract |
| --- | --- |
| Vertex data | Six vertices with `Float2` position and `Float2` UV attributes at locations 0 and 1; explicit stride/offsets |
| Geometry | Non-indexed triangle list, first vertex 0, count 6 |
| Texture | One RGBA8 texture at engine resource slot 0, nearest filtering and clamp-to-edge sampling |
| Parameters | Vertex-stage parameter slot 0 contains two scale floats and explicit padding to a 16-byte block |
| Shader layout | Pipeline metadata maps engine parameter/resource slots to the shader's declared resources; names/locations remain private |
| Pipeline state | Explicit target format, triangle topology, blending disabled, depth disabled, culling disabled |
| Target | One acquired window color target; extent/format reported by the frame |

Replace `uScale`'s public uniform location with that parameter block, and map the block to a cached OpenGL uniform-buffer binding inside the backend. The OpenGL shader uses a matching `std140` layout. Map texture slot 0 to the program's sampler privately. Validate shader/layout compatibility during pipeline initialization. Parameter bytes are copied when recorded; later updates cannot modify an earlier draw's parameters. Keep native program/block/sampler lookups out of the example.

A pipeline describes complete draw state; OpenGL applies it using its state model. It does not need a fictional native pipeline object equivalent to Metal or Vulkan. Each draw validates its required bindings rather than inheriting another renderer's global OpenGL state. `OpenGLBackend` owns program/VAO/state setup, including the relationship between the vertex layout and selected buffer.

Separate samplers, indexed drawing, depth attachments, multiple render passes, compute, and advanced shader stages are added when examples exercise them. Do not add empty advanced methods to `GraphicsBackend` now. Sampling can remain part of the initial texture description. A single window color pass is sufficient for all three current examples.

### Images, shaders, and coordinate conventions

`Texture2D` retains scoped GPU ownership, while PNG/JPEG decoding moves to `Image`. Compile the pinned stb_image implementation once and keep decoder configuration private. Device limits belong to `GraphicsDevice`. A file-loading convenience on `Texture2D` can compose image loading and upload; application code never calls a static `Texture2D::initializeSupport()`.

Preserve missing/corrupt/oversized-file diagnostics and overflow-safe pixel-size validation, including inspection against caller-supplied device limits before full decoding. CPU loading itself must work without initializing OpenGL. The same decoded image can be inspected by CPU checks before any upload.

For the OpenGL extraction, preserve the current bottom-left image/UV convention, RGBA8 upload behavior, and quad orientation. Keep projection math out of the backend extraction. Before another backend is implemented, settle the common world/clip/depth convention with the camera design, define texture/framebuffer conversions explicitly, and verify them with asymmetric images and known geometry. These coordinate decisions remain open; they do not make the class/ownership design above optional.

Examples own shader sources and their license records. `Shader` represents a stage with an explicit entry point and source format; retain the current GLSL source/file-loading path for OpenGL. `GraphicsPipeline` replaces the linked `ShaderProgram` contract for drawing. A later artifact pipeline can produce native shader formats under `build/` and package them beside each example, with source/license provenance retained. Do not assume one binary, resource layout, or shader-stage model works unchanged on every API. New compiler dependencies require their own recorded integration decisions.

### Application and frame contract

Preserve SDL's callback loop, global example adapters, and exception translation. `Application` owns `Window window_` followed by `GraphicsDevice graphics_`. It exposes a protected `graphics()` reference for resource initialization. Keep the derived `initialize()` and `handleEvent()` roles, and change rendering from `render(width, height)` to `render(RenderFrame&)`. Drawable dimensions come from the frame, so a window resize cannot leave an example drawing against a different acquired target size.

The example callback borrows the frame; it does not own submission/presentation. `RenderFrame` exposes target extent/format and operations such as clear, pipeline selection, resource/parameter binding, and drawing. These operations forward to the owning device/backend; they do not contain native API branches. Frame operations report failure so examples can propagate `SDL_APP_FAILURE`.

`Application::iterate` coordinates one frame at a time:

```text
Query window state
    ↓
Minimized / zero size? → idle delay → next callback
    ↓ otherwise
Acquire frame
    ├── unavailable → paced skip → next callback
    ├── failed → SDL_APP_FAILURE
    └── ready → render(frame)
                    ├── continue → submit/present → apply frame budget
                    ├── success → abandon frame → SDL_APP_SUCCESS
                    ├── failure → abandon frame → SDL_APP_FAILURE
                    └── exception → abandon frame → callback failure handler
```

A frame can transition from active to submitted or abandoned exactly once. A scoped guard owned by the application abandons any still-active frame during early return or exception unwinding. The example may not retain the frame reference, acquire a nested frame, or draw after completion. Device/frame identity and state checks diagnose invalid sequences.

For the first backend, acquisition establishes the current OpenGL context/default framebuffer and full-target viewport. Drawing can execute immediately; the facade does not promise native command-buffer recording. Submission presents the frame. Abandonment skips presentation and resets frame bookkeeping; it cannot undo OpenGL commands already executed. Later Vulkan/Metal abandonment may require internal submissions or synchronization to release acquired presentation resources safely; it does not simply drop a command buffer. The scoped guard's abandonment operation must not throw, including during exception unwinding.

Resize/recreation belongs to the backend and its private surface integration. Query the latest drawable size before acquisition, skip zero sizes, and use the actual acquired target extent. Temporarily unavailable targets are not fatal; permanent device/presentation failures are. Log the failed operation and native diagnostic. Report presentation failure rather than silently presenting stale content.

Retain [RFC 0004](0004-frame-pacing.md)'s frame budget, existing minimized delay, and independent software cap/VSync settings. Move `WindowConfig::vsync` to `ApplicationConfig::graphics.vsync` together with all affected example configuration. An unavailable frame also receives pacing, including an idle fallback when the software cap is disabled, to avoid a busy loop. Waiting/presentation time counts toward the budget; no simulation catch-up is introduced.

### Startup failure and teardown

Startup runs through the following ownership stages:

```text
Create derived application (no GPU allocation in constructors)
    ↓
Initialize SDL video → prepare graphics window → create Window
    ↓
Create backend → create graphics surface/context → initialize graphics
    ↓
Initialize example resources
    ↓
Transfer Application ownership to SDL callback state
```

Any failure leaves `appstate` null, and the local owning application cleans up completed stages. This covers window-attribute failure, context/loader failure, and failure partway through example resource creation. Window/backend/resource initialization is one-time unless retry is explicitly supported, and destructors tolerate partial startup without throwing. The OpenGL context remains usable until all example GPU owners have been destroyed.

Normal destruction proceeds in this order:

1. Example resource wrappers release their identities while `GraphicsDevice` still exists.
2. The backend waits/drains outstanding GPU work and retires native resources.
3. The backend releases presentation resources, context/device, and platform surface integration in their dependency order.
4. `Window` destroys the SDL window and releases its SDL video initialization. SDL's callbacks retain responsibility for final SDL shutdown.

Native resource retirement is distinct from releasing a C++ wrapper. A frame/pipeline retains the internal resource records it references, including transitive shader/buffer/texture dependencies, until those references are no longer needed. Later explicit backends associate submitted references with completion tracking before deleting native storage. Early wrapper destruction therefore cannot invalidate recorded or in-flight work. Outliving the owning device is a programming error, not a reason to make every resource share ownership of that device.

Waiting only in the base application destructor is insufficient: derived resource members are already destroyed at that point. Keep release/retirement backend-owned and validate it with the second backend. OpenGL can rely on its documented object-deletion semantics while the context is current; it does not need a speculative Vulkan-style retirement queue in the first extraction.

### Existing examples and incremental migration

| Existing code | Migration |
| --- | --- |
| `Window` owns OpenGL context/loading/presentation | Move graphics ownership to the device and private integration code |
| `Application` calls `Texture2D::initializeSupport()` and `Window::present()` | Initialize graphics support through the device and use its frame lifecycle |
| `QuadRenderer` owns VAO/VBO and OpenGL uniform locations | Own engine resources and submit the same geometry through the shared interface |
| Asteroids and Mini Artillery call `glClear` | Request their existing background clears through a frame |
| Examples include GLAD | Remove those includes and consume the engine rendering interface |

Textured Quad demonstrates the complete call path after migration:

1. `QuadDemo::initialize()` resolves its executable-relative asset directory and passes it, with `graphics()`, to its renderer.
2. `QuadRenderer` initializes its buffer, shaders, texture, and pipeline through the device-backed resource owners. It retains its existing geometry and asset selection.
3. `Application::iterate()` acquires a frame and calls `QuadDemo::render(frame)`.
4. `QuadDemo` delegates to `QuadRenderer`, which calculates its scale from the frame extent, clears the existing background, supplies the scale block and texture, and requests the six-vertex draw.
5. The frame/device facade validates the draw and forwards it to `GraphicsBackend`; the compiled `OpenGLBackend` applies state and issues the native operations.
6. The example returns continue, and the application submits/presents through the device. It never asks `Window` to swap.

The two scaffolds use only frame clears and their existing input handling. They do not need buffers, textures, or an example-specific backend branch to retain their current behavior.

Update affected examples in the same completed change as each interface change. Preserve source directory numbering, target/executable names, executable-relative asset selection, framework packaging, Escape/close behavior, and historical captures. There is one current example structure; examples do not remain indefinitely on an abandoned interface. Temporary implementation scaffolding is removed once the replacement passes validation.

Retain OpenGL as the only implemented backend throughout this extraction. Its GLAD dependency becomes an implementation dependency; verify the include/link propagation required by the static library without exposing GLAD through public rendering headers. Preserve pinned versions and local dependency records under [RFC 0001](0001-local-dependencies.md).

### Relationship to other planned work

[RFC 0005](0005-camera-keyboard-and-sprites.md) remains the Asteroids feature proposal. Its OpenGL uniform setters, direct viewport calls, and projection assumptions need reconciliation with this boundary before that rendering work is implemented. Input state and camera math remain independent responsibilities; this proposal does not implement them or change the planned controls/art direction.

The longer-term backend direction is Metal on macOS and Vulkan on Windows/Linux. The second implementation validates and may revise this interface. Exact API/OS baselines, shader tooling, and resource-binding strategy belong to that implementation's RFC.

Ray tracing and mesh shading are optional future capabilities, introduced through focused examples and separate capability/limit checks. Distinguish acceleration structures, ray queries, ray-tracing pipelines, and mesh/task stages where the APIs differ. Do not promise equivalent features, dedicated hardware acceleration, or performance across backends. The existing OpenGL 3.3 baseline remains a conventional rendering path. Browser graphics require a separate portability decision and validation; this proposal establishes no Metal/Vulkan browser path.

## Alternatives

- **Abstract Window with per-API subclasses:** Makes construction convenient to group, but combines common SDL window behavior with graphics ownership/presentation. Prefer a concrete SDL window plus private per-API surface helpers. Reconsider window polymorphism only if another window implementation is introduced.
- **Expose abstract GraphicsDevice directly:** Reduces one forwarding layer, but exposes backend implementation concerns to resource owners and callers. Prefer a stable concrete facade for validation/lifetime rules and a private abstract backend for native operations.
- **Use only direct calls or compile-time replacement implementations:** Avoids virtual dispatch and is viable with one backend. The proposed single private interface makes the implementation contract explicit now; compare its simplicity and dispatch cost with real examples before introducing more abstraction.
- **Continue direct OpenGL calls throughout examples:** Lowest immediate effort, but increases API coupling and later migration work.
- **Implement Metal/Vulkan before extracting OpenGL:** Makes backend differences concrete early, but combines learning new APIs with replacing working platform/resource behavior. Deferred in favor of smaller steps.
- **Design a complete rendering hardware interface now:** Could anticipate more features, but would add unexercised abstractions. Prefer revising a small interface against real examples.
- **Adopt an existing abstraction such as SDL GPU:** Reduces backend maintenance and remains a possible future decision. The selected learning direction is to own a small backend boundary and later investigate native advanced features.

## Risks and limitations

An interface exercised only by OpenGL may retain hidden binding, synchronization, or shader assumptions. The second backend is an architectural check, not a mechanical translation task. Moving context ownership can break startup failure cleanup or resource destruction. Changing image conventions, shader bindings, or target formats can alter otherwise unchanged examples.

The private virtual boundary adds dispatch and resource bookkeeping; it does not guarantee performance. Keep operations coarse enough to express useful drawing work and measure before adding batching or a command stream. Resource identities, frame references, and borrowed device lifetime need explicit checks and documentation. A public facade cannot hide incompatible capabilities by silently approximating them.

Windows/Linux SDKs and runtime verification remain pending. Browser builds and an automated test suite are not established. This document does not establish new platform support or authorize dependency/version changes.

## Validation

- Configure/build the native engine, all three examples, and a fresh engine-only configuration.
- Exercise Textured Quad with its original texture, geometry, filtering, orientation, scaling, and executable-relative asset lookup, including launch from outside the repository.
- Exercise both scaffold backgrounds, resizing/Retina drawable sizing, minimization/restoration, Escape/close results, VSync, and software pacing.
- Check factory/window/device/resource initialization failures, invalid shader/image inputs, callback exit/failure/exception paths, abandoned frames, partial initialization, and resource cleanup before context/device/window destruction.
- Check graphics window setup before context creation, factory/compiled-backend agreement, and cleanup if the context exists but GLAD initialization fails. Verify `Window` has no context/presentation ownership or public native-handle getter.
- Check stale/wrong-device resource rejection, parameter-block size/layout mismatches, required bindings, parameter copying between successive draws, operations on completed frames, and failure during frame submission/abandonment. Confirm shader-wrapper cleanup cannot invalidate a live pipeline.
- Inspect resource uploads and a rendered frame through focused GPU readback checks where practical; keep temporary outputs under ignored `build/`. Screenshots remain owner-supplied.
- Search for GLAD includes and direct OpenGL calls in examples/public rendering headers and ensure they are confined to implementation code. Verify no engine dependency on example assets/code.
- With the second backend, repeat the example/lifecycle checks and exercise resize/recreation, pending uploads, multiple frames in flight, and resource retirement under native validation tools. Document justified image tolerances rather than assuming byte-identical rendering across APIs.

Report checks actually executed and remaining manual/platform checks. These are implementation acceptance criteria, not results from creating this draft.

## Implementation plan

1. Establish current example behavior, uploads, shader parameters, callback results, and failure/lifetime checks as the migration baseline.
2. Introduce the concrete device facade, private abstract backend, OpenGL implementation, and private window setup/surface helpers. Move graphics ownership and presentation while keeping all examples runnable at the end of the increment. Temporary direct-OpenGL resource usage remains explicitly transitional within the working OpenGL build.
3. Add the scoped frame contract and migrate all three callbacks/clears, including pacing and abandonment paths. Verify the scaffold backgrounds and quad still work before removing old presentation entry points.
4. Introduce the minimum resource/pipeline/parameter descriptions, separate CPU image loading, and migrate Textured Quad. Remove obsolete public shader/uniform operations, direct example OpenGL calls, and the temporary path in the same completed increment.
5. Validate the full extraction and update implemented architecture, development instructions, dependency guidance, and `AGENTS.md` to match the code. Completion requires all examples using the one current interface.
6. Develop camera/sprite and then small 3D examples along the roadmap, extending the interface only for demonstrated requirements.
7. Propose and implement Metal, then Vulkan on available test systems; revise the common interface and migrate existing examples with each change.

## Open questions

- Which world/clip/depth conventions should be shared with future 2D/3D cameras before adding Metal? Preserve the current quad's image/UV contract during extraction.
- What native shader artifact tooling, language, and API/OS baselines should the Metal implementation use? The first OpenGL path retains GLSL.
- After implementing a second backend, which resource-binding descriptions or frame operations require revision? The quad's explicit slot/block layout is the initial test case, not a universal material system.
- Once Metal works, should OpenGL remain a maintained reference backend or become a deliberately retired learning implementation? Preserve history and record that decision separately.

## Decision

Draft created on 2026-10-18 to record the agreed learning direction: extract the working OpenGL implementation first, preserve the examples, and use later Metal/Vulkan work to improve the boundary. Refined the same day to propose concrete composition, one private backend interface, private surface integration, and explicit resource/frame/lifetime contracts. These are reviewable design choices, not implemented interfaces. RFCs 0002 and 0003 retain their current status and historical decisions.
