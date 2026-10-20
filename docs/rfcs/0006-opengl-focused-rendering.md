# RFC 0006: OpenGL-focused rendering

- **Status:** Implemented
- **Created:** 2026-10-20
- **Supersedes:** None
- **Superseded by:** None

## Problem

The multi-backend experiment introduced a device facade, abstract backend/surface, opaque resource registry, pipeline descriptions, and scoped frames for a renderer currently used by one textured quad and two window scaffolds. These layers obscured the relationship between OpenGL programs, uniforms, textures, and vertex buffers and imposed unexercised portability decisions on a learning project.

The owner chose OpenGL-focused classes with direct responsibilities: Texture2D performs texture calls, ShaderProgram owns an activatable linked program and uniform operations, and Window owns SDL/OpenGL integration. Examples should remain straightforward and runnable through engine operations.

The prior experiment is saved at commit `e9cf616` on `archive/multi-backend-experiment`. [RFC 9999](9999-multi-backend-rendering-research.md) preserves its design and pipeline comparison as deferred research. This RFC records the new implemented direction rather than rewriting that research into current requirements.

## Goals and scope

- Make the engine's OpenGL ownership and call flow easy to follow.
- Give each resource owner direct OpenGL operations and automatic cleanup.
- Restore a linked ShaderProgram with source/file loading, activation, uniform lookup, and typed writes.
- Keep vertices/layout separate from shaders so geometry can be reused with compatible programs.
- Restore Window ownership of SDL video/window/context, GLAD, VSync, and presentation.
- Preserve the existing examples' images, filtering, orientation, scaling, controls, assets, and teardown.

This change adds only a concrete clear/triangle Renderer, not a model/material system, GLM, cameras, lighting, depth/stencil storage, indices, instancing, another graphics API, or browser support. OpenGL 3.3 Core remains the baseline. Multi-backend compatibility does not constrain these interfaces.

## Proposed design

### Ownership and structure

```text
QuadDemo : Application
├── VertexBuffer → owns VBO and VAO
├── Texture2D → owns GL texture
├── ShaderProgram → owns linked GL program
└── base Application
    ├── Window → owns SDL video/window and GL context
    └── Renderer → persistent clear/draw service borrowed by callbacks

Rendering:
Application → drawable size / current context / viewport
            → example render(renderer, width, height)
            → renderer clear / program use / uniforms / texture bind / renderer drawTriangles
            → Window present / remaining frame budget
```

Engine graphics calls remain in implementation files. Resource headers expose primitive types and private handles without requiring GLAD. Examples choose assets and parameters; the engine does not depend on example code. The root static library, pinned dependencies, executable-relative assets, and framework packaging remain unchanged.

Delete the device/frame/pipeline/backend/surface abstraction and its resource identities. `GraphicsChecks.h/.cpp` provides only private context/error checks shared by the direct owners. This helper does not dispatch resource operations or manage lifetime. A concrete Renderer handles clear/draw submission. Application owns that persistent renderer and performs viewport setup directly. There is no backend interface or per-frame renderer allocation.

### Window and Application

Window sets context attributes before SDL window creation, creates/makes current its context, loads GLAD through SDL, and requests the configured swap interval. `WindowConfig` owns `vsync` again. Startup uses a scoped candidate so failures release completed context/window/video stages and permit retry. Successful repeated initialization is rejected. Native handles remain private with no graphics friend classes.

Application initializes Window before calling the derived initializer, supplies its persistent renderer and current drawable pixels to `render(Renderer& renderer, int width, int height)`, and presents only when it returns continue. It makes the context current and sets the viewport before the callback. Success/failure returns and exceptions skip presentation; already issued GL operations are not rolled back. Resource initialization/use, Image loading, and rendering operations throw `std::runtime_error` with diagnostics. Commands return `void`; SDL callback boundaries log once and convert exceptions to `SDL_APP_FAILURE`, skipping presentation. Window retains checked boolean SDL results.

Retain application-owned software pacing from [RFC 0004](0004-frame-pacing.md), including render/swap time, independent VSync, capped/uncapped operation, and no catch-up. Minimized windows use a 16 ms delay. Zero-sized targets use the remaining frame budget or 16 ms when uncapped. Fixed simulation scheduling remains future work.

### Texture2D and Image

Texture2D owns a native texture and calls OpenGL directly for creation/upload/binding/deletion. Validate positive dimensions, exact RGBA8 byte count, overflow, and the current context's maximum texture size. Upload preserves the previous binding on the active texture unit and pixel-unpack alignment, row length, skips, and unpack-buffer binding. Failed upload deletes its partial texture and permits retry.

Keep nearest filtering, clamp-to-edge wrapping, and no mipmaps. `bind(unit)` selects a validated unit and this texture's sampling without exposing its handle. The texture is independent of shader/program ownership; a sampler uniform selects the same unit.

CPU Image loading retains the pinned PNG/JPEG decoder, preflight dimension/overflow checks, local bottom-row-first conversion, and independence from a GL context. Texture file loading queries the current GL dimension limit and composes Image with the raw upload. Do not restore static texture-support setup or process-global decoder flip state.

### ShaderProgram and uniforms

ShaderProgram compiles vertex/fragment sources, links one program, and releases temporary stages. Retain source/file entry points and compiler/linker/path diagnostics. Failure cleans partial resources and permits retry. There is no separate public Shader stage or GraphicsPipeline wrapper.

`use()` activates the program. `uniformLocation(name)` returns a cacheable program-local integer, and throws for a missing/inactive required uniform. Setters cover integer, float, vec2/3/4, and a mat4 supplied as sixteen column-major floats. They require this program to be active, reject a negative location/null matrix input, and report native errors such as uniform type mismatches. Setters do not automatically activate a program. Callers must keep cached locations associated with their program; integer locations alone do not carry owner identity.

The quad uses `uniform vec2 uScale` and an integer texture-unit sampler. It no longer needs a padded std140 block or uniform-buffer allocation. Uniform blocks may be added for a demonstrated use; they are not required for ordinary parameters. Future camera matrices and light values can use the typed methods without adding a math library to this change.

### VertexBuffer and Renderer

VertexBuffer owns a VBO and VAO with explicit interleaved float attribute location/component count/offset/stride. Initialization validates bounds, duplicate locations, and GL limits and consumes input bytes before returning. It restores previous VAO/array-buffer bindings. Scoped failure cleans partially created storage/layout.

VertexBuffer exposes `bind()` and `vertexCount()`; it does not issue draw commands. Binding validates the creating context without exposing its native handles. The program's vertex input interface must match the caller's layout; no general reflection or resource-binding metadata is required. One geometry owner can be used by several compatible programs.

Renderer owns no GPU resources, needs no initialization, and operates immediately on the current context. `drawTriangles(vertices, count, first)` borrows geometry, validates a nonempty triangle-list range and active program, binds the buffer layout, issues the native draw, and restores the prior VAO even when binding fails. Shader programs, uniforms, and textures are explicitly selected by the example. The renderer retains no geometry or callback references.

`Renderer::clear(Color)` covers the full color target independently of scissor/color masks. Application iteration sets the viewport directly before calling `render(renderer, width, height)`; Window owns context management and presentation. Failures throw to the callback boundary; drawing does not implicitly reset all GL state. Future depth/blend/culling controls and indexed/instanced drawing arrive when an example needs them.

Application supplies the same Renderer instance by reference on every callback. Examples do not own another renderer, copy it per frame, or retain the borrowed reference. Mesh/Material/Model loading and high-level model drawing remain a possible later extension, not implemented capabilities. Renderer is named for that broader responsibility without adding speculative model APIs now.

### Lifetime and failure handling

Operations throw diagnostic `std::runtime_error` exceptions rather than returning success flags. Standard allocation/filesystem exceptions also propagate. Required uniform lookup throws for an absent/optimized-out name; valid lookup still returns an integer. Destructors and binding-restoration guards never throw. Destruction with an unavailable creating context logs the lifetime violation and skips deletion. Scoped candidates release partial GPU objects during unwinding; texture/vertex uploads restore their promised state before checking GL errors, and a scoped guard restores the prior draw VAO even on binding failure. Owners remain retryable after failed initialization.

GPU owners are non-copyable. They record the creating context and require it to remain current and outlive initialization/use/destruction. Context checks diagnose foreign-context use but do not switch contexts, retain Window, or implement context-loss recovery. Resources cannot survive context recreation.

Application stores Window before Renderer so the renderer dies first; derived GPU members die before both. The base Window destroys its context, SDL window, and video initialization. This order also applies after partial example startup. The current scope is one window/context on SDL's callback thread. Native GL object deletion semantics protect outstanding work; no engine resource registry, shared frame references, or speculative retirement queue is required.

## Alternatives

- **Keep the universal abstraction:** Already working, but adds indirection and a binding/pipeline model that competes with learning OpenGL. Preserve it on the archive branch instead of requiring it in current code.
- **Issue native calls throughout examples:** Makes the GL operations visible but spreads ownership/error handling into examples. Keep direct operations in focused engine owners, with examples supplying geometry/shaders and scene behavior.
- **Combine shader and geometry into one class:** Shortens a single quad but prevents straightforward reuse with multiple shaders. Keep linked programs and vertex storage independent.
- **Move all initialization back into Application:** Simplifies one startup path but weakens class responsibilities. Window initializes its own context/loader; resource owners initialize their own objects.

## Risks and limitations

Explicit state is easier to inspect but callers must choose the correct program, uniform locations/types, texture units, and attribute layouts. Catching a required rendering exception and returning continue can present a partially rendered target; let it reach the callback boundary unless recovery is deliberate. Program-local integer locations and native context pointers are lightweight contracts, not ownership-safe handles. The documented application/context lifetime and current-context requirements remain necessary.

The API is intentionally OpenGL-specific. Another backend would need a new design and coordinated example migration; preserving research does not guarantee reuse unchanged. Windows/Linux runtime and browser integration remain unverified. This change establishes no speedup, new dependency, release, or tag.

## Validation

Build all three examples and a fresh engine-only configuration. Exercise actual example entry points from outside the repository, compare pre-swap framebuffer hashes with the archived baseline, and check synthetic Escape/close exits. Check source boundaries and documentation links.

Verify shader compilation/link/file failures and retry, typed uniforms including matrix values, two compatible programs sharing geometry, missing locations and wrong-active-program failures, texture limits/bytes/orientation, unpack-state preservation and injected upload failure/retry, vertex layout/range validation, and context guards.

Inject Window/GLAD/startup/presentation failures, check callback results/exceptions, and verify texture deletion before context destruction and context before window/video teardown. Check drawable resize and controlled-clock pacing. Record remaining interactive/minimize checks and other platform limitations. Screenshots remain owner-supplied.

## Implementation plan

1. Preserve the previous experiment in Git and deferred RFC 9999.
2. Move SDL/context/GLAD/presentation into Window and restore dimension-based Application callbacks.
3. Implement direct Texture2D and ShaderProgram ownership; keep CPU Image loading.
4. Replace device-backed geometry/pipeline submission with storage/layout-only VertexBuffer and concrete Renderer clear/draw operations, passed by reference by Application. Keep viewport setup in Application.
5. Migrate all three examples together, simplify the quad uniforms, and delete obsolete abstraction files.
6. Validate output/failure/lifetime behavior and align usage/architecture documentation.

## Open questions

Choose depth formats/testing, indexed meshes, math/camera support, and richer sampling when the small 3D example needs them. Choose higher-level renderer conveniences through demonstrated reuse. Additional backends remain deferred research.

## Decision

Accepted and implemented on 2026-10-20 following the owner's instruction to simplify the engine for OpenGL. Texture2D, ShaderProgram, and Window now own their direct native responsibilities; VertexBuffer owns geometry, Renderer submits clears/draws, and Application passes its persistent renderer to callbacks while owning viewport/presentation coordination. Removed the device/backend/surface/pipeline/frame/resource-registry layers.

Native example and fresh engine-only builds passed. Actual example callbacks retained byte-identical framebuffer hashes, and focused direct-resource, context, lifecycle/fault, and controlled-pacing checks passed. See the [development verification record](../development.md#opengl-focused-simplification-validation-2026-10-20) for executed evidence and remaining manual checks. The archived branch is unchanged.

Clarified that day after the owner's review: drawing belongs to Renderer rather than VertexBuffer, and clearing belongs alongside drawing. The persistent renderer is owned by Application and borrowed as an explicit callback parameter. No model loader, material hierarchy, or universal backend interface was added.

Revised that day to use exception-based rendering failures: direct command sequences replace boolean checks in examples, with reporting at existing SDL callback boundaries. Window boolean integration and callback control-flow results remain unchanged. See the [exception validation record](../development.md#exception-based-rendering-validation-2026-10-20) for executed checks.
