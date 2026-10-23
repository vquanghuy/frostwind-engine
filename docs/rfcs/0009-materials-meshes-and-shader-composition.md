# RFC 0009: Materials, meshes, models, and shader composition

- **Status:** Draft
- **Created:** 2026-10-21
- **Supersedes:** None
- **Superseded by:** None

## Problem

[RFC 0008](0008-basic-lighting.md) demonstrates lighting directly in Textured
Cube with example-owned shaders and uniform uploads. That small learning step
must not depend on an engine overhaul. Later, repeating geometry ownership,
surface sampling, texture/material bindings, and lighting code across examples
will justify reusable engine components.

This RFC captures the proposed structure and ownership discussed with the owner.
It is an architecture draft for follow-up work, not authorization to implement
these systems now. RFC 0008 was implemented independently after its 2026-10-22 mark; this
architecture still requires separate implementation authorization.

## Goals and scope

- Separate CPU geometry, GPU mesh resources, material surface inputs, scene
  illumination, and the shader program used to combine them.
- Use a shared `Material` description rather than make every material's identity
  depend on Phong. Keep shading-model response parameters explicit components.
- Let a Model own meshes/materials/textures, with separately placed instances.
- Share GLSL functions through engine-resolved includes and built-in templates
  while retaining complete custom shader control.
- Preserve C++17, local pinned dependencies, OpenGL 3.3 Core, existing resource
  owners, Renderer submission, and example/context lifetime rules.

No new graphics backend, renderer replacement, C++ upgrade, dependency, asset
importer, editor, ECS, scene graph, resource registry, shader graph, or material
compiler is selected. PBR, normal mapping, submesh draw ranges, and automated
model loading are future increments, not prerequisites for the first extraction.
Introduce each class/directory only when its implementation is needed.

## Reference designs and adaptation

The [Filament Materials Guide](https://google.github.io/filament/Materials.md.html)
distinguishes material definitions, instances, and prepared surface inputs.
Filament definitions include shader code and raster state and are compiled to
packages. Frostwind borrows the separation of parameter values, surface
preparation, and lighting, while retaining caller-owned programs and explicit
Renderer state. Its Material is not a Filament material definition or package.

[Filament's RenderableManager](https://github.com/google/filament/blob/main/filament/include/filament/RenderableManager.h)
associates geometry and material instances at the renderable primitive level.
[glTF meshes](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#meshes)
instead group primitives with geometry and material references, and nodes place
meshes through transforms. These illustrate that the term mesh has multiple
conventions; the Frostwind definitions below are explicit design choices.

The references support concepts, not this exact directory tree, borrowing
policy, or API. No Filament code, compiler, PBR model, or runtime dependency is
adopted. glTF is a reference, not a commitment to an importer.

## Proposed design

### Project structure

This is a proposed eventual layout; existing unrelated files are omitted.
Headers stay beside their implementations. Names and signatures remain subject
to review before implementation.

```text
engine/
├── CMakeLists.txt
├── src/
│   ├── core/, app/, platform/, camera/, input/  # Existing modules
│   ├── geometry/
│   │   └── MeshData.h                         # CPU vertices and indices
│   ├── rendering/                            # Existing OpenGL owners
│   │   ├── Mesh.h/.cpp                        # Compose buffers and VertexArray
│   │   ├── Renderer.h/.cpp
│   │   ├── ShaderProgram.h/.cpp
│   │   ├── Texture2D.h/.cpp
│   │   ├── VertexBuffer.h/.cpp
│   │   ├── IndexBuffer.h/.cpp
│   │   └── VertexArray.h/.cpp
│   ├── materials/
│   │   ├── Material.h                        # Common surface description
│   │   └── PhongParameters.h                 # Optional model-specific response
│   ├── lighting/
│   │   └── DirectionalLight.h
│   ├── models/
│   │   ├── Model.h/.cpp                      # Own meshes/materials/textures
│   │   └── ModelInstance.h                   # Borrow Model; store placement
│   └── shaders/                              # C++ source tooling and adapters
│       ├── ShaderSource.h/.cpp
│       ├── BuiltinShaders.h/.cpp              # Private embedded-text lookup
│       └── bindings/
│           ├── PhongLightingUniforms.h/.cpp
│           └── PhongMaterialBindings.h/.cpp
└── shaders/                                  # Authored GLSL outside C++ src/
    ├── materials/
    │   └── Surface.glsl
    ├── lighting/
    │   └── Phong.glsl
    └── templates/
        ├── Phong.vert
        └── Phong.frag
```

Normal-map GLSL may later live under `engine/shaders/surface/`; do not create a
placeholder directory now. Generated embedded shader data belongs under ignored
`build/`, not `src/`, `external/`, or the authored GLSL tree. Examples retain
their custom shaders, runtime assets/licenses, controls, and geometry generation.

### Component responsibilities

| Component | Responsibility | Does not own or decide |
| --- | --- | --- |
| MeshData | CPU vertex attributes and optional indices | GPU objects, material, shader, placement |
| Mesh | Own VBO, optional index buffer, and borrowing VertexArray | Material, shader, object transform |
| Material | Surface color and borrowed texture inputs; associated response component | Geometry, light, camera, shader program |
| PhongParameters | Specular color/strength and shininess for Phong | A universal definition of every material |
| DirectionalLight | World-space ray direction and color/intensity | Material or object settings |
| Model | Own meshes, materials, textures; associate mesh parts with materials | Placement of every instance or shader execution |
| ModelInstance | Reference a Model and store a model transform | Duplicate model GPU resources |
| ShaderSource | Resolve built-in/custom sources and preserve diagnostics | GPU resources or lighting policy |
| Binding helpers | Upload compatible material/light data into a borrowed program | Program lifetime, geometry submission |
| ShaderProgram | Existing shader compilation, linking, activation, and typed uniforms | Material identity or scene traversal |
| Renderer | Existing clear/depth/culling and draw submission | Model/material selection, implicit bindings |
| Example | Own scene resources, programs, instances, controls, and draw order | Engine internals or native GL calls |

Dependencies flow from example orchestration into these concrete facilities.
Source loading does not depend on lighting/material adapters. ShaderProgram
consumes resolved source without selecting Phong or any other surface model.
No universal manager or abstract backend is introduced.

### Geometry and GPU meshes

MeshData is context-free geometry, initially position/UV/normal vertices and
optional u32 indices. Finalize the typed vertex/layout contract before adding
it; do not assume every custom shader must use one fixed vertex type. Existing
raw buffer/layout operations remain available for custom attributes.

Mesh composes existing GPU owners and uploads validated geometry. Declare its
buffers before the borrowing VertexArray so destruction releases the array
first. It retains no CPU geometry copy unless a concrete caller needs one;
upload completion does not require MeshData to stay alive. Invalid geometry
or initialization failure must leave cleanup automatic and allow retry under
the existing resource conventions.

Mesh geometry is drawable once a compatible program, uniforms, textures, and
render state have been established. Prefer a borrowed VertexArray accessor
and existing `renderer.drawTriangles(mesh.vertexArray())` over a Mesh::render
method that silently selects a material/program. No Renderer mesh overload is
required for this composition.

Initially one Mesh represents one complete draw. A model with several material
parts can reference several meshes without changing the current whole-geometry
draw contract. Shared-buffer submeshes/ranges need a separate explicit decision;
do not smuggle range APIs or automatic batching into this extraction. Cube
generation remains in its example until repeated use warrants primitive helpers.

### Material and shading response

Material describes common surface inputs: base color and optional base-color
texture initially; a normal texture and strength belong here when normal mapping
is implemented. Material references textures but does not own duplicate GPU
copies. Several material instances may share one texture.

Keep Phong response as a named component containing specular color and shininess.
Material remains named Material; Phong is a calculation a compatible shader can
use. A future PBR response would describe roughness/metallic instead of silently
interpreting Phong shininess as roughness. Do not add unsupported response fields
or a generic string-to-any uniform container.

The exact representation of the associated response component is intentionally
unresolved: compare explicit composition with a typed response variant at
implementation design time. It must permit common Material surface inputs to
be used by custom shaders, with additional example-owned parameters as needed.
This draft does not commit a misleading universal Material struct containing
every possible shading-model field.

One Material value represents one surface's settings; there is no separate
Material/MaterialInstance inheritance hierarchy yet. Program selection remains
explicit example/render orchestration, not a hidden pointer from Material to
a PhongShader owner. The chosen shader and binding contract must agree with
the material's surface and response data.

### Model ownership and placement

For a standalone cube the example may own Mesh, Material, and Texture2D directly.
Introduce Model when grouping or reusing a complete asset is useful. Proposed
ownership for a self-contained Model is:

```text
Example / Application-derived scene
├── ShaderPrograms and their borrowing binding helpers
├── Model
│   ├── owned Textures
│   ├── owned Materials ──borrow──→ Textures
│   ├── owned Meshes
│   └── Parts: mesh index + material index
└── ModelInstances ──borrow──→ Model
    └── per-instance transform
```

A part associates geometry with a material; it need not own either. Model owns
its resources and defines their cleanup order. Use stable resource storage or
validated model-local indices so growing collections cannot invalidate texture
or part references. Texture owners must remain alive for material binding;
buffer owners must remain alive for their VertexArray. Model is non-copyable
as a GPU resource aggregate; move support requires explicit lifetime handling,
not implicit relocation of borrowed relationships.

ModelInstance borrows a Model that outlives it. Multiple instances share model
resources while using distinct transforms; instances do not mutate shared
materials to simulate per-instance overrides. Such overrides need a later
explicit design. Model is not required to own ShaderPrograms, a resource
registry, import logic, or a scene graph. Initial construction may be procedural.

All model GPU resources remain within the creating context lifetime and are
released before the base Window/context. Application does not acquire a hardcoded
scene/model manager. Cross-model texture sharing and asynchronous loading remain
separate ownership decisions, not reasons to add shared_ptr globally.

### Surface preparation, lighting, and binding

The GLSL flow is explicit:

```text
Material values + sampled textures + geometry inputs
→ prepared surface (color, world normal, response)
→ optional Phong evaluation with scene light and camera
→ custom output effects
```

Surface.glsl defines only the evaluated surface contract and useful preparation
functions; it does not force texture units or a lighting model. Phong.glsl
consumes the prepared values and Phong response, returning lit RGB. The caller
controls alpha, texture blending, procedural effects, deformation, and main().
Template shaders demonstrate the complete path and can be used directly or
copied/customized. Shared functions avoid GLSL inheritance or a program-owning
PhongShader wrapper.

PhongLightingUniforms uploads light/ambient data. Camera and transforms can
initially use ordinary uniform setters. PhongMaterialBindings consumes Material
surface inputs and its Phong response, uploads parameters, and binds textures.
Both cache locations for a borrowed initialized ShaderProgram and require that
program to be active; they neither activate/destroy it nor submit draws. Each
program has its own helpers; missing required uniforms fail clearly. Programs
must outlive their helpers. Material/texture references are consumed for a bind
and are not retained by the helper.

Sampler units are explicit configuration, not globally allocated slots. With
no color texture, a presence flag selects constant base color; stale previous
bindings must not affect output. With a texture, color is sampled and multiplied
by the material factor. Document changed texture-unit/binding state and preserve
Texture2D's sampling semantics. Custom shaders can use direct setters rather
than conform to an adapter they do not need.

### Engine-resolved includes and built-in shaders

Use engine expansion before ordinary GLSL compilation, without optional driver
include extensions:

```glsl
#version 330 core
#include <frostwind/lighting/Phong.glsl>
#include "surface/CustomSurface.glsl"
```

Angle-bracket names resolve only engine built-ins; quoted names resolve relative
to the including disk file or built-in virtual directory. No current-directory
fallback or custom shadowing of engine modules. Built-in files mirror the
`engine/shaders/` tree under the virtual `frostwind/` prefix.

Expand literal nested includes, ignoring include-like text in comments. Report
missing/malformed targets and canonical-path cycles with source names, lines,
and include chains; bound recursion and expanded byte size. Root sources retain
their version directive; included modules have no version or main. Include
guards handle repeated noncyclic modules. Preserve numeric GLSL #line mappings
and append source-ID/name tables to original driver logs.

Keep this a small include resolver rather than a second macro preprocessor.
The proposed initial rule expands includes textually, even inside inactive
conditional branches; document/test that limitation and reject macro-generated
include paths. Included cycles remain errors even if guards might suppress them
in a full preprocessor.

Embed an explicit built-in source manifest into private generated C++ data,
refreshing it when authored GLSL changes. No runtime checkout path or engine
shader-copy step is required. Proposed ShaderSource owns expanded text and
diagnostic source names. File/built-in loading is CPU-only; ShaderProgram can
accept resolved sources and file initialization can delegate to this loader.
Retain raw complete-source initialization without implicit include resolution.
API signatures and source expansion limits remain review items.

### Normal mapping and color semantics

Normal mapping adds optional material texture input, geometry tangent information
or another validated tangent basis, and surface-preparation decoding. It does
not add normal maps to a light or change the Phong reflection equation. Missing
maps use geometry normals. Tangent-space samples must become the world-space
normal consumed by lighting.

The [glTF material specification](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#material-normaltexture)
provides a reference for linear normal-texture channels and tangent conventions.
Frostwind must separately verify UV/row orientation, mirrored UVs/handedness,
nonuniform transforms, and normal strength. Establish color-versus-data texture
formats and output encoding before adding map support; current RGBA8 crate
rendering does not establish a correct linear-light pipeline. No normal-map
fields, tangent generation, PBR, or new textures are implemented by this draft.

## Alternatives

- **Continue example-local code indefinitely:** clearest for RFC 0008, but later
  repeats ownership and shader contracts; extract only demonstrated common needs.
- **Make Mesh own its material:** convenient for a small demo, but conflates
  reusable geometry with appearance. Model parts assign materials instead.
- **Make Model own every placed object's transform:** prevents independent reuse;
  separate shared resources from ModelInstance placement.
- **Introduce PhongMaterial as the only material type:** unnecessarily couples
  surface identity to one response model. Prefer Material plus explicit response.
- **Adopt Filament or its compiler:** valuable existing architecture, but changes
  dependency/tooling and learning scope. Use it as a reference here.
- **Introduce a generic material/shader graph or renderer facade:** larger than
  current needs; retain existing direct owners and explicit custom GLSL.

## Risks and limitations

Borrowed references can dangle during model mutation/movement or teardown; stable
storage, immutable usage boundaries, and context lifetime rules are essential.
Material flexibility can grow into an untyped parameter system or unnecessary
class hierarchy; settle the smallest typed response composition first.

MeshData vertex flexibility, Model part semantics, and shader/binding contracts
must be proven with callers before acceptance. This RFC is not implementation
ready while those open questions remain. The reference projects do not validate
Frostwind's API or platform behavior. Browser and other graphics APIs remain
deferred; native work establishes no new portability claim.

## Validation

Documentation checks now: links/anchors, references, numbering/status, and
consistency with RFC 0008's example-only scope. Future acceptance checks:

1. Preserve the directly lit cube from RFC 0008 and the unlit quad during each
   extraction. Build all native examples and an engine-only configuration.
2. Test CPU geometry validation and include resolution independently of GL.
3. Draw shared geometry with different materials/transforms, and shared textures
   with different colors/response values. Test textured/untextured switching.
4. Render multiple instances of one Model without resource duplication or shared
   material mutation. Validate part indices and reference stability as resources
   are assembled; verify initialization failure and context-safe cleanup.
5. Compile stock templates and a distinct custom shader using the same functions;
   check per-program uniform caches, active-program errors, explicit units,
   missing required parameters, and no stale bindings affecting output.
6. Verify include lookup/cycles/limits, source line diagnostics, embedding refresh,
   and launching built-in consumers outside the repository.
7. For a separately authorized normal-map increment, verify neutral/tilted maps,
   strength zero, missing maps, UV seams/handedness, and color/data conventions.

Record executed results only when implementation occurs. Keep validation outputs
under ignored build/ and preserve owner-supplied screenshots.

## Implementation plan

1. Review this architecture draft without starting engine work. RFC 0008 is
   implemented; preserve its validated direct-lighting behavior during extraction.
2. Resolve response representation, CPU vertex/layout, model/reference stability,
   and public include/binding contracts with concrete usage examples.
3. Obtain the owner's implementation mark for the agreed follow-up scope.
4. Extract source loading/built-in GLSL and the material/surface/lighting path in
   bounded changes, preserving the working examples throughout.
5. Introduce MeshData/Mesh when geometry grouping is useful; introduce Model and
   ModelInstance only with an actual multi-part or reused-model example. Avoid
   landing unused placeholder systems just to match the eventual tree.
6. Run relevant validation and update implemented architecture documentation and
   changelog with evidence. Normal mapping/importing/PBR need separate increments.

## Open questions

- What is the smallest typed representation of Material's response components
  that permits unlit, Phong, and custom surfaces without a universal field bag?
- What CPU vertex/layout contract should MeshData expose without restricting
  custom attributes or prematurely adding tangent data?
- Should initial Model collections be immutable after assembly, and what stable
  storage/index rules should apply to internal texture and part references?
- Which concrete example justifies Model/ModelInstance and material overrides?
- Finalize include-source APIs, expansion limits, sampler-unit contracts, and
  the color/data texture policy before the corresponding implementation.

## Reference documents

- [Filament](https://google.github.io/filament/) and
  [Materials Guide](https://google.github.io/filament/Materials.md.html): material
  definitions/instances and custom surface preparation.
- [Filament RenderableManager](https://github.com/google/filament/blob/main/filament/include/filament/RenderableManager.h):
  geometry/material associations at renderable primitive granularity.
- [glTF meshes](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#meshes) and
  [materials](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#materials):
  asset geometry, material references, placement, and texture conventions.
- [Khronos glShaderSource reference](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glShaderSource.xhtml):
  source strings supplied to the OpenGL compiler after engine resolution.
- [RFC 0003](0003-frostwind-engine-and-examples.md),
  [RFC 0006](0006-opengl-focused-rendering.md), and
  [RFC 0007](0007-textured-cube-and-perspective-camera.md): current architecture.
- [RFC 0008](0008-basic-lighting.md): direct lighting learning increment that
  remains independent of this proposal.

## Decision

Draft created at the owner's request to preserve the proposed engine structure,
components, ownership, and references separately from cube lighting. No engine
restructuring or lighting implementation is authorized by this document update.
Implementation remains on hold pending the owner's mark.
