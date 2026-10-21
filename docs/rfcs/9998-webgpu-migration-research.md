# RFC 9998: WebGPU migration research

- **Status:** Deferred research
- **Created:** 2026-10-20
- **Supersedes:** None
- **Superseded by:** None

**Current direction (2026-10-21):** WebGPU is deferred research, alongside [RFC 9999's multiple-backend investigation](9999-multi-backend-rendering-research.md). [RFC 0006](0006-opengl-focused-rendering.md) remains the active implemented rendering decision. Continue C++17/OpenGL scene learning, basic lighting, and Asteroids according to the [roadmap](../../ROADMAP.md#current-focus---opengl-scenes).

The design below preserves the migration investigation and its WebGPU practices for future evaluation. Its Dawn/C++20 choices, replacement owners, implementation plan, and OpenGL retirement criteria are conditional research, not current requirements. No dependency import, toolchain upgrade, renderer replacement, or platform-support change is selected. Revisit the design when a concrete learning objective, browser requirement, compute experiment, or measured limitation justifies it.

The reserved number **9998** places this research beside RFC 9999 at the end of the index. Formerly RFC 0008, it retains its creation date, design analysis, and decision history; 0008 is available for the next regular proposal.

## Problem

Frostwind currently uses SDL3/OpenGL on macOS arm64. Its direct OpenGL owners make the relationship between programs, uniforms, textures, vertex arrays, and draws easy to learn. The owner now wants to evaluate migration to WebGPU for standalone native applications and a later browser build.

This is a rendering architecture change, not a loader substitution. `Window` owns a GL context, GLAD, VSync, and swap; `Application` calls `glViewport`; resources record their creating context. `ShaderProgram` exposes linked-program activation and integer uniform locations, and `VertexArray` owns a GL VAO. These contracts do not map directly to WebGPU pipelines, bind groups, command encoding, or asynchronous errors.

The reviewed baseline includes the reusable CPU-only `OrbitCameraController`, example-owned key bindings and animation, trilinear/generated mipmaps and anisotropy, the indexed quad/cube, and two clear-only scaffolds. The cube currently requests an 800×600 logical window; retain that example-owned setting independently of drawable-pixel sizing. README previews distinguish owner-supplied screenshots from planned-game concepts, including the updated cube capture with its earlier image preserved. Preserve that behavior, ownership, artwork, and implementation-status distinction.

The [README](../../README.md), [structure guide](../project-structure.md), and [roadmap](../../ROADMAP.md) describe OpenGL as the implemented baseline. WebGPU was briefly selected as the active focus on 2026-10-20, then deferred on 2026-10-21 because the current examples do not require a renderer migration. OpenGL remains active; lighting and Asteroids do not depend on this proposal.

## Goals and scope

- Replace the engine's OpenGL rendering path with one concrete WebGPU implementation, beginning with standalone macOS arm64 through Dawn's Metal backend.
- Preserve one root CMake project, the static `frostwind` target, all four example target/output names, SDL callbacks, input/update behavior, assets, and executable-relative loading.
- Keep rendering concepts visible: WGSL modules, pipelines, buffers, texture/sampler bindings, a render pass, and queue submission.
- Preserve the quad's appearance and the cube's transforms, orbit controls, sampling, depth/culling, 4× MSAA, VSync request, and 120 FPS software ceiling.
- Retain CPU math, controller, image decoding, geometry, and future simulation independence from graphics initialization.
- Establish a later Emscripten/WebGPU route without claiming browser or Windows/Linux support from a macOS result.

The migration excludes new lighting, gameplay, compute experiments, multiple windows, a render graph, automatic shader reflection, GLSL cross-compilation, resource registries, custom Metal/Vulkan/D3D12 implementations, runtime graphics-API selection, and transparent device-loss recovery. Performance improvement is a measurement question, not an acceptance promise.

## Proposed design

### 1. API, language, and dependencies

Select **Dawn** as the native implementation. Use its C++ WebGPU RAII bindings in engine rendering owners and WGSL shaders in examples. Native macOS selects Metal; future Windows and Linux work can use Dawn's D3D12 and Vulkan implementations after their SDL SDKs and test systems exist. These are Dawn backends, not new Frostwind backend classes. [Dawn documents its native implementations and C/C++ interface](https://dawn.googlesource.com/dawn/+/refs/heads/main/README.md).

Move project-owned targets to **C++20** during dependency integration. [Dawn's current CMake quickstart](https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/quickstart-cmake.md) requires a C++20-capable compiler and uses C++20 for its consumer example. A dependency build requirement alone does not prove every C API consumer needs C++20; this proposal chooses a consistent supported C++ toolchain rather than promising C++17 compatibility for the selected C++ bindings. This was the investigated migration policy; current conventions, AGENTS, and CMake retain C++17. Any future language-policy change requires a renewed decision.

Preserve [RFC 0001](0001-local-dependencies.md):

- Import a matched, exact Dawn revision, headers, licenses, required transitive dependencies, and provenance under `external/dawn/`. Prefer a supplied macOS arm64 static SDK with reproducible build records, keeping its matched headers and archives together. Record compiler, SDK/deployment target, Debug/Release configuration, linkage, enabled backends, patches, checksums, and reproduction commands before landing the import.
- Integrate through `cmake/Dawn.cmake` and an explicitly local imported target. Fail clearly for missing/mismatched SDKs. Disable automatic dependency fetching; neither normal configure nor build may download missing dependencies. Upstream quickstart fetching instructions are not our repository integration policy.
- Put project-generated builds, dependency rebuilds, and experiments under ignored `build/`. Intentionally supplied SDK archives under `external/` are imported dependency inputs, not project build outputs.
- Retain SDL3 3.4.18's official universal dynamic framework, packaged headers, symlinks, license, and per-example copying. Do not rebuild SDL on macOS. Retain GLM 1.0.3 and stb_image 2.28.
- Request only features/limits needed by the two examples and log the actual adapter/backend and relevant limits. Keep native-only Dawn extensions out of shared shader/resource code.

An exact Dawn revision is deliberately unresolved pending integration: the integration spike must prove the selected SDK, toolchain, surface API, and prospective Emdawnwebgpu header compatibility before recording the pin. Do not use `main`, `latest`, or an unversioned system package as the implementation dependency.

### 2. Ownership and source layout

Keep `Application` as the lifecycle coordinator and grow its existing persistent `Renderer` into the concrete owner of WebGPU instance, adapter, device, queue, surface, attachments, and transient command state. This avoids adding a second device facade for one renderer. Pipeline and binding owners are now justified by actual WebGPU objects; this explicitly revisits RFC 0006 without restoring RFC 9999's backend hierarchy.

| Component | Proposed responsibility |
| --- | --- |
| `Application` | Own Window before Renderer; coordinate pending startup, events, updates, acquisition, render callbacks, submission/presentation, and pacing |
| `Window` | Own SDL video/window and macOS SDL Metal view; query pixel size/minimized state; privately connect its platform view to WebGPU surface creation |
| `Renderer` | Own WebGPU connection and surface configuration, service callbacks, own depth/MSAA targets and frame encoders, validate draw compatibility, submit and present |
| `ShaderModule` | Own a WGSL module, source/file loading, labels and compilation diagnostics; no program activation or uniform locations |
| `PipelineLayout` | Own an explicit ordered set of bind-group layouts and their pipeline layout; permit reuse by compatible pipelines and BindGroups |
| `RenderPipeline` | Own a concrete WebGPU render pipeline created from a RenderPipelineDescription; retain layout and vertex/target/depth/sample compatibility |
| `VertexBuffer` / `IndexBuffer` | Own vertex bytes/u32 indices, counts and index-maximum metadata; require an explicit Renderer during initialization |
| `Geometry` | CPU descriptor borrowing buffers, with vertex layout and derived counts; replaces the GL-specific VertexArray owner |
| `UniformBuffer` | Own a fixed-size uniform allocation and validated queue writes; examples define the actual parameter records |
| `Texture2D` | Own RGBA8 texture, view, sampler, uploads, and optional mip generation; no texture-unit binding |
| `BindGroup` | Own bindings for a group in an explicit PipelineLayout, including uniform buffers and texture/sampler entries; independent of one pipeline instance |
| `Image`, cameras, controller, Keyboard | Retain CPU responsibilities; amend only projection/texture conventions explicitly described below |

Keep headers beside implementations. Add `ShaderModule`, `PipelineLayout`, `RenderPipeline`, `Geometry`, `UniformBuffer`, and `BindGroup` under `engine/src/rendering/` when used; keep `RenderPipelineDescription` beside RenderPipeline. Remove `ShaderProgram`, `VertexArray`, and GL-only `GraphicsChecks` after migration. Replace checks with private WebGPU diagnostics/device-identity helpers as needed. There is no abstract surface interface, backend folder tree, universal material layout, opaque resource ID, or public `RenderFrame` object.

Public owners may store private `wgpu::` RAII members and thus include matched WebGPU headers. CMake must propagate that include requirement, but examples continue to call Frostwind operations and receive no device/queue/native-window handle getters. Resource initialization takes `Renderer&`; narrow friendship/internal helpers permit device access and device-identity validation without a public device escape hatch. Do not introduce a heap implementation object for every small owner solely to hide dependency types.

### 3. SDL window and macOS surface

Create a resizable high-density SDL window for Metal instead of requesting `SDL_WINDOW_OPENGL`. Window owns the view created by `SDL_Metal_CreateView`; use `SDL_Metal_GetLayer` to create Dawn's Metal-layer surface. SDL documents both [view creation](https://wiki.libsdl.org/SDL3/SDL_Metal_CreateView) and [layer access](https://wiki.libsdl.org/SDL3/SDL_Metal_GetLayer).

Use a narrow private `Window::createWebGpuSurface(instance)` operation callable by Renderer to perform the platform connection; its exact types follow the pinned SDK. This is an explicit, limited friendship exception to the current Window policy. Keep SDL handles and the layer private. Put platform-specific surface code under `platform/`, adding an Objective-C++ translation unit and Apple frameworks only if the pinned integration requires them; SDL's opaque layer API may suffice without Objective-C++.

Destroy/release the WebGPU surface before destroying the Metal view, then call `SDL_Metal_DestroyView` before destroying the SDL window/video subsystem. There is no GL context, make-current call, GLAD initialization, or SDL GL swap in the final path.

### 4. Startup, callbacks, and frame flow

Adapter/device requests and GPU diagnostics are asynchronous. Use explicit startup states: window ready, adapter pending, device pending, example initialization, validation pending, ready, failed, and stopping. `onInit` publishes the Application after starting requests and returns continue; subsequent SDL iterations service completion and advance states. Call example initialization exactly once after device/surface setup. Do not update/render the scene or report ready while required startup validation is pending. Events and close requests remain responsive throughout startup.

Change the example hook to `initialize(Renderer&)` so resources have an explicit creating device. Retain `render(Renderer&, int width, int height)` and `update(f32 deltaSeconds)`. Reset timing and stale input when startup completes, giving the first active update delta 0. Earlier synchronous startup failure still leaves appstate null; failure after publication returns SDL failure and cleans up through onQuit.

Proposed first implementation supports one color render pass per frame:

```text
SDL iteration → process GPU completions/errors → finish pending startup
              → query pixels / skip minimized or zero-sized target
              → example update → acquire surface texture / create encoder
              → example render:
                  renderer.beginPass(clear settings)
                  update parameter buffer(s)
                  renderer.setPipeline(pipeline)
                  renderer.setBindGroup(group index, bindings)
                  renderer.drawTriangles(geometry)
              → end pass → finish/submit commands → present → remaining FPS budget
```

Application owns frame begin/finish/abort through Renderer; examples never submit or present. A scaffold simply begins its clear pass. `beginPass` selects color/depth clear values before encoding draws and rejects a second pass in this initial interface. Calling draw/bind operations outside the appropriate frame/pass is an engine error. A successful render must have begun its pass. Viewport/scissor cover the current drawable extent by default. Application no longer includes graphics API headers or issues viewport calls.

Update exit/failure skips acquisition/render/submission/presentation. Render exit/failure or exception discards the unsubmitted encoder, releases the acquired frame texture, and skips presentation. Queue writes already issued are not rolled back. A detected device error prevents further submissions; errors arriving after submission cannot undo that work. These are explicit replacements for the immediate GL failure contract.

### 5. Pipelines, geometry, and shader parameters

#### RenderPipelineDescription follows WebGPU

Adapt the pipeline description to WebGPU's actual state model. `RenderPipelineDescription` is a CPU description consumed by the concrete RenderPipeline owner, with fields corresponding directly to the supported subset of [GPURenderPipelineDescriptor](https://gpuweb.github.io/types/interfaces/GPURenderPipelineDescriptor.html). It is not a list of OpenGL state changes or a backend-neutral schema. Keep defaults explicit and initially support only the operations required by the quad/cube and mip generator.

| Description field | Initial contract |
| --- | --- |
| `label` | Diagnostic name identifying the example and pipeline variant |
| `layout` | Explicit PipelineLayout; ordered group slots, no inferred layout |
| `vertex` | ShaderModule, named entry point, vertex buffer stride/step mode and attribute format/location/offset |
| `fragment` | ShaderModule, named entry point, one color target format, explicit blend state and write mask |
| `primitive` | Triangle list, explicit CCW front face and cull mode; default clipping, no strip-index configuration |
| `depthStencil` | Optional depth format, comparison and write enable; stencil disabled in this increment |
| `multisample` | Count 1 or 4, full sample mask and alpha-to-coverage disabled |

Texture dimensions, concrete texture views, clear colors, attachment load/store operations, resolve targets, viewport/scissor, bound geometry, and uniform contents are not pipeline fields. They belong to resource creation, pass/frame setup, or draw commands. Resize changes attachment extents without changing pipeline state.

For the cube, the effective description is conceptually:

```text
label: Textured Cube / opaque
layout: [textureAndSamplerLayout, cubeParametersLayout]
vertex: cubeModule / vsMain / stride 20 / vertex step
        location 0 = float32x3 at offset 0
        location 1 = float32x2 at offset 12
fragment: cubeModule / fsMain / current surface view format
          blending disabled / write RGBA
primitive: triangle-list / front CCW / cull back
depthStencil: configured depth format / compare less / write true
multisample: 4 / full mask / alpha-to-coverage false
```

This is explanatory notation, not an API already implemented. The quad instead uses stride 16 with two float32x2 attributes, no depth state, no culling, and sample count 1. The mip generator uses its own pipeline matching the destination mip's texture format and sample count 1; it does not reuse the surface-format pipeline.

#### Explicit, reusable layouts

Create PipelineLayout before pipelines/bind groups. Each bind-group entry declares binding number, stage visibility, and exactly one typed buffer/texture/sampler contract. Uniform entries declare their minimum binding size and disable dynamic offsets initially. Texture entries declare sample type, view dimension, and multisampled state; sampler entries declare filtering/non-filtering/comparison type. Validate actual resources against these declarations. These map to [WebGPU bind-group layout entries](https://gpuweb.github.io/types/interfaces/GPUBindGroupLayoutEntry.html).

Allow compatible pipeline variants to reuse the same PipelineLayout and BindGroups. Initialize BindGroup with a layout and group index rather than making it belong to one pipeline. Explicitly retain the required layout references; CPU descriptions borrow their inputs only during initialization, while asynchronous creation retains/copies all modules, layout handles, strings, arrays and callback state it needs. Source owners must remain valid under the documented borrowing rules.

Use explicit layouts in the migrated examples and internal mip generator. Automatically inferred layouts are useful for isolated experiments, but their bind groups are pipeline-exclusive; implicit layout changes can also follow shader edits. The initial example convention puts stable texture/sampler bindings at group 0 and per-draw parameter bindings at group 1. Later frame/material/object groupings should follow actual reuse and binding-change frequency, without reserving empty groups or imposing a universal layout.

#### Creation, reuse, and pass compatibility

Create the required pipelines asynchronously during startup and wait for their completion through the existing pending-state flow before marking the scene ready. Do not compile shaders, create pipelines/layouts/bind groups, or allocate attachments in ordinary draw callbacks. Resize may recreate attachments; changing a buffer's contents does not require a new bind group. This applies [WebGPU best-practice guidance on async creation, explicit layouts, sharing bindings, and object labels](https://www.khronos.org/assets/uploads/developers/presentations/WebGPU_Best_Practices_Google.pdf). That presentation predates the current API; use the pinned headers and current specification for signatures and validation rules.

Keep one owned pipeline per required variant initially. Avoid a global pipeline cache until duplicate creation is demonstrated. If a local cache becomes useful, scope it to its device and compare the complete normalized creation description, including shader revision/entry points, layout, vertex input, primitive, color/blend/write state, depth, multisampling, and any later specialization constants. Labels, attachment extent, clear values, and uniform contents do not define a variant. Do not hash raw C++ descriptor bytes or borrowed pointer addresses as semantic keys.

Before drawing, require color view format, depth attachment presence/format, and sample count to agree with the pipeline; check vertex input and group-layout compatibility too. A pipeline switch must not assume incompatible groups remain usable. Initially explicitly bind every required group after selecting a pipeline; optimize redundant bindings only when needed. Keep viewport/scissor dynamic. Blend constants and stencil reference are also dynamic in WebGPU, but remain outside the first supported subset. This follows the [WebGPU pipeline/encoding model](https://www.w3.org/TR/webgpu/#render-pipelines); the engine may validate earlier with clearer diagnostics.

Replace `setDepthTest`, `setBackFaceCulling`, `program.use`, and individual uniform setters with this description and explicit pass binding. A culling/depth/blend change requires a matching pipeline variant, not an implicit pipeline rebuild inside a draw.

#### Geometry and parameter uploads

Retain `drawTriangles(geometry)` selecting indexed or sequential drawing from optional index storage, full-geometry counts, complete-triangle-list policy, and index-maximum bounds checks. Vertex layout now participates in pipeline creation; Geometry owns no GPU VAO. Validate its layout against the selected pipeline and require all resources to belong to the same renderer/device. Buffers outlive borrowed Geometry; binding source owners outlive BindGroup use. Keep count/byte arithmetic in `usize`, validate limits before narrowing, and retain explicit byte versus element units.

Port shader assets to example-owned `quad.wgsl` and `cube.wgsl`, with vertex and fragment entry points. No runtime GLSL translation. Use group 0 for texture/sampler and group 1 for per-draw parameters in these examples only; other examples need not adopt a global binding convention. BindGroup accepts explicit typed entries matching its PipelineLayout group. Start with ordinary uniform buffers and no dynamic offsets or reflection.

The quad parameter record contains two scale floats and explicit padding to a 16-byte record. The cube contains three column-major mat4 values (model, view, projection), totaling 192 bytes. Assert C++ sizes/offsets and document matching WGSL declarations. These sizes are examples' choices, not a universal engine parameter block. Update a buffer once before its draw. Multiple draws needing different values must use distinct buffers/bind groups or a deliberately designed offset scheme later; repeated queue writes to the same allocation do not snapshot values into recorded draws.

### 6. Coordinates, textures, and attachments

Keep right-handed world/view coordinates and column-vector `projection * view * model` math. Replace `glm::perspectiveRH_NO` with `glm::perspectiveRH_ZO`: near/far NDC depth becomes 0/1. Change camera documentation and the existing near-plane test explicitly; the orbit controller remains CPU-only and behaviorally unchanged. Do not add a blanket Y flip to the projection. Validate CCW winding, framebuffer orientation, and image orientation together. [WebGPU's coordinate/pipeline differences](https://developer.chrome.com/docs/web-platform/webgpu/from-webgl-to-webgpu) require this convention change.

Preserve Image's existing bottom-row-first RGBA8 contract and the examples' bottom-left UV convention to avoid an unrelated CPU decoder change. Upload those rows unchanged and sample at `(u, 1 - v)` in the new example shaders. Apply this conversion exactly once; validate using an asymmetric diagnostic image. Keep the original crate PNGs and licensing.

Use normalized RGBA8 textures without introducing an sRGB asset conversion during migration. Choose a supported surface format/view configuration with a documented color-space policy and verify it against the current appearance; matching pixel encoding must be demonstrated, not inferred from format names. Retain nearest/clamp-to-edge/no mipmaps for the quad. For the cube, explicitly allocate/generate the full mip chain with a small internal render-based mip generator; WebGPU does not supply OpenGL's automatic mip generation. Request 16× anisotropy within the pinned API's constraints, using the required compatible linear filters and logging any fallback. Keep upload row-pitch/copy alignment handling inside Texture2D and validate odd-width images.

Move graphics settings from WindowConfig to a new `ApplicationConfig::rendering` configuration: VSync, depth enablement, and sample count. WindowConfig retains title/dimensions. Normalize the existing external value 0 (MSAA disabled) to WebGPU sample count 1; initially support only disabled and 4×, rejecting other requests with a diagnostic instead of implying the GL policy of any count >= 2 still applies.

Renderer queries surface capabilities, selects a supported format/alpha/present mode, and logs the result. Native VSync requests FIFO; disabling it requests an available non-FIFO mode and logs fallback to FIFO. The software FPS ceiling remains independent and includes CPU submission/presentation time. Do not claim presentation is immediate or that queue submission time equals GPU duration.

Depth/MSAA belong to Renderer-owned textures at drawable-pixel size. Use a supported depth format such as `Depth24Plus`; it does not promise exactly 24 physical bits. The cube has a matching 4× depth/color attachment and resolves to the single-sample surface texture. Recreate size-dependent attachments after nonzero resize and never acquire/configure for zero dimensions. A size change alone does not recreate pipelines; target format/sample/depth changes require compatible pipelines and bindings to be rebuilt.

Describe attachment load/store behavior at pass creation. Clear the frame color/depth rather than loading data the scene replaces. Store the single-sample presentation output; resolve the 4× color target and discard its multisample contents after the pass. Discard depth contents when no later pass consumes them. These are pass decisions and do not generate pipeline variants. Preserve contents only when a later implemented pass needs them.

Interpret acquisition status according to the pinned header: success renders, recoverable outdated/lost surface results schedule reconfiguration, timeout/unavailable results skip with bounded pacing, and fatal/device-loss results stop cleanly. Suboptimal success schedules reconfiguration after releasing the frame. Surface recovery must not silently recreate the device. Reset update timing/input on an acquisition pause so recovery does not accumulate movement.

### 7. Diagnostics and GPU lifetime

Replace current-context identity with creating-device identity. Use WebGPU RAII references for GPU object ownership and release them normally; do not call explicit buffer/texture destruction on resources that pending submitted work still needs. Native object reference management must follow the pinned Dawn contract. Resize releases obsolete attachment references after ending/discarding the frame; avoid per-frame `WaitIdle` or completion waits.

Collect compilation information and scoped creation errors during startup. WebGPU callbacks never throw or call example code. They store bounded diagnostics and completion state in a lifetime-safe callback context; Application services it on the SDL thread, logs a failure once, and returns SDL failure. Use an owned reference-counted callback state where callbacks can outlive the initiating stack frame; never capture a raw Application/Renderer pointer without a proven cancellation/drain guarantee. Cover callback cancellation and instance shutdown behavior for the selected Dawn revision.

Device loss and uncaptured errors are terminal for the first implementation. At shutdown, stop new submissions, invalidate callback access to the application, discard any active encoder/acquired texture, and release example GPU owners before Renderer and Window. Renderer releases attachments/pipelines/internal GPU resources, queue/device/adapter, surface, and finally instance according to their dependencies while callback storage remains valid. Destructors tolerate partial startup and do not throw. No pending callback may dereference destroyed owners. Releasing WebGPU references is distinct from explicitly destroying a device; document any shutdown drain actually required by the pinned SDK without adding a wait to normal frames.

### 8. Browser and other platform boundary

The native migration is the first completion gate. Browser integration is a separately verified follow-up, not a condition for retiring the GL renderer on macOS. Use Emscripten with a matched **local Emdawnwebgpu package**, whose C/C++ calls route to browser WebGPU. Upstream provides a [local port option without automatic downloading](https://dawn.googlesource.com/dawn/+/refs/heads/main/src/emdawnwebgpu/pkg/README.md). Pin the package and Emscripten toolchain; do not use the remote port's build-time download behavior. Dawn's C++ bindings are not a stable ABI/API, so header matching must be checked deliberately.

Browser work also needs a separately supplied WASM-compatible SDL3 library, canvas surface integration, asynchronous startup/error handling, asset preloading, exception configuration, and browser-driven scheduling. Do not link native Dawn/Metal archives or the macOS SDL framework into WASM. Native precise sleeps/present-mode selection do not transfer directly: use browser scheduling, preserve maxFps as a ceiling, and avoid blocking the browser main thread. Keep outputs under `build/web/`, check WebGPU availability at startup, and explain unsupported environments. No automatic WebGL fallback is proposed.

Windows/Linux remain unverified until their SDL SDKs, matched Dawn packages, native surface glue, and executable packaging are built and exercised on those systems. An upstream supported backend is not Frostwind platform support.

## Alternatives

- **Continue OpenGL:** smallest immediate cost and adequate for cube/lighting/Asteroids. It preserves the current learning priority, but does not establish a native/browser WebGPU path or compute support within our 3.3 baseline.
- **[wgpu-native](https://github.com/gfx-rs/wgpu-native):** viable C API for C++ and Metal/Vulkan/D3D12 implementations. It may allow keeping C++17 consumers, but introduces a Rust dependency build/toolchain and a different integration/binding choice. Select Dawn for its C++/CMake and Emdawnwebgpu route; reconsider if the local SDK spike cannot meet repository constraints.
- **Direct Metal now, Vulkan later:** gives API-specific learning/control, but requires separate shader/platform implementations for broader reach. WebGPU delegates those backend implementations to a dependency, with some feature/control limits.
- **Keep GL and WebGPU permanently:** useful for ongoing comparisons but doubles shader/interface/testing maintenance and encourages the abandoned abstract-backend design. Retain GL only as a temporary migration baseline and Git history, with retirement after parity rather than permanent runtime selection.
- **Expose raw WebGPU throughout examples:** reduces wrappers but spreads device/surface/lifetime handling into application behavior. Keep focused concrete owners while preserving explicit pipeline and binding concepts.

## Risks and limitations

- Dawn import size, transitive dependencies, static linking, C++20, and SDK reproducibility may outweigh the benefit for our current scenes. Prove them before committing the full migration.
- Asynchronous startup and errors materially change current callback/failure tests; a non-null GPU handle is not proof initialization succeeded.
- Texture orientation, color space, mip filtering, attachment formats, and rasterization differences may change appearance. Behavioral/visual parity is required; identical pixels across APIs are not promised.
- WebGPU does not guarantee native API feature parity, mesh shaders, hardware ray tracing, or a performance improvement. Compute availability does not implement a compute example.
- Native and browser bindings, scheduling, and capabilities require separate validation. Browser GPU/device availability varies by environment.
- Pursuing this migration would change the OpenGL-first roadmap. It needs a renewed priority/design decision before any implementation or guidance changes.

## Validation

1. **Dependency gate:** prove a clean offline configure/build using the exact local SDK and C++20; exercise a temporary native clear/triangle under `build/`. Verify Metal adapter logging, matching headers, no browser/server requirement, static dependency links, and SDL framework packaging from a different working directory.
2. **CPU checks:** run camera/input/orbit tests; amend near-depth expectations to 0 and retain far-depth 1, foreshortening, transform order, rejected-state preservation, and orbit behavior. Verify parameter record sizes/offsets, count bounds, and upload overflow/alignment where meaningful. Validate description fields, layout entry uniqueness/types, and target/sample/vertex compatibility at meaningful boundaries; do not add tests that merely repeat descriptor defaults.
3. **Lifecycle checks:** adapt `tests/ApplicationUpdateTests.cpp` and its CMake target, which currently replace Window operations at link time and depend on GLAD/glViewport. Use a private test boundary for pending readiness and frame outcomes without constructing a GPU. Test startup failure before/after appstate publication, delayed completion, close during startup, late callbacks after shutdown, update/render results/exceptions, discard-without-submit, acquisition skips, resize/minimize, focus timing, and render/presentation budget accounting. Do not retain GLAD solely for tests.
4. **Native graphics:** run all four examples with validation enabled. Check indexed/sequential draws, bounds/layout/device mismatch diagnostics, WGSL/binding errors, texture load failures, orientation, color appearance, cube occlusion/culling/camera controls, pause/reset, filtering/mips/anisotropy, 4× resolve, nearest quad, resize/high-density/minimize/restore, VSync requests/fallback, and clean resource-before-window teardown. Verify asynchronous pipeline failure keeps startup unready, explicit layouts allow a BindGroup to be shared by compatible variants, incompatible attachments/groups fail clearly, and normal frames/resizes do not compile pipelines. Check that discarded attachments have no later readers. Trigger device/error paths where feasible and record any untested cases.
5. **Performance:** compare matching Release scenes/resolution/sampling against the saved GL baseline. Report CPU submission separately from GPU time where timestamp queries are supported; include frame-time distribution and startup cost. Disable caps/VSync where possible, disclose remaining presentation constraints, and avoid claiming FPS improvement from a capped scene.
6. **Documentation:** check relative links/assets, current-versus-proposed status, verified commands, and unchanged historical captures. The owner supplies any new screenshots; automated captures are excluded.

Browser and Windows/Linux checks are recorded separately when those stages begin. Do not mark this RFC Implemented until native parity, lifecycle behavior, GL retirement, and documentation updates are complete. Browser delivery remains explicitly pending if not separately validated.

## Conditional implementation plan

1. Re-evaluate whether migration solves a concrete need and explicitly select it before proceeding. Reconfirm implementation choice, language/toolchain, native/browser scope, and GL maintenance/retirement policy; the earlier acceptance is deferred.
2. Select/pin the Dawn SDK and prove the dependency/surface/clear-triangle gate under `build/`. Record the exact dependency and toolchain manifest. If this gate fails, revise the choice before migrating public APIs.
3. Add concrete WebGPU owners and asynchronous lifecycle/diagnostics alongside the temporary GL baseline in an isolated migration branch. Use a bounded internal smoke target before switching the normal examples; no permanent backend selector. Keep intermediate build states explicit.
4. Port Textured Quad and the two clear-only scaffolds, including WGSL assets, parameter/binding setup, startup hooks, native packaging, and callback tests.
5. Port Textured Cube: projection/test convention, orbit behavior, pipeline/depth/culling, generated mips/anisotropy, and 4× MSAA/resolve. Run native parity and failure checks for all four examples.
6. Remove GL calls, GL resource owners, `cmake/GLAD.cmake` integration, and the now-unused GLAD dependency import once tests/examples no longer use it. Preserve historical screenshots/RFCs and the existing archived experiment; use a recorded Git baseline for comparisons. Retire GLSL assets only after WGSL equivalents and license records are in place.
7. Update README, ROADMAP, AGENTS, structure/rendering/development guides, dependency inventory, examples, CMake presets, and Unreleased changelog to describe verified behavior. Only after a renewed acceptance, record any RFC 0006 supersession and scoped amendments to RFCs 0002/0007, adding implementation evidence as behavior changes; retain RFC 0003's engine/example ownership and RFC 9999's historical status. Reconcile draft RFC 0005's rendering assumptions before implementing sprites. Mark this RFC Implemented only after the native completion gate.
8. Undertake Emscripten/canvas and then other native platforms as separately selected work with their own dependency and runtime evidence. Resume lighting/Asteroids after the native migration rather than adding them concurrently.

## Open questions

- Which exact Dawn revision, SDK/deployment target, static packaging layout, and supported Debug/Release configurations pass the dependency gate? Record them before import.
- Which supported surface format/view and color configuration best preserve current appearance on the development Mac? Verify with the actual adapter and display.
- When should browser delivery become an active objective after native parity? This proposal prepares its boundary but does not claim to deliver it.

## Decision

Accepted on 2026-10-20 following the owner's confirmation that WebGPU is now the focus and its best practices should guide the engine. Follow WebGPU pipeline descriptions, explicit reusable layouts, asynchronous creation, pass/resource separation, and GPU lifetime rules rather than preserving OpenGL-shaped interfaces.

Deferred on 2026-10-21 following the owner's decision to retain WebGPU as a research topic, similar to multiple-backend support. The existing renderer is sufficient for current examples; learning lighting and building gameplay take priority over rebuilding rendering infrastructure. The earlier acceptance is no longer active.

RFC 0006 remains Implemented and is not superseded. C++17, direct OpenGL owners, and existing projection/lifetime contracts remain current. Preserve this design and its best-practice analysis for a future explicitly selected need. No WebGPU implementation, dependency, toolchain upgrade, release, or tag was introduced.

## Learning references

- [WebGPU Unleashed: A Practical Tutorial — Shi Yan](https://shi-yan.github.io/webgpuunleashed/): a free JavaScript/WebGPU learning resource covering the GPU pipeline, buffers/uniforms, textures, transforms/cameras, lighting, MSAA/mipmapping, and compute. Use it for a separate learning experiment; adapt browser examples deliberately if native C++ research is selected. It does not establish Frostwind implementation status or replace the specification and pinned Dawn documentation for API contracts.
