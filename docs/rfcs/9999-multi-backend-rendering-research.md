# RFC 9999: Multi-backend rendering research

- **Status:** Deferred research
- **Created:** 2026-10-18
- **Revised:** 2026-10-20
- **Supersedes:** None
- **Superseded by:** None

This document preserves deferred research into a shared OpenGL/Metal/Vulkan rendering architecture, including engine structure, native pipeline differences, shader variation, state ownership, and example migration. Multi-backend compatibility is not a requirement for current engine changes. Frostwind is focused on OpenGL; useful existing ownership and rendering facilities remain in place.

The number **9999** is reserved to keep this deferred proposal after active RFCs in filename/index order; it is excluded from normal sequential numbering. This document was previously RFC 0006 and retains its original creation date and analysis.

The sections below preserve the investigated design and its historical decisions. Future-backend requirements are conditional on explicitly reviving this research, not instructions to implement them now. Only the OpenGL foundation exists. The [roadmap](../../ROADMAP.md) records current priorities; the [structure guide](../project-structure.md) and [rendering guide](../rendering-guide.md) describe the working code.

**Historical snapshot:** The device/frame/pipeline/backend setup described below is preserved at commit `e9cf616` on `archive/multi-backend-experiment`. It has been replaced on the working branch by the direct OpenGL design in [RFC 0006](0006-opengl-focused-rendering.md). References below to current implementation/status describe that saved experiment, not the present source tree.

## Problem

Frostwind needs a rendering boundary that lets examples keep their application behavior and engine drawing operations when another graphics API is introduced. OpenGL is the working foundation; the intended later backends are Metal on macOS and Vulkan on Windows/Linux. Native graphics APIs organize equivalent rendering behavior differently, so copying one API's object model into the public interface would make later migrations harder.

The design must address the owner's specific concerns:

- **Different native pipeline boundaries:** Metal binds depth/stencil state separately from its render pipeline state; OpenGL applies much state through context calls; Vulkan places much of it in graphics pipeline creation with optional dynamic state. Examples should express one consistent engine operation.
- **Accidental state differences when changing shaders:** Two objects that only need different shaders should share their intended non-shader configuration rather than independently repeat it.
- **State left by previous draws:** Selecting a pipeline must establish its declared behavior, including disabling behavior that a previous pipeline enabled.
- **Existing examples becoming obsolete:** Interface changes must migrate the current examples together, preserving runnable references rather than leaving an old API to learn alongside a new one.
- **Unclear ownership and growing friendship:** Window, device, private surface, resources, and frames need explicit lifetimes. Adding a backend must not require a new public window hierarchy or another `Window` friend.
- **Too much architecture for a learning project:** A consistent contract is useful now; implementing every native feature and portability tool before a scene needs it is not the objective.

This refines the graphics ownership from [RFC 0002](0002-graphics-integration.md) while preserving the engine/example boundary from [RFC 0003](0003-frostwind-engine-and-examples.md).

## Goals and scope

- Provide shared public device, frame, resource, and pipeline interfaces with private native implementations.
- Express rendering semantics consistently even where a backend needs several objects or commands.
- Separate SDL window lifetime from graphics context/device, presentation, and GPU resource lifetime.
- Define pipeline, pass/target, and draw responsibilities, with explicit defaults and compatibility validation.
- Support shader variations without accidental changes to shared drawing state.
- Preserve current examples and keep native graphics calls/types out of them and public rendering headers.
- Add Metal and Vulkan through staged, testable ports rather than an immediate universal renderer.

Runtime backend switching, multiple devices/windows, rendering threads, a render graph, a general material system, universal shader compilation, ray tracing, mesh shading, and browser graphics are outside the current implementation scope. Later examples can introduce a specific capability with its own design and validation. This RFC does not require identical advanced features or performance on every backend.

## Implementation status

| Area | Current implementation | Intended extension |
| --- | --- | --- |
| Window/device ownership | Concrete SDL `Window`; public `GraphicsDevice` owns private `OpenGLBackend` | Keep public ownership when another backend is implemented |
| Platform integration | Private abstract `WindowSurface`, sole graphics friend of `Window`; concrete OpenGL surface | Private Metal/Vulkan integration with API-specific dependencies |
| Frames and resources | Scoped `RenderFrame`; opaque Buffer/Shader/Texture2D/GraphicsPipeline owners; CPU Image | Extend operations only as selected examples require |
| Pipeline state | RGBA8 window target, triangle list, opaque, depth disabled, no culling | Explicit depth/stencil, blending, rasterization and target compatibility as exercised |
| Resource bindings | One optional vertex parameter block and fragment texture at engine slot 0 | Richer stage/binding layouts for matrices, lighting, and other demonstrated needs |
| Shaders | GLSL vertex/fragment source/file loading with `main` entry point | Native artifacts and validated mappings for each added backend |
| Backend selection | One compiled OpenGL factory/setup hook | Build selection with a second implementation; runtime selection remains separate |
| Platforms | Native macOS OpenGL exercised; Windows/Linux SDK verification pending | Metal macOS and Vulkan Windows/Linux verified independently |

Textured Quad owns its engine resources directly in `QuadDemo` and draws through `RenderFrame`. Asteroids and Mini Artillery use the same frame interface for their scaffold clears. No Metal/Vulkan files, stubs, or public per-API resource subclasses exist today. Native minimize/restore remains a manual check; see [development validation](../development.md#shared-rendering-validation-2026-10-19).

## Proposed design

### Architecture decisions

Use composition for window/device ownership and one private polymorphic boundary for GPU operations. The OpenGL implementation establishes this structure; Metal and Vulkan will implement the same responsibilities.

| Type | Concrete or abstract? | Visibility | Ownership and purpose |
| --- | --- | --- | --- |
| `Application` | Existing abstract application base | Public | Owns one `Window` and one `GraphicsDevice`; examples implement application behavior |
| `Window` | Concrete, non-polymorphic | Public | Owns SDL video/window lifetime and reports size/state |
| `GraphicsDevice` | Concrete facade | Public | Owns one selected backend and validates access to rendering operations |
| `GraphicsBackend` | Abstract, with a virtual destructor | Private | Defines the native graphics implementation contract |
| `OpenGLBackend` | Concrete, final implementation of `GraphicsBackend` | Private | Owns OpenGL graphics state and implements that contract |
| `WindowSurface` | Abstract, with a virtual destructor and initialization contract | Private | Provides the single privileged window-access boundary for concrete surface implementations |
| `RenderFrame` | Concrete, scoped facade | Public | Borrows the device for one active frame; examples use it to draw |
| `Buffer`, `Texture2D`, `Shader`, `GraphicsPipeline` | Concrete resource owners | Public | Own opaque resource identities associated with one device |
| `OpenGLWindowSurface` | Concrete integration helper | Private | Owns the SDL OpenGL context and handles make-current/swap operations |
| `Image` | Ordinary CPU data and loading operations | Public | Owns decoded pixels, independently of a GPU |

Later `MetalBackend` and `VulkanBackend` implementations inherit from `GraphicsBackend`. They do not inherit from `Window` or `GraphicsDevice`. There are no per-API public resource subclasses such as `Texture2DMetal`, and examples do not downcast engine objects.

```text
Example application : Application
├── example-owned Buffer / Texture2D / Shader / GraphicsPipeline
└── base Application
    ├── Window                         concrete SDL window owner
    └── GraphicsDevice                 concrete public facade
        └── unique_ptr<GraphicsBackend>
            └── OpenGLBackend          implemented first
                ├── OpenGLWindowSurface : WindowSurface
                └── private GPU resources and frame state

Calls: GraphicsDevice → GraphicsBackend contract → selected implementation

Private inheritance hierarchy:
GraphicsBackend (abstract)
├── OpenGLBackend                       first implementation
├── MetalBackend                        later, no stub now
└── VulkanBackend                       later, no stub now

WindowSurface (abstract)
├── OpenGLWindowSurface                 first implementation
├── MetalWindowSurface                  later, no stub now
└── VulkanWindowSurface                 later, no stub now
```

Only the OpenGL branches exist today. The inheritance hierarchy provides implementation dispatch. The ownership tree above it provides lifetime. `GraphicsDevice` owns a backend through the abstract base; it does not inherit from that base. The concrete backends do not inherit from one another.

Examples continue to derive from `frostwind::Application`, select their own assets, and supply global SDL callback adapters. The engine does not choose example assets or include example code. All initial window and rendering operations remain on SDL's callback thread; this RFC does not introduce a renderer thread or multiple windows/devices.

### Why Window stays concrete

An SDL window still has a title, size, focus state, and close events regardless of the selected graphics API. A `WindowOpenGL`/`WindowMetal` hierarchy would make native graphics creation and presentation part of the window abstraction again, coupling them to otherwise shared window behavior.

Keep these responsibilities separate:

- `Window` owns the `SDL_Window` and SDL video initialization. Its normal interface deals with window state and dimensions. It does not own a context, GLAD, VSync, or `present()`.
- `GraphicsDevice` owns graphics initialization, rendering resources, frame acquisition, and submission/presentation. `GraphicsConfig` owns VSync; `WindowConfig` keeps title and size.
- Private platform integration bridges the two lifetimes. Different setup flags or surface APIs require different bridge implementations, not public window subclasses.

If the project later replaces SDL with another window system, evaluate a window abstraction at that point. This proposal addresses graphics API variation while retaining SDL.

### Window creation and private platform integration

Window setup must happen before graphics-device initialization, but the compiled backend determines its requirements. Keep the selected backend and its pre-window setup consistent. Today only OpenGL is linked; when build selection is added, the ordering remains:

1. CMake selects one backend and its matching platform integration translation units.
2. `Window::initialize` initializes SDL video, calls the private `prepareGraphicsWindow` hook, and creates the SDL window with the returned flags.
3. The OpenGL hook sets the existing requested context attributes before window creation and selects `SDL_WINDOW_OPENGL`. API-specific setup stays outside the generic window implementation.
4. `GraphicsDevice::initialize(Window&, GraphicsConfig)` creates the compiled backend. `OpenGLBackend` initializes its `OpenGLWindowSurface`, makes the context current, loads GLAD, checks the required OpenGL version, and configures presentation.
5. Once the device is ready, `Application` calls the example's `initialize()`.

The hook has exactly one linked implementation. It is a private platform operation, not a runtime registry or a virtual method on `Window`. `Window` retains ownership of the window after creation even though the selected integration determines the graphics flags.

`Window` grants friendship once to the private abstract `WindowSurface` base. Friendship is not inherited: concrete surfaces use the base's protected `sdlWindow(Window&)` operation, which performs the privileged access. `Window` does not grant friendship to `OpenGLWindowSurface` or future per-API classes, and gains no public `nativeHandle()` getter. Raw SDL window access remains confined to private platform integration.

`WindowSurface` has a virtual destructor and a fallible `initialize(Window&)` contract. Its constructor performs no native allocation. It owns no context, GPU device, or SDL window; concrete surfaces own their API-specific objects and borrow the window. `OpenGLWindowSurface` owns the context and OpenGL swap-interval/swap operations. The OpenGL backend holds that concrete surface directly, so a base class does not require an additional heap allocation or surface factory.

Presentation and VSync are not universal virtual surface methods. Later Metal/Vulkan implementations may need device queues, swapchains, or backend-specific arguments. The owning backend establishes prerequisites before surface initialization; for example, a Vulkan surface can borrow a previously created instance through its concrete construction/configuration before `initialize(window)` attaches it. Define that ownership precisely when implementing the backend, without passing unrelated native handles through the public engine API.

Later integrations follow the same ordering but can have different creation dependencies:

| Implementation | Window setup | Graphics-side ownership after window creation |
| --- | --- | --- |
| OpenGL, first | SDL OpenGL attributes and `SDL_WINDOW_OPENGL` | Context in `OpenGLWindowSurface`; GLAD/state/resources in `OpenGLBackend` |
| Metal, later | Metal-compatible SDL window setup | Private view/layer integration and Metal device/queue/presentation resources |
| Vulkan, later | `SDL_WINDOW_VULKAN` | Instance, SDL-created surface, device/queues, and swapchain with dependency-aware teardown |

SDL documents the [OpenGL attribute ordering](https://wiki.libsdl.org/SDL3/SDL_GL_SetAttribute), [Vulkan window/instance requirements](https://wiki.libsdl.org/SDL3/SDL_Vulkan_CreateSurface), and [Metal view creation](https://wiki.libsdl.org/SDL3/SDL_Metal_CreateView). A Metal view does not assign its GPU device automatically. The later backend RFCs must define their concrete integration owners and destruction order; they cannot assume an OpenGL context and swap call map directly to those APIs.

### Public facade and private backend contract

`GraphicsDevice` is the engine-facing facade, not an abstract base for examples to implement. It owns `std::unique_ptr<detail::GraphicsBackend>`, with the backend forward-declared and destruction implemented out of line. A private `createGraphicsBackend` factory returns the implementation compiled into the engine. OpenGL is currently the only factory result.

The facade keeps engine-level checks in one place: initialization state, resource/device identity, valid resource descriptions, and frame sequencing. The abstract backend implements native creation, state, drawing, and presentation. The current facade caches the limits needed by the first contract; public backend identification can be added when another implementation makes it useful. The backend base has a virtual destructor so deletion through the owning base pointer releases the actual implementation. Native failure diagnostics come from the backend and propagate through the facade to application callback results.

The initial contract covers operation groups exercised by the existing examples:

| Operation group | Contract |
| --- | --- |
| Initialization and limits | Initialize against an existing window, report required limits, clean up partial startup |
| Resource creation | Create buffers, RGBA8 textures, vertex/fragment shader stages, and graphics pipelines from engine descriptions |
| Resource release | Relinquish an opaque resource identity and perform native deletion when safe |
| Frame acquisition | Return ready, temporarily unavailable, or failed, with the actual target extent when ready |
| Frame commands | Clear the target, select pipeline/resources/parameters, and draw vertices |
| Frame completion | Submit/present once, or abandon safely; drain outstanding work during shutdown |

Use ordinary concrete resource wrappers rather than a virtual class for every resource or API call. One private interface makes the native implementation replaceable without exposing polymorphism to the examples. Virtual dispatch is an implementation choice for this boundary, not a promise of runtime backend switching. The second backend may revise the contract where its implementation demonstrates a missing concept.

### Intended source layout and build selection

The implemented layout establishes these ownership boundaries. Group related operations in the listed implementation files; do not split every operation into a new class or create future backend placeholders.

```text
engine/src/
├── core/Types.h
├── app/
│   └── Application.h, Application.cpp
├── platform/
│   ├── Window.h, Window.cpp
│   ├── internal/
│   │   ├── WindowGraphicsSetup.h        private setup-hook declaration
│   │   └── WindowSurface.h, WindowSurface.cpp
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
            ├── OpenGLResources.h, OpenGLResources.cpp
            └── OpenGLCommands.cpp
```

The OpenGL translation units provide the setup hook and factory definitions. `OpenGLResources.cpp` and `OpenGLCommands.cpp` group implementations declared in the private backend header; they do not introduce additional abstract layers. Private helpers can be added beside them when their size/responsibility warrants it. Use namespace `frostwind`, with implementation contracts in `frostwind::detail`, and retain the existing primitive aliases and header conventions.

Examples include the public rendering headers. They do not include `internal/`, `backends/`, or platform integration headers. Public rendering headers expose no `GLuint`, `GLint`, `Vk*`, or Objective-C types. The physical include root stays `engine/src/`; enforce the intended boundary through dependency checks and review rather than claiming these files are inaccessible to the compiler.

Keep the static `frostwind` target and explicit CMake source lists. Today only OpenGL is compiled. Retain existing presets/output directories and GLAD as an implementation dependency with static-link requirements propagated as needed. Add a backend cache selection only when a second real implementation exists; reject unsupported choices at configure time. Compile matching platform/backend sources together. Shader compilation products and any backend-specific intermediate outputs belong under `build/`.

When their implementations begin, extend the private tree beside OpenGL:

```text
engine/src/                            future additions only
├── platform/
│   ├── metal/MetalWindowSurface.h, .mm
│   └── vulkan/VulkanWindowSurface.h, .cpp
└── rendering/backends/
    ├── metal/MetalBackend.h, .mm
    └── vulkan/VulkanBackend.h, .cpp
```

These are intended locations, not existing files or a requirement to mirror every OpenGL helper filename. Keep Objective-C++ and native headers in private implementation. Public resource/window classes remain shared. Add platform/backend helpers only when their implementation needs them.

## Pipeline consistency across backends

### One logical engine pipeline

The owner identified two related risks: pipelines that should differ only in shaders can accidentally configure different drawing state, and native APIs place that state in different objects. In particular, Metal configures depth/stencil behavior separately from its render pipeline state. Mirroring each native object's boundary in example code would make backend migration require different drawing sequences and leave room for missing state.

Keep one logical, immutable Frostwind `GraphicsPipeline` that establishes its declared drawing behavior whenever used. A private backend may represent it with several native objects and command settings. Native object count, state ownership, and whether a native setting is static or dynamic do not determine the public engine contract. Consistency means documented behavior and validation for supported operations; it does not promise identical native objects, shader binaries, performance, or pixels.

Apply this constraint as examples extend the renderer. The existing OpenGL subset is implemented; the broader multi-backend contract is retained as deferred research awaiting reconsideration and validation.

### Native state comparison

For conventional vertex/fragment rendering, the APIs divide responsibilities as follows:

| Behavior | OpenGL 3.3 Core | Metal | Vulkan graphics pipeline, using static state where available |
| --- | --- | --- | --- |
| Shader stages | Linked program | Render pipeline state | Pipeline shader stages |
| Vertex interpretation | Attribute/VAO configuration | Vertex descriptor used for pipeline creation | Vertex input state |
| Primitive topology | Draw command mode | Draw command primitive type | Input assembly state |
| Blend equations and write masks | Context state | Pipeline color attachment configuration | Color blend state |
| Depth/stencil tests and writes | Context state | Separate depth/stencil state bound to encoder | Depth/stencil state |
| Culling and front face | Context state | Encoder settings | Rasterization state |
| Viewport and scissor | Context state | Encoder settings | Static state or declared dynamic state |

OpenGL distributes this behavior across its state machine and drawing commands. The engine must apply the supported state itself. See the official [OpenGL 3.3 Core specification](https://registry.khronos.org/OpenGL/specs/gl/glspec33.core.pdf), particularly vertex specification, drawing, rasterization, and per-fragment operations.

Metal uses a render pipeline state alongside `MTLDepthStencilState` and encoder settings such as culling, winding, and viewport. The encoder also receives the draw's primitive type. A future private Metal pipeline record can retain both native state objects and the values needed for encoder commands. See Apple's [render command encoder guide](https://developer.apple.com/library/archive/documentation/Miscellaneous/Conceptual/MetalProgrammingGuide/Render-Ctx/Render-Ctx.html), including fixed-function operations and depth/stencil operations.

Vulkan's `VkGraphicsPipelineCreateInfo` includes shader stages, input assembly, rasterization, multisampling, depth/stencil, color blending, and layout. Declared dynamic state is supplied through commands; the available states and restrictions depend on the chosen API version and enabled features/extensions. A future backend must select and validate that baseline. Native dynamic state need not become publicly mutable engine state. See the [graphics pipeline creation reference](https://docs.vulkan.org/refpages/latest/refpages/source/VkGraphicsPipelineCreateInfo.html) and [dynamic state guide](https://docs.vulkan.org/guide/latest/dynamic_state.html).

The intended mapping is:

```text
Example → engine GraphicsPipeline + complete draw bindings
              ↓ private backend translation
              ├── OpenGL → program + vertex setup + explicit GL state
              ├── Metal  → render pipeline state + depth/stencil state
              │            + encoder settings
              └── Vulkan → graphics pipeline + required dynamic commands
```

Metal and Vulkan branches are planned mappings, not existing classes or verified implementations.

### Engine state ownership

As examples require more features, keep these responsibilities explicit. The groupings below describe intended semantics, not new C++ types or already available fields:

| Owner | Intended responsibility |
| --- | --- |
| Pipeline | Shader stages, vertex/resource layout, topology, rasterization/culling, depth/stencil behavior, blending/write masks, and compatible attachment formats/sample count |
| Rendering pass/target | Actual color and depth/stencil attachments, their extent, and clear/load/store behavior |
| Frame/draw inputs | Geometry range, resource bindings, parameter values, and explicitly supported dynamic values |

Every effective state value must come from the pipeline, an explicit frame/pass/draw input, or a documented engine default. Nothing may depend on whichever unrelated draw ran previously. Defaults must have engine-defined semantics; native defaults are not the contract. Applying another pipeline must also turn off previously enabled behavior when the new pipeline disables it.

Backends may cache effective state and skip redundant commands. Such caching must account for new contexts, frames/encoders, and pass transitions; it cannot assume a native pipeline bind establishes state that the API stores elsewhere. Examples continue to issue engine operations rather than native calls that bypass this ownership.

Viewport and scissor are candidates for explicit frame/draw state when needed. Today acquisition establishes the full-target viewport and scissoring is disabled. If stencil reference values, blend constants, or other dynamic values are introduced, define their owner and default before exposing them. A setting being dynamic in Vulkan or encoder state in Metal is not sufficient reason to add a public setter.

### Shader changes with shared drawing state

The current API already accepts shaders separately from a copyable `PipelineDescription`. An example can configure that description once and initialize two persistent pipelines with it:

```text
commonDescription ← configure shared layout and drawing state once
pipelineA.initialize(device, commonDescription, vertexShader, fragmentShaderA)
pipelineB.initialize(device, commonDescription, vertexShader, fragmentShaderB)

render(frame):
    draw object A with pipelineA and object A's resources/parameters
    draw object B with pipelineB and object B's resources/parameters
```

Both pipelines copy the description. Later changes to the source description do not mutate either pipeline. Many objects may share a pipeline; object transforms, textures, and lighting values normally arrive through bindings/parameters rather than requiring one pipeline per object. A new compatible shader combination creates another pipeline during initialization or an explicit reload, not on every draw.

The two shaders must match the declared vertex inputs, parameter/resource bindings, and target outputs. A shader that requires a different interface needs an explicitly different layout. Sharing configuration prevents accidental state divergence; it does not make arbitrary shaders interchangeable or prevent a shader from intentionally changing appearance, discarding fragments, or writing depth. General materials, pipeline caches, and shader-variant builders remain deferred until repeated usage justifies them.

### Depth/stencil behavior and attachment compatibility

Depth testing, depth writing, and comparison are separate semantics. When depth support is implemented, specify each supported combination explicitly; one ambiguous enabled/disabled switch is insufficient for both opaque geometry and depth-tested geometry that must not write depth. Unsupported combinations must fail validation. Stencil support, when needed, must likewise define comparisons, operations, masks, face behavior, and the ownership of reference values.

The pipeline describes how depth/stencil is used. The pass supplies compatible storage and its clear/load/store operations. A depth clear value belongs with the pass operation, not the depth comparison setting. The camera/projection convention, comparison, and clear value must agree. Enabling depth/stencil behavior must not silently work without its required attachment, and binding a pipeline must not clear that attachment.

Validate color/depth/stencil formats, sample counts, and supported read/write usage against the actual rendering target before issuing incompatible work. Pipeline initialization validates known configuration and shader interfaces; drawing/pass setup validates the acquired attachments. This distinction matters on resize or target recreation, when compatible dimensions may change without changing the pipeline's shader/state semantics. Concrete pass APIs and the exact validation locations remain implementation decisions when the first example needs them.

Reject unsupported features or combinations with a useful diagnostic. Do not silently disable depth, replace blending, or choose another target format to make a backend accept a description. An example may deliberately select a documented alternative after checking support; an implicit backend substitution would undermine the shared contract.

### Conventions and limits of stability

Pipeline state alone cannot establish portability. Before the first 3D camera and a second backend, document the appropriate shared world/clip/depth conventions, front-face winding, texture/framebuffer orientation, color-space behavior, and shader parameter/binding layouts. Define any backend conversions once and verify them with known geometry and asymmetric images. Do not assume the quad's `std140` bytes are automatically a compatible Metal or Vulkan shader interface.

The stable objective is that a supported engine operation retains its documented meaning across backends. The precise public fields remain provisional until concrete examples and a second implementation validate them. API changes must migrate affected examples together. Shader artifacts and unsupported advanced features can still require backend-specific work; a shared pipeline does not eliminate it.

### Current coverage and incremental implementation

Today `PipelineDescription` carries vertex/binding metadata and target, topology, blend, depth, and cull fields. Their supported drawing-state values are only RGBA8, triangle lists, opaque blending, disabled depth, and no culling. There is no stencil description, configurable depth attachment, multisampling contract, or general pass API. The OpenGL backend explicitly applies its supported state on draws; this is not an implementation of all the future states above.

For the small 3D example, extend only the formats, transforms, depth storage/state, and any culling or indexed drawing it actually needs. Specify defaults, attachment compatibility, and state-transition checks with each addition. Preserve Textured Quad and both scaffolds. Implement a Metal or Vulkan mapping only when that backend is selected, and use it to test this contract rather than declaring portability established from the comparison table.

## Resources, shaders, and frame lifecycle

### Resource ownership and the first drawing contract

Resource wrappers are non-copyable scoped owners. Initialization remains explicitly fallible. Each wrapper borrows its creating `GraphicsDevice` and holds a private typed identity; native objects live in backend-owned resource storage. This is not shared ownership of the graphics device. Examples must destroy resources before that device, as the application member order guarantees for normal example members.

Resource identities are typed, nonzero, monotonically assigned values that are never reused during a device lifetime; backend registries reject stale entries, and the facade checks device ownership and resource type before use. Binding a texture from another device or a released buffer is an error. Native storage is selected entirely inside the backend, without casts or native handles in example code. Moving a wrapper can transfer its identity when an actual caller needs move support.

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

The quad scale parameter block replaces the former public `uScale` uniform location and maps to a cached OpenGL uniform-buffer binding inside the backend. The OpenGL shader uses a matching `std140` layout. Texture slot 0 maps to the program's sampler privately. Pipeline initialization validates the supported shader/layout contract. Parameter bytes are consumed/copied before each draw returns; later updates cannot modify an earlier draw's parameters. Keep native program/block/sampler lookups out of the example.

### Images, shaders, and coordinate conventions

`Texture2D` retains scoped GPU ownership, while PNG/JPEG decoding moves to `Image`. Compile the pinned stb_image implementation once and keep decoder configuration private. Device limits belong to `GraphicsDevice`. A file-loading convenience on `Texture2D` can compose image loading and upload; application code never calls a static `Texture2D::initializeSupport()`.

Preserve missing/corrupt/oversized-file diagnostics and overflow-safe pixel-size validation, including inspection against caller-supplied device limits before full decoding. CPU loading itself must work without initializing OpenGL. The same decoded image can be inspected by CPU checks before any upload.

Preserve the current bottom-left image/UV convention, RGBA8 upload behavior, and quad orientation. Keep shared camera/projection math independent of the native implementation. Before another backend is implemented, settle the common world/clip/depth convention with the camera design, define texture/framebuffer conversions explicitly, and verify them with asymmetric images and known geometry. These coordinate decisions remain open; they do not make the class/ownership design above optional.

Examples own shader sources and their license records. `Shader` represents a stage with an explicit entry point and source format; retain the current GLSL source/file-loading path for OpenGL. `GraphicsPipeline` combines stages and drawing state without exposing the former linked `ShaderProgram` uniform API. Do not assume one binary, resource layout, or shader-stage model works unchanged on every API.

The current quad uses a 16-byte CPU scale block and GLSL `std140` block containing a `vec4`, of which `.xy` holds scale. A `vec2` member could express the two values while retaining appropriate CPU block padding. This is a verified OpenGL data contract, not a universal memory layout. Future shader artifacts must specify offsets, alignment, resource kinds, stage visibility, and engine-to-native binding mappings explicitly. Camera matrices and lighting values will use those resource contracts as the examples require them.

For a second backend, choose deliberately between maintained native shader variants and shared source compiled into native artifacts. The current `Shader` compiles GLSL; it does not translate languages. A common engine interface does not imply that OpenGL GLSL can be passed directly to Metal or Vulkan unchanged. Entry points, supported stages, artifact selection, packaging, compiler versions, and source/license provenance belong in the backend-specific design. Keep generated outputs under `build/` and introduce compiler dependencies through the pinned local dependency policy.

### Application and frame contract

Preserve SDL's callback loop, global example adapters, and exception translation. `Application` owns `Window window_` followed by `GraphicsDevice graphics_`. It exposes a protected `graphics()` reference for resource initialization. Keep the derived `initialize()` and `handleEvent()` roles, and use `render(RenderFrame&)` for rendering. Drawable dimensions come from the frame, so a window resize cannot leave an example drawing against a different acquired target size.

The example callback borrows the frame; it does not own submission/presentation. `RenderFrame` exposes target extent/format, `clear`, and a coarse `draw` operation that receives the pipeline, vertex buffer, range, optional texture, and parameter bytes together. Complete per-draw bindings avoid inheriting state from a previous renderer without requiring separate binding methods in this first contract. These operations forward to the owning device/backend; they do not contain native API branches. Frame operations report failure so examples can propagate `SDL_APP_FAILURE`.

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

In the current OpenGL backend, acquisition establishes the current OpenGL context/default framebuffer and full-target viewport. Drawing can execute immediately; the facade does not promise native command-buffer recording. Submission presents the frame. Abandonment skips presentation and resets frame bookkeeping; it cannot undo OpenGL commands already executed. Later Vulkan/Metal abandonment may require internal submissions or synchronization to release acquired presentation resources safely; it does not simply drop a command buffer. The scoped frame's destructor performs abandonment and must not throw, including during exception unwinding. The stack object itself performs no GPU allocation; backend reference storage can grow as draws are issued.

Resize/recreation belongs to the backend and its private surface integration. Query the latest drawable size before acquisition, skip zero sizes, and use the actual acquired target extent. Temporarily unavailable targets are not fatal; permanent device/presentation failures are. Log the failed operation and native diagnostic. Report presentation failure rather than silently presenting stale content.

Retain [RFC 0004](0004-frame-pacing.md)'s frame budget, existing minimized delay, and independent software cap/VSync settings. VSync is configured through `ApplicationConfig::graphics.vsync`. An unavailable frame receives the remaining configured budget, with a 16 ms fallback when the software cap is disabled, to avoid a busy loop. Minimized windows retain the separate 16 ms idle delay. Waiting/presentation time counts toward the budget; no simulation catch-up is introduced.

`clear` and `draw` have ordered engine semantics. A clear initializes the target before the intended group of draws; it does not render geometry or belong implicitly to a pipeline. An explicit backend may implement a leading clear with an attachment load action, but must preserve ordering for any supported clear after draws. If supporting such an operation requires another native command/pass, that is private backend work; it must not silently erase or reorder earlier engine work. Future pass APIs must define clear scope, load/store behavior, and supported sequencing before replacing the current operations.

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

Waiting only in the base application destructor is insufficient: derived resource members are already destroyed at that point. Keep release/retirement backend-owned and validate it with the second backend. OpenGL can rely on its documented object-deletion semantics while the context is current; it does not need a speculative Vulkan-style retirement queue for the current OpenGL implementation.

### Existing examples and migration policy

Examples derive from `Application`, own their behavior and assets, and use public engine drawing operations. They do not choose a native renderer implementation or submit native commands. The initial backend choice belongs to engine/build setup; backend-specific shader artifact selection should be centralized in asset loading when needed.

```text
QuadDemo : Application
├── Buffer / Texture2D / GraphicsPipeline
├── initialize() → create geometry, shaders, texture, pipeline
├── handleEvent() → Escape handling
└── render(frame) → clear, calculate scale, draw quad

Application → acquire frame → QuadDemo::render(frame) → submit/present
```

The quad's persistent resources are created once, outside the render callback. Its callback supplies the six-vertex draw, scale parameters, and texture; `Application` owns presentation. Asteroids and Mini Artillery need only their existing clears and events until their scene work begins. Shape-specific renderer classes are not required to expose triangle/quad drawing; extract reusable higher-level rendering only when an example demonstrates the need.

Every shared-interface change must update affected examples in the same completed increment. Preserve target/executable names, source directory numbering, executable-relative assets, licensing, framework packaging, controls, and visible behavior unless an intended change is recorded. Do not keep examples indefinitely on an abandoned interface. Keep temporary migration scaffolding out of the completed design.

[RFC 0005](0005-camera-keyboard-and-sprites.md)'s camera/sprite work must use this boundary when implemented; its former direct uniform/viewport assumptions are not a second rendering API. Geometry, camera math, and simulation should remain independent of a graphics context where practical. Game rules and scene-specific grass/water/physics behavior remain example-owned under RFC 0003.

### Backend selection and optional capabilities

Prefer one backend selected at build time for each initial port. Compile its matching window setup, platform integration, resource implementation, and factory together. Reject unavailable backend/platform choices at configure time. Supporting several implementations in the repository does not require loading or switching between them in one running application.

Runtime selection, if later required, must choose graphics window requirements before window creation and define how assets, device resources, and presentation are recreated. Do not add an enum that suggests unsupported choices work today. The public facade and private dispatch boundary do not themselves provide runtime switching.

Keep the conventional draw contract shared. Introduce optional capabilities and limits only with implemented operations and a focused example. Distinguish acceleration structures, ray queries, ray-tracing pipelines, and mesh/task stages where APIs differ; unsupported work must be diagnosed or deliberately replaced by an example-selected alternative. Do not silently approximate a requested feature, claim dedicated hardware acceleration, or promise a feature merely because a native API has a similarly named facility. Browser graphics and WASM-compatible dependencies require a separate decision.

## Alternatives

| Alternative | Tradeoff and decision |
| --- | --- |
| Public window/resource subclasses for each API | Exposes native distinctions throughout examples and mixes window state with graphics lifetime. Use shared concrete facades and private backend/surface implementations. |
| One `Window` friend for each concrete surface | Requires changes to the generic window whenever a backend is added. Keep the abstract surface as the sole privileged access boundary. |
| Expose the abstract backend as the public device | Saves a forwarding layer but spreads implementation concerns into callers. Keep shared validation and lifetime rules in a concrete facade. |
| Expose only shaders and let examples set remaining native state | Makes Metal/Vulkan ports require different drawing sequences and leaves state leakage possible. Use one logical pipeline plus explicit engine pass/draw inputs. |
| Mirror Vulkan pipeline creation exactly | Provides a detailed native model but forces its version/feature and layout decisions on other APIs. Define supported engine semantics and map them privately. |
| Duplicate complete descriptions for every shader | Simple initially, but risks accidental state differences. Reuse a common description for compatible shader variations. |
| Direct or compile-time-only implementation with no private interface | Viable for one API, but the selected private dispatch boundary makes the backend contract explicit. Keep its operations coarse and measure overhead before expanding it. |
| Implement all backends and features immediately | Exposes differences early but combines several learning projects and speculative interfaces. Implement and validate one selected increment at a time. |
| Adopt an existing abstraction such as SDL GPU | Could reduce native backend maintenance. The selected learning direction is to own a small boundary and study native APIs later; adopting a library remains a separate decision. |

## Risks and limitations

Only OpenGL has exercised the interface. Hidden shader, coordinate, binding, or synchronization assumptions may require a coordinated API revision during the second port. The native comparison describes a feasible mapping, not proof that the current interface is portable unchanged.

A logical pipeline can establish state without making arbitrary shaders visually interchangeable. Incompatible layouts and attachment usage must fail validation. Coordinate conversions, color space, and shader behavior can still cause differences when state fields match. Native precision and rasterization differences require justified correctness tolerances.

The private boundary adds dispatch and resource bookkeeping. Resource identities, frame references, borrowed device lifetime, pending GPU work, and partial startup need concrete checks; uniform public names do not solve those lifetimes. Keep dependency versions, origins, supplied SDKs, and native/web build separation under [RFC 0001](0001-local-dependencies.md). This RFC does not establish new platform support or authorize dependency changes.

### Performance expectations and evidence

There is no OpenGL-versus-Metal benchmark for Frostwind because only OpenGL is implemented. Metal remains a future option for studying native Apple GPU submission/resource control; changing APIs alone does not establish a speedup or fix expensive shading, excessive geometry, or overdraw. The single quad is too small to justify an API migration on visual performance grounds. Shader layout padding is a data-contract choice, not a demonstrated optimization.

Profile the active example before changing architecture. If a second backend is implemented, compare the same scene, resolution, assets, shader work, and drawing quality on the same hardware. Separate startup/shader compilation from steady-state frame measurements, warm up the workload, and record CPU rendering time, GPU time, and frame-time variation. Disable both software FPS pacing and VSync for throughput comparisons, then separately check normal paced presentation and responsiveness. Keep validation/capture overhead and hardware/settings in the measurement record. Do not infer a numerical speedup or portable performance from API names or a capped FPS counter.

## Validation

- Configure/build the native engine, all three examples, and a fresh engine-only configuration.
- Exercise Textured Quad with its original texture, geometry, filtering, orientation, scaling, and executable-relative asset lookup, including launch from outside the repository.
- Exercise both scaffold backgrounds, resizing/Retina drawable sizing, minimization/restoration, Escape/close results, VSync, and software pacing.
- Check factory/window/device/resource initialization failures, invalid shader/image inputs, callback exit/failure/exception paths, abandoned frames, partial initialization, and resource cleanup before context/device/window destruction.
- Check graphics window setup before context creation, factory/compiled-backend agreement, and cleanup if the context exists but GLAD initialization fails. Verify `Window` has no context/presentation ownership or public native-handle getter, and grants graphics friendship only to the private `WindowSurface` base.
- Check stale/wrong-device resource rejection, parameter-block size/layout mismatches, required bindings, parameter copying between successive draws, operations on completed frames, and failure during frame submission/abandonment. Confirm shader-wrapper cleanup cannot invalidate a live pipeline.
- Inspect resource uploads and a rendered frame through focused GPU readback checks where practical; keep temporary outputs under ignored `build/`. Screenshots remain owner-supplied.
- Search for GLAD includes and direct OpenGL calls in examples/public rendering headers and ensure they are confined to implementation code. Verify no engine dependency on example assets/code.
- With the second backend, repeat the example/lifecycle checks and exercise resize/recreation, pending uploads, multiple frames in flight, and resource retirement under native validation tools. Document justified image tolerances rather than assuming byte-identical rendering across APIs.

When pipeline states and additional backends are implemented, also apply these acceptance checks; they have not been executed by this documentation update:

- Draw A → B → A with intentionally different supported blend/depth/cull states, checking that B leaves no residual state in A. Repeat across frame/pass boundaries and after resizing.
- Create two compatible shader variants from one shared description and verify that non-shader configuration remains equal. Reject incompatible vertex/resource/output interfaces with diagnostics appropriate to the supported shader tooling.
- Verify depth occlusion, depth testing without writes, clearing, and supported stencil behavior as each becomes available. Reject missing/incompatible attachments and mismatched formats/sample counts before issuing work.
- Run the same scene semantics on each implemented backend, using known transforms, winding, asymmetric textures, and documented image tolerances. Keep performance measurements separate from correctness checks; preserve the existing examples throughout migration.

Report checks actually executed and remaining manual/platform checks. These are implementation acceptance criteria, not results of revising this RFC. Existing executed OpenGL checks and remaining manual checks are recorded in the [development guide](../development.md#shared-rendering-validation-2026-10-19).

## Implementation plan

| Stage | Scope and completion evidence | State |
| --- | --- | --- |
| 1. Shared OpenGL foundation | Separate window/device/surface ownership; shared frames/resources/pipelines; all three examples migrated; native and engine-only validation recorded | Implemented; remaining manual/platform checks documented |
| 2. Exercise conventional pipeline state | Use a selected OpenGL 3D scene to specify needed vertex/index formats, matrices, depth attachment/test/write/compare, and culling; verify state transitions and preserve existing examples | Planned, through example requirements |
| 3. Specify the second backend | Select concrete examples and API/OS baselines; settle shader artifacts, coordinates, binding layouts, surface/device ownership, completion and retirement; record a backend-specific RFC | Deferred until selected |
| 4. Metal port on macOS | Implement private Metal backend/surface and build selection; map one logical pipeline to native pipeline/depth-stencil/encoder state; validate equivalent example semantics and lifecycle | Planned |
| 5. Vulkan port on Windows/Linux | Specify SDK/version/features, shader artifacts and queue/swapchain lifetimes; map engine state to pipeline/dynamic commands; verify each claimed platform independently | Planned |
| 6. Optional extensions | Add a capability, richer pass/binding model, or runtime selection only with a concrete requirement and validation | Future work |

A stage does not silently retire OpenGL or make the next stage the active task. The second backend may revise the interface, but must preserve its supported documented meaning and migrate examples alongside those changes. Metal then Vulkan is the intended backend learning order; adjust it explicitly if available systems or the active objective change.

### Current learning plan: OpenGL first

The active learning objective remains a small OpenGL 3D scene followed by a bounded grass experiment. The detailed steps, observable completion criteria, and distinction between wind animation and simulation are in the [roadmap's current focus](../../ROADMAP.md#current-focus---opengl-scenes). This RFC owns the rendering architecture; the roadmap owns experiment sequencing.

Keep the existing boundary and extend it for those scenes. Designing the multi-backend contract does not make Metal/Vulkan implementation, shader cross-compilation, generalized materials, or runtime switching the next task. Asteroids, Mini Artillery, water, and physics remain separately selectable work. Reconsider the next backend when native API learning is selected, using working scenes as the porting baseline.

## Open questions

- Which world, clip/depth, winding, framebuffer, and color-space conventions should the first 3D camera establish? Define conversions alongside the relevant rendering implementation.
- Which depth/stencil formats, sample counts, explicit dynamic values, and pass operations does the first example actually require?
- What API/OS baselines, native shader formats, compiler tooling, and resource-binding layouts will the Metal and Vulkan ports use?
- Which device/queue, presentation, in-flight reference, and failure-recovery details belong in each backend-specific RFC?
- Which shared bindings or frame operations must change after a second real backend tests the current contract?
- Should OpenGL remain a maintained reference after Metal works, or be retired deliberately? Record that choice separately; adding a backend does not decide it.

## Decision

Created on 2026-10-18 and accepted for staged implementation on 2026-10-19. The shared OpenGL window/device/surface, frame, and resource extraction and all three example migrations were completed on 2026-10-19. The owner retained the OpenGL-first learning priority and simplified Textured Quad into `QuadDemo` on 2026-10-20. Completed checks and remaining manual validation are recorded in the development guide and changelog.

Revised on 2026-10-20 at the owner's request to make multi-backend structure, pipeline consistency, shader variation, and the native OpenGL/Metal/Vulkan comparison the core of this RFC. The agreed design uses shared public facades, private native backends, one privileged abstract surface boundary, and one logical pipeline with explicit pass/draw responsibilities. Detailed scene/grass learning steps now live in the roadmap.

Deferred on 2026-10-20 following the owner's decision to focus on OpenGL and avoid the maintenance and learning cost of multiple backends. Renamed from RFC 0006 to reserved RFC 9999 so the research sorts after active proposals. The existing OpenGL setup remains implemented; its preservation does not require maintaining a universal backend contract. Metal/Vulkan implementation and further portability infrastructure remain deferred until explicitly reconsidered. The analysis above is preserved for reference.
