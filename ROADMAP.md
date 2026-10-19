# Roadmap

Frostwind grows through practical examples, starting with the working OpenGL implementation and progressing toward Metal on macOS and Vulkan on Windows/Linux. The phases below group the intended work by theme and suggested learning order. They have no delivery dates or release commitments, and later priorities can change as examples reveal requirements.

The current baseline is native macOS window/application management, frame pacing, shader and texture loading, Textured Quad, and the Asteroids/Mini Artillery window scaffolds. Windows/Linux SDK verification and browser builds remain pending. See the [README](README.md) for current capabilities, [RFCs](docs/rfcs/README.md) for detailed proposals, and the [changelog](CHANGELOG.md) for completed work and owner-supplied captures.

Phase numbers describe intended sequencing; they are separate from historical development milestone identifiers. Each phase describes its target scope; see the changelog for implemented increments. A roadmap phase does not accept its detailed RFC, establish platform support, or authorize a release or tag.

## Phase 1 - Rendering boundary

The first phase uses the existing OpenGL renderer to learn graphics ownership and establish a small interface that future backends can improve. [Accepted RFC 0006](docs/rfcs/0006-rendering-backend-boundary.md) describes the migration. The intended capabilities are:

- An OpenGL implementation behind shared engine resource and drawing operations.
- Separate ownership for the SDL window and graphics context/device, including presentation.
- CPU image and geometry data that can be used independently of GPU resources.
- Textured Quad, Asteroids, and Mini Artillery using the same current interface, preserving their assets, controls, backgrounds, and packaging.
- Verified native and engine-only builds, initialization failures, callbacks, frame pacing, and resource teardown.

Add only the abstractions exercised by these examples. Metal/Vulkan directories and configuration choices arrive with working implementations.

## Phase 2 - Input, cameras, and sprites

With the rendering boundary in place, Asteroids introduces reusable 2D fundamentals. Reconcile [draft RFC 0005](docs/rfcs/0005-camera-keyboard-and-sprites.md)'s drawing assumptions with RFC 0006 before implementing the renderer. Input state and camera math can be developed independently where useful. The intended capabilities are:

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

Use the working OpenGL 2D/3D scenes to learn Metal and test the rendering boundary against another API. Backend learning may move earlier if it becomes the immediate priority. A dedicated RFC will establish API/OS baselines, shader tooling, and lifetime rules. The intended capabilities are:

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
| Grass and vegetation | A small patch of grass with adjustable wind animation | Interactive bending, larger instanced fields, visibility culling, and levels of detail | Conventional 3D rendering, camera controls, and shader parameters |
| Water and fluid simulation | A bounded water surface with adjustable waves | Shallow-water flow, disturbances and obstacles, or a separate particle/volume-fluid experiment | Mesh rendering, elapsed-time updates, and a fixed simulation timestep when introducing a solver; GPU compute only when an implemented backend supports it |
| Physics sandbox | A 2D scene for dropping bodies, applying forces, and inspecting collisions | Constraints/joints, stacking, material properties, debug drawing, and a later 3D sandbox | Input, rendering, fixed-step simulation, and an appropriate physics integration |

For grass, distinguish visual wind animation from physical interaction. For water, choose explicitly between surface animation and simulated fluid motion before expanding the scope. For physics, retain the planned Box2D 3.x direction for 2D; choose a 3D library when the 3D experiment begins.

When selecting an idea, define its learning objective, prerequisites, and observable completion criteria, then place it in the intended sequence or develop it alongside a phase whose foundations are ready. Create its example directory only when work begins. Keep scene setup, controls, assets, and research notes in that example's README and `docs/`; extract reusable facilities into the engine as needed. Use an RFC for significant engine architecture, dependency, or algorithm decisions. Completed work stays in the changelog.

## Further directions

Audio, Dear ImGui tooling, richer asset workflows, browser builds, and 3D physics remain driven by example needs. Retain the planned Box2D 3.x and Clipper2 direction for appropriate 2D work; choose a 3D physics library when needed. Browser graphics and WASM dependencies require a separate design and verification effort. Multiplayer belongs to an example and needs explicit authority, ordering, and recovery rules.

Keep each increment small enough to explain and validate. Preserve pinned local dependencies, asset provenance, existing examples, and native/web output separation. Update this roadmap when priorities change; record completed progress in the changelog. Screenshots remain owner-supplied, and generated validation outputs stay under ignored `build/`.

Structure adapted from [Dovyski's roadmap template](https://github.com/Dovyski/template/blob/master/ROADMAP.md).
