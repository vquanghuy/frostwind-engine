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
5. **Post-processing:** <a id="post-processing"></a> introduce a resizable floating-point HDR scene target and explicit fullscreen passes for exposure, tone mapping, and FXAA, controlled through ImGui. Establish the linear-light input, color-versus-data texture, storage, and display-encoding contracts here, before PBR. Keep the skybox in the scene and render UI after scene effects. Preserve or explicitly compare existing MSAA; resolve any multisampled scene attachments before ordinary single-sample post-processing. Validate target completeness, resize/failure cleanup, unclipped HDR readback, exposure/tone curves, single display encoding, FXAA off/on image behavior and fine-detail softness, and crisp UI composition. HDR monitor output and a generic render-graph/effect framework are outside this first increment. See the [technique scope table](#post-processing-techniques) for follow-ups.
6. **Physics simulation:** <a id="physics-simulation"></a> introduce a bounded simulation example with fixed timesteps, simple bodies/collision shapes, and synchronization between simulation and rendered transforms. Use ImGui for inspection. Select 2D/3D scope and a suitable library when this work begins; Box2D 3.x remains the planned 2D direction and the 3D library is undecided. Model importing and skeletal animation are not prerequisites for simple procedural bodies.
7. **Water simulation:** <a id="water-simulation"></a> build a bounded shallow-water height-field example after physics, using fixed simulation timesteps and ImGui controls. Start with a small pool, click-driven disturbances, propagating/interfering ripples, boundary reflection, and adjustable damping, disturbance strength, and wave speed. Use simple surface lighting to inspect the simulation before PBR. Define the numerical solver, stability limits, boundary conditions, and coordinate conversions in an RFC; a CPU implementation is a candidate, not a requirement for GPU compute or a new graphics backend. Validate rest-state stability, finite values, timestep limits, repeatable disturbances, and render-rate-independent updates. Reflections, refraction, foam, two-way body coupling, and freely splashing 3D fluid are separate later increments.
8. **Instanced grass:** <a id="instanced-grass"></a> demonstrate a continuous grassland reaching visually to the horizon, using a seamless grass-color ground texture beneath shared blade/clump geometry and GPU instanced drawing, with per-instance placement, rotation, scale, color, and wind phase. Blend nearby blades into the matching ground texture at distance so gaps and far views remain green. Use finite terrain patches, visibility culling, distance-based density/detail, and smooth transitions as part of this target; visually continuous coverage does not require infinite geometry. Animate upper vertices in the shader while roots remain anchored. Reuse the camera, directional lighting/shadow facilities, static skybox, elapsed-time animation, and ImGui density/wind controls. Introduce focused instance-buffer/attribute and draw operations when needed; retain existing non-instanced drawing. Validate instance transforms/counts, root anchoring, orientation, consistent deformation in visible/shadow passes, and bounded performance on the development machine. This step is visual wind animation, independent of the water solver and per-blade rigid-body physics. Local interaction and persistent bending follow separately; compute and newer graphics APIs are not prerequisites.
9. **Instanced particles:** <a id="instanced-particles"></a> reuse the instanced geometry/attribute/draw operations introduced for grass to render one bounded spark emitter with camera-facing textured quads. Add dynamic per-instance position, size, color, age/lifetime, and explicit emission/update/recycling behavior; keep emitter rules and effect appearance example-owned. Start with additive blending, depth testing against the scene, and depth writes disabled for particles; restore documented state afterward. Texture alpha must modulate contributions, including additive sparks. Conventional alpha-blended smoke and its ordering policy can follow separately. Validate billboard orientation, instance updates/counts/capacity, lifetime expiry, frame-rate-independent motion/emission, occlusion, and blending/state behavior. Do not infer instancing or performance from the concept image; collisions, soft particles, GPU compute, and volumetric fluid/fire simulation are separate increments.
10. **Physically based rendering (PBR):** <a id="physically-based-rendering"></a> reuse the linear-light/color-texture, HDR, and output-encoding contracts established during post-processing, then add a bounded metallic-roughness material/shading path. Prove it on procedural geometry with direct lighting first, then add a simple alpha-blended test pane with visible geometry behind it as the baseline for see-through model windows. Define opaque/transparent draw ordering, alpha convention, depth-test/write behavior, and sorting limitations explicitly; validate from multiple camera angles before loading the car. Environment lighting, reflection prefiltering, and image-based lighting require their own scoped decisions; the existing static skybox does not supply them automatically.
11. **glTF scene + resource management + basic ECS:** <a id="gltf-model-rendering"></a> develop one example through three checkpoints: load one static glTF model, share its resources between several model instances, then organize those instances with a small entity/component system. Support a documented subset of geometry, indices, textures, metallic-roughness materials, and internal node transforms using the PBR foundation. Include core material alpha modes and double-sided handling in the loader contract. See-through Nissan windows belong in the static-model acceptance checks; inspect the actual file before deciding whether core alpha blending suffices or a material extension is required. Keep glass behavior in material settings, referenced through the model, without adding a dedicated Glass ECS component. Keep resource loading/ownership separate from ECS; entity transforms and model references use explicitly valid resource lifetimes. Start with safe entity identity/lifetime, transform/model-reference components, basic queries, and safe structural changes; optional physics components should reuse existing physics integration. Define importer/dependency policy, synchronous asset ownership/sharing, unsupported-feature handling, and asset licenses before integration, reconciling draft RFC 0009 without accepting it wholesale. See the [milestone checkpoints](#gltf-resources-and-basic-ecs).
12. **Skeletal animation + incremental ECS:** <a id="skeletal-animation"></a> extend glTF rendering with skins, joint hierarchies, inverse-bind matrices, GPU skinning, and animation sampling/playback. Add only the ECS facilities the example needs: per-entity animation-player state and explicit update ordering between animation, transforms, rendering, and physics synchronization when present. Instances can share immutable model/clip resources while retaining independent playback time and evaluated poses. Keep model joints/internal nodes within the model instance rather than automatically making every joint an entity. Validate bind poses, node/joint transforms, independent playback, shared-resource lifetime, and animation timing independently of physics; blending and broader ECS features remain later increments.

Keep the simple Textured Cube lighting reference. Present each selected stage as an individual technical example, with normal mapping + skybox combined in one example. Share reusable engine facilities; keep scene setup, assets, shaders, and feature-specific pass ordering example-owned. Preserve earlier working examples as learning references. Choose each directory/name when implementation begins; the README lists planned examples without creating empty source directories. Demonstrate each stage before layering on the next. Camera2D, mouse aiming, sprites, and parallax remain future 2D foundations in draft RFC 0005. Significant API/dependency decisions need their own RFCs; this sequence does not implement or accept their designs.

### Post-processing techniques

The first example selects **HDR rendering + exposure/tone mapping + FXAA**. The table records candidate additions, not a commitment to implement every effect in the first increment. Keep concrete framebuffer/texture owners and explicit example pass ordering; introduce shared helpers only through actual callers.

| Technique | Purpose | Scope and requirements |
| --- | --- | --- |
| HDR rendering | Preserve lighting values above 1 in a floating-point scene target | Foundation in the first increment; this is internal scene precision, not HDR monitor output |
| Exposure + tone mapping | Adjust scene brightness and compress HDR values into a displayable range | First increment; select/document the curve and verify linear-light handling and display encoding |
| FXAA | Filter visible edges in the tone-mapped scene image | First increment; fullscreen GLSL pass over a single-sample color image, using the chosen variant’s expected encoding; compare with no AA/MSAA and inspect fine-detail softness and remaining temporal shimmer |
| Bloom | Spread bright scene contributions into a restrained glow | Next increment after the baseline works; filter/combine HDR contributions before tone mapping, with adjustable strength and radius |
| Color grading | Adjust color balance, contrast, and saturation | Optional follow-up; define its working color space and position in the chain; LUT support only if needed |
| Vignette | Apply artistic edge darkening | Optional follow-up; tunable and disabled by default |
| Depth of field | Blur regions away from a selected focus distance | Later; sample scene depth, define focus/aperture behavior, and validate depth-edge bleeding |
| Motion blur | Filter along screen-space movement | Later; define motion inputs and history/reset behavior; depth-reconstructed camera motion alone does not represent independently moving objects |

The intended flow is linear HDR scene rendering (including skybox and particles when present), optional MSAA resolve, optional HDR bloom, exposure/tone mapping, FXAA with its defined color-encoding contract, presentation, then UI composition. Establish conversion and framebuffer state explicitly so scene display encoding happens once and UI remains correct. Optional grading/vignette placement depends on their working color space. Do not apply scene exposure or FXAA to ImGui text.

FXAA is feasible within OpenGL 3.3: NVIDIA’s reference includes GLSL 1.30-and-later support and processes a single-sample color image in one fullscreen pass. It does not require compute shaders or guarantee elimination of moving-grass shimmer. Review source licensing/provenance and pin any adopted implementation before integration. See the [NVIDIA FXAA white paper](https://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf). [Khronos framebuffer color-space rules](https://wikis.khronos.org/opengl/Framebuffer) and [multisample resolve rules](https://wikis.khronos.org/opengl/Multisample) inform the target/encoding contract.

Further technique references: [NVIDIA real-time glow](https://developer.nvidia.com/gpugems/gpugems/part-iv-image-processing/chapter-21-real-time-glow), [post-process depth of field](https://developer.nvidia.com/gpugems/gpugems3/part-iv-image-effects/chapter-28-practical-post-process-depth-field), and [motion blur](https://developer.nvidia.com/gpugems/gpugems3/part-iv-image-effects/chapter-27-motion-blur-post-processing-effect). These explain techniques; reference availability does not validate Frostwind’s implementation or select new dependencies.

### glTF resources and basic ECS

Treat the combined glTF milestone as three reviewable checkpoints in one example:

1. **Static model:** load and draw one documented glTF subset; verify geometry, materials, texture semantics, node transforms, errors, and context-safe cleanup. The intended asset is the owner-selected [Nissan R34 Brians Fast Furious model](https://sketchfab.com/3d-models/nissan-r34-brians-fast-furious-wwwvecarzcom-412aa249c2f548eebf64bdb35f3553af). Validate the loader with a minimal fixture first, then inspect/import this model’s actual GLB, material extensions/transparency, textures, normals/tangents, hierarchy, and resource costs. Preserve creator credit, the exact asset license/version, source URL, and modifications in the owning example’s asset record before distributing runtime files; the current license information comes from the owner-supplied model-details screenshot, not a verified download.
2. **Shared resources:** place multiple model instances with different transforms while sharing meshes, textures, and materials. Establish synchronous loading, stable references, duplicate-load policy, and teardown without premature release or double deletion. Keep resource management a focused facility rather than a universal engine manager; background streaming and hot reload are separate work.
3. **Basic ECS:** add safe entity identity/lifetime, transform and model-reference components, basic iteration/queries, and explicit safe mutation boundaries. Demonstrate creating, moving, and destroying independent objects sharing one model; add optional physics references only when used. The imported model’s node hierarchy stays within the model. Existing minimal examples retain their direct ownership/rendering flow, and grass blades/individual particles remain compact system-owned arrays.

For transparent materials, parse glTF `OPAQUE`, `MASK`, and `BLEND` semantics, base-color alpha, cutoff, and `doubleSided` consistently. Start the blended pass after opaque geometry and the background, before scene post-processing, with depth testing enabled, an explicit depth-write policy, and back-to-front ordering where applicable. Simple object sorting has limitations for overlapping/intersecting glass; document the supported cases rather than claiming general transparency correctness. Verify interior visibility, overlapping panes, opaque occlusion, camera movement, and state restoration. Particle additive blending alone does not establish this material path. Core alpha blending is an appearance approximation for glass, not a physically complete transmission model; see [glTF alpha coverage](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#alpha-coverage).

If the selected GLB uses `KHR_materials_transmission` or another unsupported glass extension, record the gap and decide an explicit, scoped implementation or documented approximation before accepting the model. Do not silently reinterpret transmission as base-color opacity. See the [Khronos transmission specification](https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_transmission). Advanced refraction, thickness/absorption, environment reflections, and physically meaningful glass shadows remain future work.

Skeletal animation then adds per-instance playback state and explicit system ordering through a real caller. It does not require implementing the [advanced ECS candidates](#advanced-ecs-candidates) first. Architecture and any ECS/importer dependencies need focused RFC decisions when the milestone begins; this roadmap update selects scope without implementing it or selecting a library.

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

The primary 12-stage OpenGL learning sequence (including baseline Physics Simulation, shallow Water height fields, and horizon-reaching Instanced Grass) is committed under [Current focus - OpenGL scenes](#current-focus---opengl-scenes).

Keep exploratory follow-up increments and uncommitted ideas here before assigning them to a phase. Each entry records a small starting scope and possible follow-up work; it does not select an algorithm, library, graphics backend, or delivery date. Choose an experiment when its prerequisites and learning value fit the engine's progress.

| Experiment | First scope | Possible follow-up | Foundations |
| --- | --- | --- | --- |
| **[Deferred rendering](#deferred-rendering)** | A bounded many-light scene with a G-buffer geometry pass, separate lighting pass, and matched forward-rendering comparison | Light volumes/culling and other measured optimizations after correctness; no automatic renderer replacement | PBR, glTF scene/resource sharing/basic ECS, HDR/post-processing, multiple render targets and ImGui inspection |
| **Advanced glass materials** | A dedicated transmission/refraction test with a known material and controlled background | Thickness, absorption, richer reflections, overlapping glass, and glass-shadow treatment | PBR/basic transparent test pane, glTF material inspection, explicit color/depth passes; select supported extensions and limitations before implementation |
| **Interactive grass deformation** | Radial blade bending around moving character/sphere markers via vertex shader uniforms | Persistent trampling trails via dynamic displacement render targets and gradual spring-back recovery | [Instanced Grass (Stage 8)](#instanced-grass), player position inputs; avoids rigid-body physics for vegetation |
| **3D fluid simulation & coupling** | Two-way coupling between floating rigid bodies and surface height fields | 3D particle/SPH or Eulerian grid fluid, volumetric splashing, foam generation, and screen-space water refraction | [Water Simulation (Stage 7)](#water-simulation), [Physics Simulation (Stage 6)](#physics-simulation) |
| **Voxel world & meshing** | Chunk data structure, exposed-face meshing, and basic block picking/editing | Procedural Perlin/Simplex terrain generation, chunk streaming/caching, and world persistence | Conventional 3D rendering, camera controls, Ray-AABB intersection; mesh shaders are not a prerequisite |
| **Volumetric effects & soft particles** | Depth-faded soft particles (preventing sharp geometry intersections) and alpha-blended smoke | GPU compute particle simulation, vector field advection, and illuminated smoke | [Instanced Particles (Stage 9)](#instanced-particles), depth buffer texture sampling |
| **Audio system integration** | SDL3 audio playback for 2D sound effects (WAV) and looping background music | 3D positional audio, distance attenuation, and SDL3_mixer integration if richer mixing is needed | SDL3 application lifecycle; keep audio thread-safe and decoupled from rendering frame rates |
| **Multiplayer networking** | Authoritative client-server loop with simple state replication and snapshot interpolation | Client-side prediction, entity interpolation, lag compensation, and packet loss recovery | Fixed simulation timesteps, deterministic serialization; optional example-level feature |

For grass, distinguish visual wind animation and radial vertex pushing from rigid-body physics. For fluid, validate surface height fields before pursuing volumetric simulation. For physics, retain Box2D 3.x for 2D and select a 3D library when 3D body experiments begin.

When selecting an idea, define its learning objective, prerequisites, and observable completion criteria, then place it in the intended sequence or develop it alongside a phase whose foundations are ready. Create its example directory only when work begins. Keep scene setup, controls, assets, and research notes in that example's README and `docs/`; extract reusable facilities into the engine as needed. Use an RFC for significant engine architecture, dependency, or algorithm decisions. Completed work stays in the changelog.

### Deferred rendering

Consider this as a separate technical example after the model/resource/ECS foundations, with a finite many-light scene rather than replacing the active forward path. Reuse the Nissan R34 asset selected for the glTF example, its verified materials/resources, and the same scene setup for a consistent comparison; the current concept is not a verified GLB render. Render opaque geometry into a documented G-buffer, then evaluate lighting in a separate pass. Decide which surface/material inputs to store, whether to reconstruct position from depth, and the coordinate-space, precision, bandwidth, resize, and clear-value contracts in a dedicated RFC. Preserve the existing forward examples and avoid backend/device abstractions.

Use the same geometry, materials, camera, light inputs, and output/tone-mapping settings for a forward-versus-deferred comparison. Expose final lighting and G-buffer views through ImGui. Verify attachments/formats and depth reconstruction, normal/material readback, lighting parity, target recreation, and initialization/teardown behavior before measuring CPU/GPU time and memory over representative object/light counts. More lights do not guarantee a performance win; no target count or timing is selected by the concept.

Define the treatment of the skybox and forward-rendered transparent objects/particles explicitly, including shared depth and blending. MSAA/G-buffer handling and per-light shadowing need separate scope decisions; this example does not promise shadows from every local light. HDR and post-processing remain separate stages and do not themselves imply deferred shading. The [README expected-output concept](README.md#engine-and-examples) illustrates the intended many-light scene, not proof of its architecture, G-buffer values, or performance.

### Advanced ECS candidates

These capabilities are future experiments, not prerequisites for the glTF/basic-ECS or skeletal-animation milestones. Expand the ECS through demonstrated example needs; select an observable result and validation scope before each addition.

| Capability | What it enables | When to consider it |
| --- | --- | --- |
| Entity transform hierarchy | Parent/child scene objects and propagation of world transforms | Scene-object parenting needs beyond the model’s internal glTF hierarchy |
| Deferred structural changes | Queue spawn/destroy and component changes while systems iterate | Richer mutation workflows; the basic ECS must already define safe mutation boundaries |
| Richer queries | Required, optional, or excluded components and selective query caching | Concrete systems outgrow basic iteration; profile before caching |
| System scheduling | Explicit dependencies and ordered phases for input, physics, animation, transforms, and rendering | Grow from explicit animation update ordering; add a scheduler only when manual orchestration becomes insufficient |
| Scene serialization | Save/load entities and components with stable asset references | A scene-authoring or persistence example needs it; define schema/version and reference recovery rules |
| Prefabs | Reusable object configurations with instance overrides | Repeated authored object setups justify an override/lifetime contract |
| Editor integration | Entity/component inspection, hierarchy editing, and relationship visualization through ImGui | Inspectors and authoring tasks exceed the existing example-owned debug panels |
| Performance tooling and storage evaluation | Measure system time, query cost, allocation, and component layout; compare storage choices | Representative scenes reveal bottlenecks; archetype/chunk storage is a design option, not an automatic requirement |
| Parallel execution | Run independent systems concurrently with explicit dependencies and safe access | Measured workloads justify scheduling/data-access complexity; keep OpenGL/context operations within their required thread/lifetime |

Resource loading and GPU ownership stay separate from ECS orchestration. Components reference resources with explicit lifetime rules; ECS does not absorb every engine subsystem. Streaming, hot reload, ragdolls, animation blending, and inverse kinematics require their own needs and scope. Preserve the current C++17/local-dependency policy and existing simple examples while evaluating any library or storage design.

## Further directions

Audio, Dear ImGui tooling, richer asset workflows, browser builds, and 3D physics remain driven by example needs. Retain the planned Box2D 3.x and Clipper2 direction for appropriate 2D work; choose a 3D physics library when needed. Browser graphics and WASM dependencies require a separate design and verification effort. Multiplayer belongs to an example and needs explicit authority, ordering, and recovery rules.

Keep each increment small enough to explain and validate. Preserve pinned local dependencies, asset provenance, existing examples, and native/web output separation. Update this roadmap when priorities change; record completed progress in the changelog. Screenshots remain owner-supplied, and generated validation outputs stay under ignored `build/`.

Structure adapted from [Dovyski's roadmap template](https://github.com/Dovyski/template/blob/master/ROADMAP.md).
