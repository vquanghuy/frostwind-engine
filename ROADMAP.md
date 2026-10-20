# Roadmap

Frostwind grows through practical examples, with OpenGL as the current learning focus. Metal on macOS and Vulkan on Windows/Linux remain deferred future directions. The phases below group the intended work by theme and suggested learning order. They have no delivery dates or release commitments, and later priorities can change as examples reveal requirements.

The current baseline is native macOS window/application management, frame pacing, direct OpenGL texture/program/buffer/vertex-array owners, CPU image loading, GLM-backed perspective camera, keyboard/elapsed-time updates, Textured Quad/Textured Cube, and the Asteroids/Mini Artillery window scaffolds. Windows/Linux SDK verification and browser builds remain pending. See the [README](README.md) for current capabilities, [RFCs](docs/rfcs/README.md) for detailed proposals, and the [changelog](CHANGELOG.md) for completed work and owner-supplied captures.

Phase numbers describe intended sequencing; they are separate from historical development milestone identifiers. Each phase describes its target scope; see the changelog for implemented increments. A roadmap phase does not accept its detailed RFC, establish platform support, or authorize a release or tag.

## Current focus - OpenGL scenes

Keep one active learning objective: use Textured Cube to explore model/view/projection matrices and camera movement, then add basic lighting before returning to Asteroids. A bounded grass experiment remains a later OpenGL objective. This roadmap owns the learning sequence; [RFC 9999](docs/rfcs/9999-multi-backend-rendering-research.md) preserves the multi-backend analysis as deferred research, not a requirement for current OpenGL changes. Keep the current OpenGL classes focused on their responsibilities and extend them only when the selected example needs an operation. Grass appearance and controls belong to its example; reusable rendering operations belong in the engine.

1. **Textured Cube and basic lighting:** the [second example](examples/02-textured-cube/README.md) implements indexed cube geometry, an orbit perspective camera, explicit model/view/projection matrices, depth/culling, 4× MSAA, and a 120 FPS cap with VSync. Explore its transforms and camera, then add vertex normals and a small ambient/diffuse/specular lighting scene. Skyboxes, model loading, and shadow mapping are separate later increments. [RFC 0007](docs/rfcs/0007-textured-cube-and-perspective-camera.md) records the foundation.
2. **Return to Asteroids:** reuse math/input/update foundations for Camera2D, mouse aiming, sprites, parallax, and the scoped normal-map experiment in RFC 0005. The complete game remains a separate increment.
3. **Small grass patch (later):** build a bounded example on a simple ground plane with a camera and reusable blade/clump geometry. Introduce instance data and instanced drawing when the patch needs many copies; keep placement, variation, controls, and grass appearance in the example.
4. **Wind and appearance:** animate blade vertices using time, world position, height along the blade, wind direction/strength, and per-instance variation. Study colors, blade shapes, density, and simple lighting. Keep this first increment a visual animation experiment; it does not require per-blade rigid bodies, a fluid solver, or general terrain generation.
5. **Local interaction:** bend nearby grass around a moving sphere or character marker, using a bounded influence position/radius. Introduce persistent interaction data only if a later observable effect requires it; a complete character controller or physics system is unnecessary for the first interaction test.
6. **Larger field:** divide the field into patches, skip invisible patches, reduce distant density/detail, and profile geometry, overlapping fragments, CPU submission, and memory use. Expand only after the small patch behaves correctly.

The visual target is grass inspired by Breath of the Wild, not a claim about Nintendo's implementation or a commitment to reproduce the entire game's vegetation/world systems. The intended first grass experiment uses conventional OpenGL 3.3 rendering; compute, mesh shaders, and ray tracing are not prerequisites. Exact field size, density, and frame rate must be measured on the development machine.

Completion of the small 3D step means an inspectable, controllable scene with correct depth/transforms and preserved existing examples. Completion of the first grass step means a bounded patch with observable wind controls and consistent root anchoring/orientation. Local interaction and larger-field optimization are separate increments. Keep temporary validation under ignored `build/`; example directories/assets are added when implementation begins.

Asteroids follows the bounded cube/lighting step. Mini Artillery, water/fluid work, and physics experiments remain later possibilities; they are not simultaneous tasks. Reuse a focused input/math facility where it serves the selected example; reconsider ordering explicitly when changing the active objective.

Phase numbers remain thematic guidance. The 2D Asteroids proposal follows the cube/lighting step and shares its implemented input/timing foundation. Metal/Vulkan implementation, shader cross-compilation, runtime backend selection, and further portability infrastructure are deferred. Choose a new active objective explicitly before pursuing them. Completed progress remains in the changelog.

## Phase 1 - OpenGL rendering owners

Use the existing OpenGL renderer to learn resource ownership and rendering. [RFC 0006](docs/rfcs/0006-opengl-focused-rendering.md) records the direct implementation; the [structure guide](docs/project-structure.md) documents the code. Completed work and validation stay in the changelog. The intended capabilities are:

- Window owns SDL/OpenGL context setup, GLAD, VSync, and presentation.
- Texture2D uploads/binds textures, ShaderProgram links/activates/writes uniforms, VertexBuffer owns vertex bytes, IndexBuffer owns indices, VertexArray configures layout/buffer associations, and Renderer submits sequential/indexed draws from VertexArray.
- CPU images and geometry remain independent of GL where practical.
- All examples use current engine operations, preserving assets, controls, backgrounds, and packaging.
- Native and engine-only builds, failure recovery, callbacks, pacing, and resource teardown are checked.

The previous device/frame/pipeline/backend experiment is preserved in `archive/multi-backend-experiment` and [RFC 9999](docs/rfcs/9999-multi-backend-rendering-research.md). Additional backend structure is deferred, not a prerequisite for learning OpenGL scenes.

## Phase 2 - Input, cameras, and sprites

With OpenGL resource ownership in place, Asteroids introduces reusable 2D fundamentals. Reconcile [draft RFC 0005](docs/rfcs/0005-camera-keyboard-and-sprites.md)'s drawing assumptions with the [current rendering interface](docs/rendering-guide.md) before implementing the renderer. Input state and camera math can be developed independently where useful. The intended capabilities are:

- A GLM-backed camera using explicit world and drawable-pixel coordinates.
- Keyboard state, cursor-to-world conversion, and elapsed-time updates.
- Mouse heading, heading-relative WASD movement, and automatic camera following.
- Overlapping sprites and a scrolling starfield with parallax.
- A normal-mapped sprite-lighting prototype after the baseline scene works.
- Checked aiming, movement, resize behavior, focus handling, and continued operation of the other examples.

This phase demonstrates engine facilities through an Asteroids scene; completing the game is a separate increment.

## Phase 3 - Conventional 3D rendering

The next phase builds on rendering resources and camera/input foundations to explore perspective, geometry, and visibility. The intended capabilities are:

- A focused textured-cube example with a movable perspective camera.
- Indexed mesh drawing, depth testing, and face culling.
- Documented world, clip/depth, image, and framebuffer conventions.
- Camera calculations and CPU geometry that need no graphics context.
- Verified transforms, occlusion, texture orientation, aspect ratio, resizing, and existing-example behavior.

Extend the rendering interface through the needs of this small scene before pursuing advanced GPU features.

## Phase 4 - Metal backend

If native API learning is selected later, use the working OpenGL 2D/3D scenes as a baseline for a Metal experiment. Reconsider architecture at that time; the current direct wrappers do not promise backend portability. This phase is deferred while OpenGL scenes are the active objective; reconsider it when native API learning is explicitly selected. A dedicated RFC will establish API/OS baselines, shader tooling, and lifetime rules. The intended capabilities are:

- Metal rendering on the development Mac, beginning with Textured Quad and the small 3D scene.
- Backend shader artifacts, resource bindings, command submission, and presentation.
- Safe handling of pending GPU work and resource retirement.
- All existing examples migrated alongside any shared-interface changes.
- Native validation of asset loading, resize/minimize behavior, startup failures, and teardown, plus measured comparisons on matching scenes.

Decide separately whether OpenGL remains a maintained reference backend. Adding Metal does not silently retire it.

## Phase 5 - Vulkan backend

With the interface evaluated through Metal, add Vulkan on available Windows/Linux test systems. Start with the platform that can actually be exercised, then verify the other independently. The intended capabilities are:

- Recorded, pinned SDK/dependency integration under the existing local dependency policy.
- Vulkan resources, shaders, command submission, synchronization, and presentation.
- Existing examples running through the shared rendering interface.
- Surface/swapchain recreation, safe resource retirement, and explicit backend selection.
- Verified builds, runtime packaging, and native validation on each claimed platform.

A successful macOS build or Vulkan translation layer does not establish Windows/Linux support.

## Phase 6 - Advanced rendering experiments

Once a backend and GPU expose the required capabilities, use small scenes to study modern rendering and measure where it helps. These experiments can begin on one backend while another remains pending. The intended capabilities are:

- A mesh-shading example exploring geometry generation and culling, with a conventional comparison path.
- A ray-tracing example starting with acceleration structures and a bounded effect such as shadows.
- Separate capability and limit checks for mesh/task stages, acceleration structures, ray queries, and ray-tracing pipelines where applicable.
- Clear handling of unsupported features while earlier examples continue to run.
- Correctness checks and profiling records that distinguish API availability from dedicated hardware acceleration.

Do not require identical advanced features or performance across backends.

## Phase 7 - Larger game and world examples

Apply the foundations to bounded gameplay, terrain, and world-building increments. These examples can develop alongside backend work when their prerequisites exist; they do not all wait for Phase 6. The intended capabilities are:

- **Asteroids:** weapons, asteroid behavior, collision, scoring, and game flow after the 2D scene works.
- **Mini Artillery:** aiming/projectiles, terrain carving, affected-chunk collision rebuilding, turns, and wind as 2D geometry/physics foundations arrive.
- **Voxel world:** chunk data, exposed-face meshing, block selection/editing, player collision, terrain generation, streaming, and saving after conventional 3D rendering works.
- Verified example-specific rules, matching terrain render/collision edits, fixed-step simulation where needed, and voxel chunk-boundary/persistence checks.

Define a playable or inspectable outcome for each increment before implementation. Keep game rules and assets in their examples, geometry/simulation testable independently of graphics where practical, and extract reusable facilities as needed. A voxel world does not require ray tracing or mesh shaders. Select new example directories when their implementation begins.

## Future experiments

Keep ideas here before committing them to a phase. Each entry records a small starting scope and possible follow-up work; it does not select an algorithm, library, graphics backend, or delivery date. Choose an experiment when its prerequisites and learning value fit the current work.

| Experiment | First scope | Possible follow-up | Foundations |
| --- | --- | --- | --- |
| Grass and vegetation | A small patch of grass with adjustable wind animation | Interactive bending, larger instanced fields, visibility culling, and levels of detail | Conventional 3D rendering, camera controls, shader parameters, and instanced drawing as field density grows; see the current OpenGL focus |
| Water and fluid simulation | A bounded water surface with adjustable waves | Shallow-water flow, disturbances and obstacles, or a separate particle/volume-fluid experiment | Mesh rendering, elapsed-time updates, and a fixed simulation timestep when introducing a solver; GPU compute only when an implemented backend supports it |
| Physics sandbox | A 2D scene for dropping bodies, applying forces, and inspecting collisions | Constraints/joints, stacking, material properties, debug drawing, and a later 3D sandbox | Input, rendering, fixed-step simulation, and an appropriate physics integration |

For grass, distinguish visual wind animation from physical interaction. For water, choose explicitly between surface animation and simulated fluid motion before expanding the scope. For physics, retain the planned Box2D 3.x direction for 2D; choose a 3D library when the 3D experiment begins.

When selecting an idea, define its learning objective, prerequisites, and observable completion criteria, then place it in the intended sequence or develop it alongside a phase whose foundations are ready. Create its example directory only when work begins. Keep scene setup, controls, assets, and research notes in that example's README and `docs/`; extract reusable facilities into the engine as needed. Use an RFC for significant engine architecture, dependency, or algorithm decisions. Completed work stays in the changelog.

## Further directions

Audio, Dear ImGui tooling, richer asset workflows, browser builds, and 3D physics remain driven by example needs. Retain the planned Box2D 3.x and Clipper2 direction for appropriate 2D work; choose a 3D physics library when needed. Browser graphics and WASM dependencies require a separate design and verification effort. Multiplayer belongs to an example and needs explicit authority, ordering, and recovery rules.

Keep each increment small enough to explain and validate. Preserve pinned local dependencies, asset provenance, existing examples, and native/web output separation. Update this roadmap when priorities change; record completed progress in the changelog. Screenshots remain owner-supplied, and generated validation outputs stay under ignored `build/`.

Structure adapted from [Dovyski's roadmap template](https://github.com/Dovyski/template/blob/master/ROADMAP.md).
