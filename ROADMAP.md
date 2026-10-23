# Roadmap

Frostwind grows through practical examples, with OpenGL as the current learning focus. Metal on macOS and Vulkan on Windows/Linux remain deferred future directions. The phases below group the intended work by theme and suggested learning order. They have no delivery dates or release commitments, and later priorities can change as examples reveal requirements.

The current baseline is native macOS window/application management, frame pacing, direct OpenGL texture/program/buffer/vertex-array owners, CPU image loading, GLM-backed perspective camera, keyboard/elapsed-time updates, Textured Quad/Textured Cube, and the Asteroids/Mini Artillery window scaffolds. Windows/Linux SDK verification and browser builds remain pending. See the [README](README.md) for current capabilities, [RFCs](docs/rfcs/README.md) for detailed proposals, and the [changelog](CHANGELOG.md) for completed work and owner-supplied captures.

Phase numbers describe intended sequencing; they are separate from historical development milestone identifiers. Each phase describes its target scope; see the changelog for implemented increments. A roadmap phase does not accept its detailed RFC, establish platform support, or authorize a release or tag.

## Current focus - OpenGL scenes

Keep one active learning objective: develop OpenGL features through small technical examples, building on Textured Cube’s implemented matrices, camera movement, and basic lighting. Asteroids and Mini Artillery scene/game development is deferred until the relevant engine foundations have been demonstrated. The selected sequence below sets learning priorities; detailed implementation contracts and dependencies still need review. RFC 0009 remains a draft. An instanced grassland reaching visually to the horizon follows water simulation, with grass-textured ground beneath blade geometry; local interaction remains separate. This roadmap owns the learning sequence; [RFC 9999](docs/rfcs/9999-multi-backend-rendering-research.md) preserves the multi-backend analysis as deferred research, not a requirement for current OpenGL changes. Keep the current OpenGL classes focused on their responsibilities and extend them only when the selected example needs an operation. Grass appearance and controls belong to its example; reusable rendering operations belong in the engine.

1. **Textured Cube and basic lighting:** the [second example](examples/02-textured-cube/README.md) implements indexed cube geometry, an orbit perspective camera, explicit model/view/projection matrices, depth/culling, 4× MSAA, and a 120 FPS cap with VSync. Explore its transforms, camera, face normals, and example-owned ambient/diffuse/Phong specular lighting. Skyboxes, model loading, and shadow mapping are separate later increments. [RFC 0007](docs/rfcs/0007-textured-cube-and-perspective-camera.md) records the foundation; [RFC 0008](docs/rfcs/0008-basic-lighting.md) records implemented lighting.
2. **Normal mapping + skybox:** <a id="normal-mapping-and-skybox"></a> develop one bounded scene with a crate, ground plane, tangent-space normal textures, and a static cubemap background. Verify tangent handedness, UV/image orientation, normal strength, and unchanged geometry silhouettes. Use a known normal-map test surface before adapting the crate’s baked shading. The skybox initially supplies only background imagery; environment lighting and reflections remain separate work.
3. **Shadow mapping:** <a id="shadow-mapping"></a> build a separate technical example on the normal-mapping/skybox scene, adding one directional light’s depth pass, a depth texture/framebuffer, light-space transforms, bias, and basic PCF. Validate surface lighting separately from visibility and preserve normal mapping and the skybox.
4. **Dear ImGui:** <a id="dear-imgui"></a> add reusable SDL/OpenGL integration and example-owned panels for light, normal-map, and shadow settings, shadow-map preview, and frame-time diagnostics. Verify input capture and rendering state around the UI pass; pin the dependency under `external/` according to existing policy.
5. **Physics simulation:** <a id="physics-simulation"></a> introduce a bounded simulation example with fixed timesteps, simple bodies/collision shapes, and synchronization between simulation and rendered transforms. Use ImGui for inspection. Select 2D/3D scope and a suitable library when this work begins; Box2D 3.x remains the planned 2D direction and the 3D library is undecided. Model importing and skeletal animation are not prerequisites for simple procedural bodies.
6. **Water simulation:** <a id="water-simulation"></a> build a bounded shallow-water height-field example after physics, using fixed simulation timesteps and ImGui controls. Start with a small pool, click-driven disturbances, propagating/interfering ripples, boundary reflection, and adjustable damping, disturbance strength, and wave speed. Use simple surface lighting to inspect the simulation before PBR. Define the numerical solver, stability limits, boundary conditions, and coordinate conversions in an RFC; a CPU implementation is a candidate, not a requirement for GPU compute or a new graphics backend. Validate rest-state stability, finite values, timestep limits, repeatable disturbances, and render-rate-independent updates. Reflections, refraction, foam, two-way body coupling, and freely splashing 3D fluid are separate later increments.
7. **Instanced grass:** <a id="instanced-grass"></a> demonstrate a continuous grassland reaching visually to the horizon, using a seamless grass-color ground texture beneath shared blade/clump geometry and GPU instanced drawing, with per-instance placement, rotation, scale, color, and wind phase. Blend nearby blades into the matching ground texture at distance so gaps and far views remain green. Use finite terrain patches, visibility culling, distance-based density/detail, and smooth transitions as part of this target; visually continuous coverage does not require infinite geometry. Animate upper vertices in the shader while roots remain anchored. Reuse the camera, directional lighting/shadow facilities, static skybox, elapsed-time animation, and ImGui density/wind controls. Introduce focused instance-buffer/attribute and draw operations when needed; retain existing non-instanced drawing. Validate instance transforms/counts, root anchoring, orientation, consistent deformation in visible/shadow passes, and bounded performance on the development machine. This step is visual wind animation, independent of the water solver and per-blade rigid-body physics. Local interaction and persistent bending follow separately; compute and newer graphics APIs are not prerequisites.
8. **Physically based rendering (PBR):** <a id="physically-based-rendering"></a> establish linear-light/color-texture handling, output encoding, and a bounded metallic-roughness material/shading path. Prove it on procedural geometry with direct lighting first. Environment lighting, reflection prefiltering, and image-based lighting require their own scoped decisions; the existing static skybox does not supply them automatically.
9. **Static glTF model rendering:** <a id="gltf-model-rendering"></a> load a documented subset of glTF geometry, indices, textures, metallic-roughness materials, and node transforms using the PBR foundation. Define importer/dependency policy, ownership, unsupported-feature handling, and asset licenses before integration. Introduce reusable mesh/model facilities through actual imported callers, reconciling draft RFC 0009 as needed.
10. **Skeletal animation:** <a id="skeletal-animation"></a> extend glTF rendering with skins, joint hierarchies, inverse-bind matrices, GPU skinning, and animation sampling/playback; add blending in a later increment after basic playback is correct. Validate bind poses, node/joint transforms, and animation timing independently of physics.

Keep the simple Textured Cube lighting reference. Present each selected stage as an individual technical example, with normal mapping + skybox combined in one example. Share reusable engine facilities; keep scene setup, assets, shaders, and feature-specific pass ordering example-owned. Preserve earlier working examples as learning references. Choose each directory/name when implementation begins; the README lists planned examples without creating empty source directories. Demonstrate each stage before layering on the next. Camera2D, mouse aiming, sprites, and parallax remain future 2D foundations in draft RFC 0005. Significant API/dependency decisions need their own RFCs; this sequence does not implement or accept their designs.

### Grass experiment scope and follow-ups

The selected Instanced Grass example targets the full-field appearance below. A small patch can validate implementation first, but the expected output is continuous grass coverage to the horizon; local interaction remains a follow-up:

1. **Layered grass coverage:** use a seamless green grass texture on the ground beneath instanced blades/clumps, matching color and scale so gaps remain covered. Validate a small patch first, then extend the visible field without an exposed rectangular edge. Keep placement, variation, controls, and grass appearance example-owned.
2. **Wind and appearance:** animate blade vertices using time, world position, height along the blade, wind direction/strength, and per-instance variation. Study colors, blade shapes, density, and simple lighting. Keep this first increment a visual animation experiment; it does not require per-blade rigid bodies, a fluid solver, or general terrain generation.
3. **Local interaction (later):** bend nearby grass around a moving sphere or character marker, using a bounded influence position/radius. Introduce persistent interaction data only if a later observable effect requires it; a complete character controller or physics system is unnecessary for the first interaction test.
4. **Field coverage and scaling:** divide the field into patches, skip invisible patches, reduce distant density/detail, and profile geometry, overlapping fragments, CPU submission, and memory use. Validate scaling after the small patch behaves correctly; this is required for the horizon-reaching target, with no fixed performance claim.

The visual target is grass inspired by Breath of the Wild, not a claim about Nintendo's implementation or a commitment to reproduce the entire game's vegetation/world systems. The intended first grass experiment uses conventional OpenGL 3.3 rendering; compute, mesh shaders, and ray tracing are not prerequisites. Exact field size, density, and frame rate must be measured on the development machine.

Completion of the small 3D step means an inspectable, controllable scene with correct depth/transforms and preserved existing examples. Completion of the grass example means continuous green coverage to the horizon, observable wind controls, consistent root anchoring/orientation, and checked patch culling and distance transitions. Local interaction and persistent deformation are separate increments. Keep temporary validation under ignored `build/`; example directories/assets are added when implementation begins.

Revisit Asteroids when a technical scene has demonstrated 2D camera, mouse/input, sprite drawing, and the intended lighting workflow. Revisit Mini Artillery when the required 2D rendering, terrain geometry, and physics foundations have been demonstrated in bounded experiments. These are readiness guides, not a requirement to finish every planned engine system before either game resumes. Preserve both scaffolds and concepts. The selected water step demonstrates a bounded surface simulation; larger fluid experiments remain later possibilities. The bounded physics step above does not resume either game automatically; select one objective at a time.

Phase numbers remain thematic guidance. The 2D Asteroids proposal shares the implemented input/timing foundation but is deferred alongside Mini Artillery; neither follows the cube/lighting step automatically. Metal/Vulkan implementation, shader cross-compilation, runtime backend selection, and further portability infrastructure are deferred. Choose a new active objective explicitly before pursuing them. Completed progress remains in the changelog.

## Phase 1 - OpenGL rendering owners

Use the existing OpenGL renderer to learn resource ownership and rendering. [RFC 0006](docs/rfcs/0006-opengl-focused-rendering.md) records the direct implementation; the [structure guide](docs/project-structure.md) documents the code. Completed work and validation stay in the changelog. The intended capabilities are:

- Window owns SDL/OpenGL context setup, GLAD, VSync, and presentation.
- Texture2D uploads/binds textures, ShaderProgram links/activates/writes uniforms, VertexBuffer owns vertex bytes, IndexBuffer owns indices, VertexArray configures layout/buffer associations, and Renderer submits sequential/indexed draws from VertexArray.
- CPU images and geometry remain independent of GL where practical.
- All examples use current engine operations, preserving assets, controls, backgrounds, and packaging.
- Native and engine-only builds, failure recovery, callbacks, pacing, and resource teardown are checked.

The previous device/frame/pipeline/backend experiment is preserved in `archive/multi-backend-experiment` and [RFC 9999](docs/rfcs/9999-multi-backend-rendering-research.md). Additional backend structure is deferred, not a prerequisite for learning OpenGL scenes.

## Phase 2 - Input, cameras, and sprites

With OpenGL resource ownership in place, small technical examples can introduce reusable 2D fundamentals before Asteroids resumes. Reconcile [draft RFC 0005](docs/rfcs/0005-camera-keyboard-and-sprites.md)'s drawing assumptions with the [current rendering interface](docs/rendering-guide.md) before implementing the renderer. Input state and camera math can be developed independently where useful. The intended capabilities are:

- A GLM-backed camera using explicit world and drawable-pixel coordinates.
- Keyboard state, cursor-to-world conversion, and elapsed-time updates.
- Mouse heading, heading-relative WASD movement, and automatic camera following.
- Overlapping sprites and a scrolling starfield with parallax.
- A normal-mapped sprite-lighting prototype after the baseline scene works.
- Checked aiming, movement, resize behavior, focus handling, and continued operation of the other examples.

Demonstrate these facilities in a bounded technical scene first. The Asteroids-themed scene and complete game remain deferred consumers of the validated foundations; RFC 0005 must be reconciled with this sequencing before implementation.

## Phase 3 - Conventional 3D rendering

The next phase builds on rendering resources and camera/input foundations to explore perspective, geometry, and visibility. The intended capabilities are:

- A focused textured-cube example with a movable perspective camera.
- Indexed mesh drawing, depth testing, and face culling.
- Documented world, clip/depth, image, and framebuffer conventions.
- Camera calculations and CPU geometry that need no graphics context.
- Verified transforms, occlusion, texture orientation, aspect ratio, resizing, and existing-example behavior.

Extend the rendering interface through the needs of this small scene before pursuing advanced GPU features.

## Deferred graphics research

[WebGPU migration (RFC 9998)](docs/rfcs/9998-webgpu-migration-research.md) and [multiple graphics backends (RFC 9999)](docs/rfcs/9999-multi-backend-rendering-research.md) are research topics, not prerequisites for lighting, Asteroids, or grass. Retain C++17/OpenGL and add capabilities through concrete examples. Revisit these topics only when learning a new API, browser delivery, GPU compute, or a measured limitation becomes an explicitly selected objective. WebGPU research includes the [WebGPU Unleashed learning reference](https://shi-yan.github.io/webgpuunleashed/).

The Metal/Vulkan phases below preserve possible future directions; they are not scheduled implementation commitments.

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

- **Asteroids (deferred):** resume the scene after its 2D camera/input/sprite and intended lighting foundations are demonstrated; add weapons, asteroid behavior, collision, scoring, and game flow in later increments.
- **Mini Artillery (deferred):** resume after relevant 2D rendering, terrain geometry, and physics experiments; then develop aiming/projectiles, terrain carving, affected-chunk collision rebuilding, turns, and wind in bounded increments.
- **Voxel world:** chunk data, exposed-face meshing, block selection/editing, player collision, terrain generation, streaming, and saving after conventional 3D rendering works.
- Verified example-specific rules, matching terrain render/collision edits, fixed-step simulation where needed, and voxel chunk-boundary/persistence checks.

Define a playable or inspectable outcome for each increment before implementation. Keep game rules and assets in their examples, geometry/simulation testable independently of graphics where practical, and extract reusable facilities as needed. A voxel world does not require ray tracing or mesh shaders. Select new example directories when their implementation begins.

## Future experiments

Keep ideas here before committing them to a phase. Each entry records a small starting scope and possible follow-up work; it does not select an algorithm, library, graphics backend, or delivery date. Choose an experiment when its prerequisites and learning value fit the current work.

| Experiment | First scope | Possible follow-up | Foundations |
| --- | --- | --- | --- |
| Grass and vegetation | Selected after water: horizon-reaching grass-textured ground and instanced blades, wind, culling, and distance-based detail | Interactive bending and persistent deformation | Conventional 3D rendering, camera controls, shader parameters, and instanced drawing as field density grows; see the current OpenGL focus |
| Water and fluid simulation | Selected after physics: a bounded shallow-water height-field pool with disturbances, damping, and boundary reflection | Obstacles, body coupling, richer water rendering, or a separate particle/volume-fluid experiment | Fixed-step updates, surface mesh rendering, input, and ImGui controls; exact solver/stability policy needs an RFC; GPU compute is not a prerequisite |
| Physics sandbox | A 2D scene for dropping bodies, applying forces, and inspecting collisions | Constraints/joints, stacking, material properties, debug drawing, and a later 3D sandbox | Input, rendering, fixed-step simulation, and an appropriate physics integration |

For grass, distinguish visual wind animation from physical interaction. For water, choose explicitly between surface animation and simulated fluid motion before expanding the scope. For physics, retain the planned Box2D 3.x direction for 2D; choose a 3D library when the 3D experiment begins.

When selecting an idea, define its learning objective, prerequisites, and observable completion criteria, then place it in the intended sequence or develop it alongside a phase whose foundations are ready. Create its example directory only when work begins. Keep scene setup, controls, assets, and research notes in that example's README and `docs/`; extract reusable facilities into the engine as needed. Use an RFC for significant engine architecture, dependency, or algorithm decisions. Completed work stays in the changelog.

## Further directions

Audio, Dear ImGui tooling, richer asset workflows, browser builds, and 3D physics remain driven by example needs. Retain the planned Box2D 3.x and Clipper2 direction for appropriate 2D work; choose a 3D physics library when needed. Browser graphics and WASM dependencies require a separate design and verification effort. Multiplayer belongs to an example and needs explicit authority, ordering, and recovery rules.

Keep each increment small enough to explain and validate. Preserve pinned local dependencies, asset provenance, existing examples, and native/web output separation. Update this roadmap when priorities change; record completed progress in the changelog. Screenshots remain owner-supplied, and generated validation outputs stay under ignored `build/`.

Structure adapted from [Dovyski's roadmap template](https://github.com/Dovyski/template/blob/master/ROADMAP.md).
