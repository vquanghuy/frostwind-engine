# Working on Frostwind

## Project context

Frostwind is an experimental C++17 engine for 2D and 3D games, developed through practical examples in one repository. Reusable engine code lives under `engine/`; examples own application behavior, game rules, and their assets. Mini Artillery is a planned destructible-terrain example, not the engine's identity or an implemented game. Keep scope lightweight and add systems only as examples need them.

Implemented: SDL3 Window with OpenGL context/GLAD/VSync/presentation, application lifecycle and pacing, direct OpenGL Texture2D/ShaderProgram/VertexBuffer/IndexBuffer/VertexArray owners, Application-owned Renderer for clear/draw operations and application viewport handling, CPU PNG/JPEG loading, GLM-backed PerspectiveCamera and OrbitCameraController, keyboard state, elapsed-time updates, depth/culling and optional MSAA, the textured-quad/textured-cube examples with example-owned directional Phong lighting on the cube, and minimal Asteroids/Mini Artillery scaffolds. Camera2D, mouse input, sprites, audio, physics, terrain destruction, and reusable engine lighting/material systems remain planned. Read `README.md`, `docs/coding-conventions.md`, `docs/project-structure.md`, and relevant RFCs before changes. RFC 0003 records engine/example ownership; RFC 0006 records the active OpenGL-focused implementation. RFC 9998 preserves WebGPU migration as deferred research; it does not authorize renderer replacement, a C++20 upgrade, or new dependencies. RFC 9999 is historical deferred multi-backend research, preserved in commit e9cf616 on archive/multi-backend-experiment.

## Technology decisions

- Use one root CMake project with a static `frostwind` library and separate example executables. Retain native/web output separation.
- Use pinned local dependencies under `external/`; follow its README and RFC 0001. Do not add FetchContent or a package manager without changing that policy.
- SDL3 3.4.18 uses the official macOS universal dynamic framework and packaged headers. Preserve its layout, symlinks, license, and provenance. Copy it beside each example; do not rebuild it locally or restore its source tree. Prefer static linking for other compiled dependencies.
- Current learning priority is OpenGL feature development through small technical examples, building on Textured Cube and its implemented basic lighting. Asteroids and Mini Artillery scene/game development is deferred until the relevant engine features have been demonstrated; neither game is the automatic next step. Follow the selected root-roadmap sequence: normal mapping + static skybox, shadow mapping, Dear ImGui, physics simulation, bounded shallow-water simulation, instanced grass with vertex-shader wind, PBR, static glTF model rendering, then skeletal animation. Present the selected stages as individual technical examples, combining normal mapping + skybox in one example and preserving the minimal cube lighting reference. Share reusable engine facilities; choose new example directories only when implementation begins. Detailed APIs, dependencies, and physics scope require design when their work begins; RFC 0009 remains draft. Grass targets continuous coverage to the horizon using matching grass-textured ground under instanced blades, with finite patches, culling, and distance-based density/detail. A small patch is an implementation validation step, not the final visual target; interaction and persistent bending remain later increments. Keep classes focused on their own OpenGL responsibilities. Do not reintroduce GraphicsDevice, RenderFrame, GraphicsPipeline, abstract backends/surfaces, resource registries, or universal binding layouts without a demonstrated need and an explicit design decision. Multi-backend compatibility is not a requirement for current changes; Metal/Vulkan, cross-compilation, and runtime backend selection remain deferred.
- Native graphics target OpenGL 3.3 Core through GLAD 2.0.8. Newer actual contexts are allowed. GLAD is native-only; browser GLES3/WebGL 2 and a separate WASM-compatible SDL library remain pending.
- Use the pinned stb_image 2.28 header for PNG/JPEG decoding; compile its implementation once and keep decoder configuration private to the engine.
- GLM 1.0.3 is pinned locally and exposed transitively for camera/math types; retain explicit right-handed/OpenGL depth conventions. Dear ImGui, Box2D 3.x, and Clipper2 remain planned. Box2D 3 uses a C API callable from C++; never mix in 2.x APIs. Select a 3D physics library and other graphics backends when their work begins; do not introduce unused backend hierarchies.
- Preserve exact dependency versions, origins, local patches, and supplied binary toolchain/configuration records. Generated artifacts belong under `build/`, not `external/`.
- Record significant architectural decisions in numbered RFCs, without author metadata.

## Code organization and ownership

Follow `docs/coding-conventions.md` and `docs/project-structure.md`. Headers stay beside `.cpp` files. Use namespace `frostwind` for engine code, not example behavior. Add directories when implementation needs them; avoid empty placeholders.

Use primitive aliases from `<core/Types.h>` where appropriate: `u8`, `u32`, `i32`, `f32`, `byte`, and `usize`; qualify them in examples. Keep external API types and ordinary `int` dimensions where they fit. Prefer implicit numeric conversions for validated values, avoid redundant casts, and keep size arithmetic in `usize` to prevent integer overflow. Required pointer conversions remain explicit.

- `engine/src/app/`: reusable application lifecycle, callback coordination, direct viewport setup, ownership of the persistent Renderer passed to render callbacks, and future simulation scheduling.
- `engine/src/platform/`: Window owns SDL video/window/context, context attributes, GLAD loading, VSync, drawable queries, and presentation. Native handles stay private; there is no surface hierarchy or per-backend friendship.
- `engine/src/rendering/`: Texture2D calls GL directly for texture operations; ShaderProgram compiles/links/activates and updates uniforms; VertexBuffer owns immutable VBO bytes and binding; IndexBuffer owns immutable u32 index storage and cached count/maximum metadata; VertexArray owns VAO layout/associations, borrows initialized buffers, and derives vertex count from VertexLayout stride and stores optional index count; Renderer clears the target and exposes one drawTriangles(VertexArray) method that draws the complete stored geometry, choosing indexed/sequential drawing automatically; Image decodes CPU pixels. Keep GLAD and native graphics calls in engine implementation files. Examples use these owners through `initialize()` and `render(Renderer& renderer, int width, int height)`. Declare buffers before their borrowing VertexArray so the array is destroyed first. GPU owners require their creating context to remain current and outlive them; locations belong to the linked program that returned them. VSync is `ApplicationConfig::window.vsync`.
- `engine/src/camera/`: CPU-only PerspectiveCamera pose/view/projection math and a composed OrbitCameraController accepting device-independent axes and elapsed seconds. Input bindings and initial camera settings stay in examples; FPS movement remains planned. `engine/src/input/`: reusable Keyboard state, coordinated by Application. Application exposes `update(f32 deltaSeconds)` and read-only keyboard queries; first/resumed delta is 0 and stalls clamp to 0.1 seconds.
- Texture2D defaults to nearest/clamp-to-edge without mipmaps. Optional `TextureSampling` selects trilinear/generated mipmaps and capability-checked anisotropy. Textured Cube requests 16× (clamped/logged, isotropic fallback if unsupported); Textured Quad retains nearest sampling.
- `WindowConfig::depthBuffer` opts into 24-bit depth; `msaaSamples` defaults to 0, with explicit requests of at least 2 verified at startup. Textured Cube requests 4× MSAA and a 120 FPS cap with VSync; other examples retain their defaults. Renderer exposes explicit depth clear/test and back-face culling controls.
- Future engine camera, input, audio, terrain, and physics facilities belong in the engine when needed.
- `examples/01-textured-quad/`: current demo, executable entry point, and its assets/license record.
- `examples/02-textured-cube/`: rotating textured cube, example-owned key bindings, controller configuration, and model animation, shaders and runtime assets. Ambient/diffuse/Phong specular lighting and direct uniform uploads stay example-owned; normal maps/shadows/skyboxes remain future experiments.
- `examples/03-asteroids/`: minimal window scaffold and CMake target, plus a planning README and `docs/assets/` concepts/prototypes; future ship, asteroid, arena, weapon, collision, scoring, and game rules stay here.
- `examples/04-mini-artillery/`: minimal window scaffold and CMake target, plus a planning README and `docs/assets/` references; future tanks, weapons, aiming, damage, wind, turns, match state, UI, and game assets stay here.
- Each example README embeds its preferred concept and states implementation status. Example planning artwork/provenance belongs in its `docs/assets/`, separate from future runtime `assets/` and `assets/LICENSES.md`. Preserve older concepts and reference attribution.
- Example source directories use two-digit prefixes for suggested learning order. Keep CMake target names, executable names, and runtime output directories unnumbered.
- `external/`: shared pinned dependencies; `cmake/`: shared build/SDK/example packaging helpers.
- `docs/assets/`: shared branding and historical milestone PNGs/captions; game-specific concept and reference images belong with their owning examples.

Dependencies flow from examples to the engine. Engine code must not include example code or choose example assets. Each example derives from `frostwind::Application` to supply startup, events, and rendering; its global SDL callback adapters delegate to the engine. Example GPU members must be destroyed before the base Window/context. Window destroys the context before the SDL window/video subsystem. Keep native handles private and use scoped ownership for heap/C API resources. Keep geometry and simulation independent of a graphics context where practical.

## Future terrain and simulation rules

- Coordinate terrain geometry and GPU masks with the same explosion position/radius.
- Rebuild collision shapes only for affected chunks, accounting for holes, winding, small edges, and boundaries.
- Make render-pixel, physics-unit, and integer clipping conversions explicit.
- Use fixed simulation timesteps independently of rendering when physics is introduced; retain browser-compatible callback flow.
- Fixed timesteps and integer clipping do not establish whole-game determinism. Validate native/WASM behavior before relying on it.
- Optional multiplayer belongs to an example and needs explicit authority, event ordering, and state recovery.

## Build and validation

On the development Mac, use:

```sh
cmake --preset native-debug
cmake --build --preset native-debug --parallel
./build/native/bin/frostwind-textured-quad/frostwind-textured-quad
```

See `docs/development.md` for engine-only builds, IDE setup, and verification. Windows x64 and Linux x86_64 presets remain unverified with SDKs pending. Browser builds remain pending. Opt-in context-free CTest checks are available with `FROSTWIND_BUILD_TESTS=ON`; see development instructions. Native graphics validation remains separate. Never report checks as passing without executing them.

Run checks appropriate to the change:
- For documentation-only changes: verify link integrity, section anchors, image paths, and formatting. Do not trigger compiler builds or launch executables.
- For code, shader, or build configuration changes: execute native builds, relevant rendering/platform verification, and meaningful geometry/simulation tests as those systems arrive. For lifecycle changes, check initialization failure, callback results, and teardown while graphics resources still have a context.
Keep temporary validation outputs under ignored `build/` and report remaining manual checks.

Configure web builds with Emscripten through `emcmake` once that integration is implemented. Update verified commands and implemented architecture documentation as behavior changes.

## Repository hygiene

Use Conventional Commits for every new commit: `type: description` or `type(scope): description`. Scopes are optional and identify a module or area, such as `rendering`, `platform`, `engine`, `examples`, or `deps`. Use a lowercase type/scope and a short imperative description without a trailing period. Follow [the contributing guide](docs/contributing.md) for types, examples, and breaking-change notation. Preserve existing commit history.

Maintain `CHANGELOG.md` using Keep a Changelog 1.1.0. Collect notable changes under Unreleased using standard categories; identify engine versus named example changes when useful. Add dated version sections only when the owner explicitly requests a release or numbered milestone. Label development milestones clearly; a milestone does not authorize a release or tag. Preserve existing releases, tags, screenshots, and historical Mini Artillery captions; restructuring does not reset versions or create a release.

The owner captures screenshots; never capture automatically. When asked to retrieve a supplied milestone screenshot, preserve its PNG under `docs/assets/milestones/` and embed it directly in the changelog with date/platform caption. Record pending captures rather than fabricating images. Keep progress in the changelog, not a separate milestone document.

Do not commit generated builds, environment files, editor state, or `.serena/`. Preserve unrelated changes. Each example records runtime asset sources/licenses in its own `assets/LICENSES.md`. Keep third-party asset licensing separate from project code licensing, and label reference images as references rather than screenshots of implemented examples.
