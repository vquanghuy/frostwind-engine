# RFC 0007: Textured cube, perspective camera, and elapsed-time updates

- **Status:** Implemented
- **Created:** 2026-10-20
- **Supersedes:** None
- **Superseded by:** None

## Problem

The direct OpenGL owners can draw textured indexed geometry, but Textured Quad
does not exercise perspective, model/view transforms, depth, or camera motion.
The owner selected a cube as the next example before returning to Asteroids.
Introduce the shared math/input/timing facilities through this bounded scene.

## Goals and scope

Render a textured cube with model rotation and a controllable orbit camera.
Establish coordinate conventions, resize behavior, depth storage/testing, and
back-face culling. Preserve existing examples and engine/example ownership.
Lighting, shadows, skyboxes, imported models, mouse capture, physics, and other
graphics backends are later increments.

## Design

- **Dependencies:** pin GLM 1.0.3 at commit
  `8d1fd52e5ab5590e2c81768ace50c72bae28f2ed` under `external/glm/`, with its
  upstream license, archive checksum, and provenance. A local interface target
  propagates headers and consistent radians configuration through the engine.
  No configuration-time downloads or aligned/experimental types.
- **Camera:** `engine/src/camera/PerspectiveCamera` calculates view/projection
  matrices without SDL or OpenGL. Use explicit right-handed `lookAtRH` and
  `perspectiveRH_NO`, +Y up, view-space -Z forward, column vectors, radians,
  and NDC depth [-1, 1]. Invalid, non-finite, degenerate views/projections throw
  without changing valid state. Call setters before using its identity defaults.
  The camera stores/exposes the validated position, target, and normalized up
  vector alongside its view matrix; pose getters initially match identity view.
- **Orbit movement:** `OrbitCameraController` composes with PerspectiveCamera,
  owning target/yaw/pitch/distance, limits, and angular/distance speeds. It accepts
  device-independent signed axes and elapsed seconds, wraps yaw, clamps pitch/
  distance, and applies a +Y-up view without changing projection. Reset applies
  a validated configuration immediately. Invalid/non-finite/unrepresentable
  input retains both controller and camera state. Neither type depends on SDL
  or OpenGL. Bindings and initial settings belong to the example. FPS movement
  remains a later controller using the same camera; no inheritance is required.
- **Window:** append `WindowConfig::depthBuffer` (default false), preserving
  existing aggregate initializers and 2D framebuffer requests. The cube opts
  into a 24-bit depth attachment; Window checks the obtained depth size. Append
  `msaaSamples` (default 0); requests of at least 2 must be fulfilled or startup
  fails cleanly. The cube requests 4× MSAA, explicitly enables multisampling,
  logs the actual count, and uses a 120 FPS cap with VSync. Other examples keep
  their defaults. MSAA handles geometric edges, not texture filtering.
- **Renderer:** add `clearDepth`, `setDepthTest`, and `setBackFaceCulling` as
  direct OpenGL operations. Depth clear disables scissor, enables depth writes,
  and clears to 1. Depth setup uses LESS and writes; culling uses BACK and CCW.
  State persists until changed. Color clearing retains its existing behavior.
- **Texture sampling:** per-initialization `TextureSampling` defaults to nearest/no
  mipmaps, preserving Textured Quad. Trilinear generates the mipmap chain,
  uses linear magnification, and keeps clamp-to-edge wrapping. The cube requests
  16× anisotropy, checking EXT/ARB support or OpenGL 4.6 core, clamping to the
  supported maximum and logging the applied level. Unsupported contexts retain
  trilinear filtering at anisotropy 1. Binding/unpack restoration and failed
  upload cleanup/retry remain part of the texture contract.
- **Input:** Application owns `Keyboard`, exposes const queries, and processes
  scancode events before the example callback. Held state persists; pressed/
  released flags accumulate until an update consumes them. Repeats do not make
  new presses. Focus loss/minimization releases keys and clears edges; inactive
  key events do not alter state. Focus gain starts from cleared state. Raw
  event dispatch remains available and there is no extra polling loop.
- **Updates:** an optional `update(f32 deltaSeconds)` returns SDL_AppResult.
  Active update runs after viewport setup and before rendering; edges clear
  afterward. First/resumed delta is 0; later deltas use successive monotonic
  callback starts, including prior render/swap/pacing/event time, clamped to
  0.1 seconds. Focus loss pauses updates while visible rendering can continue.
  Minimized/zero-sized frames skip update/render/present, clear input, and reset
  timing. Update exit/failure skips rendering/presentation; exceptions are
  caught at the existing SDL callback boundary. FPS pacing remains separate.
- **Example:** `02-textured-cube` owns the 24-vertex/36-index unit cube, shader
  files, texture, input bindings, initial camera settings, and animation. Arrow keys orbit,
  W/S change distance, Space pauses rotation, R resets, and Escape quits.
  Delegate orbit state/movement to the controller. Upload explicit model/view/projection
  uniforms through existing ShaderProgram setters. No mouse input is needed
  for this first inspectable camera. Shader color is unlit.

Asteroids and Mini Artillery become `03-asteroids` and `04-mini-artillery`;
their executable names, output paths, artwork, and behavior stay intact.
The cube packages its own unmodified copy of the existing crate with provenance.
No engine code selects example assets, and examples include no native GL calls.

## Alternatives

Implementing the full Asteroids RFC first would prioritize 2D camera and sprite
composition. The cube instead follows the selected OpenGL learning objective.
A fixed camera would validate projection but not interactive view transforms.
An orbit controller provides that inspection without introducing mouse capture
or a camera inheritance hierarchy. A scene/mesh/material hierarchy is unnecessary for
one cube; reuse the existing buffer, array, texture, program, and Renderer owners.

## Risks and limitations

GLM types become a public dependency. The engine orbit controller implements
reusable movement; bindings, initial pose, speeds, and limits remain caller policy.
Animation uses variable delta with a
stall clamp, not deterministic/fixed-step simulation. Native macOS verification
does not establish Windows/Linux or browser support. Face UVs deliberately
repeat the same unlit crate; no normals or lighting correctness are implied.

## Validation

Opt-in CTest checks cover context-free projection/near-far mapping, view
translation/orbit, aspect changes, invalid setters retaining state, keyboard
transitions/repeats/invalid scancodes, and controlled Application timing,
inactivity, callback results/exceptions, and initialization cleanup.
The callback tests replace platform operations and run on native non-Windows
targets; they do not claim platform-window behavior.

Build all examples and an engine-only configuration. Native readback and
lifecycle checks should verify cube output, depth/culling, transform changes,
resize, asset paths, startup failures, and GPU teardown before the context.
Compare existing example output to prior hashes. Record actual executed checks
and remaining manual checks in [development](../development.md#textured-cube-validation-2026-10-20).
Screenshots remain owner-supplied.

## Decision

Implemented following the owner's selection of a cube as the next example.
Timing/input/camera and orbit movement are reusable; scene, bindings, initial
camera settings, and model animation stay in the example.
[RFC 0005](0005-camera-keyboard-and-sprites.md) remains a draft
for Asteroids-specific Camera2D, mouse aiming, sprites, parallax, and lighting;
it should reuse the implemented foundation instead of duplicating it.
