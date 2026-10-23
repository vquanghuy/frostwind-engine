# RFC 0008: Directional Phong lighting in Textured Cube

- **Status:** Implemented
- **Created:** 2026-10-21
- **Supersedes:** None
- **Superseded by:** None

## Problem

Textured Cube previously sampled crate textures without accounting for surface orientation or camera position. Before extracting reusable material, mesh, and shader composition systems across the engine, Frostwind required a practical demonstration of directional lighting and camera-dependent highlights on an existing 3D scene.

Attempting to build engine-wide material abstractions, mesh loaders, and shader libraries at the same time would obscure that learning objective. This RFC records the architectural decisions, resource contracts, shader pipeline, and vertex layout used to implement directional Phong lighting directly within the example.

## Goals and scope

### Goals
- Demonstrate one directional light with ambient fill, Lambert diffuse shading, and per-fragment Phong specular highlights on the existing textured cube.
- Preserve the engine/example ownership boundary defined in [RFC 0003](0003-frostwind-engine-and-examples.md): keep light parameters, surface values, uniform locations, face geometry, and shaders inside `examples/02-textured-cube/`.
- Upload uniforms directly through `ShaderProgram` without modifying engine public APIs.
- Retain existing controls, trilinear texture sampling with 16× anisotropy, 4× MSAA, depth testing, back-face culling, frame pacing, and lifecycle safety.

### Non-Goals
- No reusable engine material, mesh, model, light, or binding abstractions (deferred to [RFC 0009](0009-materials-meshes-and-shader-composition.md)).
- No normal mapping, point or spot lights, shadow maps, PBR, HDR tone mapping, or lighting debug UI.
- No changes to third-party dependencies: preserve C++17, pinned GLM, OpenGL 3.3 Core, and GLSL 330.

---

## Proposed design

For conceptual vector diagrams, mathematical formulas, and the geometric foundations of the Phong reflection model, see [Basic Lighting Theory](../concepts/lighting-theory.md).

### 1. Coordinate Space Selection: World Space

Lighting is evaluated in **World Space**:
- `uLightDirection` remains constant in world coordinates regardless of camera motion or cube rotation.
- Camera position is supplied via `uCameraPosition` (queried each frame from `PerspectiveCamera::position()`).
- Orbiting the camera dynamically changes the specular term while keeping ambient and diffuse contributions stationary relative to the cube faces.
- World space provides an intuitive coordinate frame for directional light rays and aligns with future multi-object scene placement.

### 2. Geometry and Vertex Layout

To maintain flat cube faces with sharp 90-degree edges, each face requires its own independent 4 vertices. The cube retains 24 vertices and 36 indices, with an outward unit normal assigned to each vertex.

**C++ Vertex Struct:**
```cpp
struct Vertex {
    frostwind::f32 x, y, z;    // Attribute 0: local position (offset 0)
    frostwind::f32 u, v;        // Attribute 1: UV coordinates (offset 12)
    frostwind::f32 nx, ny, nz;  // Attribute 2: surface normal (offset 20)
};
static_assert(sizeof(Vertex) == 32, "Vertex must be 32 bytes with no padding");
```

**Attribute Configuration in `CubeDemo::initialize()`:**
```cpp
vertexArray_.initialize(vertices_, indices_, {sizeof(Vertex), {
    {0, 3, offsetof(Vertex, x)},   // layout(location = 0) in vec3 aPosition
    {1, 2, offsetof(Vertex, u)},   // layout(location = 1) in vec2 aUv
    {2, 3, offsetof(Vertex, nx)},  // layout(location = 2) in vec3 aNormal
}});
```

### 3. Shader Pipeline Specification

The example owns its shader files under `examples/02-textured-cube/assets/shaders/`.

#### Vertex Shader (`cube.vert`)
Transforms vertex positions and normals into world space. Normals are transformed using the inverse transpose of the upper-left $3 \times 3$ model matrix to preserve perpendicularity under non-uniform scale:
```glsl
#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec2 vUv;
out vec3 vWorldPosition;
out vec3 vWorldNormal;

void main()
{
    vec4 worldPosition = uModel * vec4(aPosition, 1.0);
    vWorldPosition = worldPosition.xyz;
    // Inverse transpose keeps normals perpendicular under nonuniform scale.
    vWorldNormal = transpose(inverse(mat3(uModel))) * aNormal;
    vUv = aUv;
    gl_Position = uProjection * uView * worldPosition;
}
```

#### Fragment Shader (`cube.frag`)
Computes ambient, diffuse, and specular terms. Re-normalizes interpolated normals, guards against zero-distance camera vectors, and suppresses specular highlights on surfaces facing away from the light:
```glsl
#version 330 core

in vec2 vUv;
in vec3 vWorldPosition;
in vec3 vWorldNormal;

uniform sampler2D uTexture;
uniform vec3 uCameraPosition;
uniform vec3 uLightDirection;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;
uniform vec3 uSpecularColor;
uniform float uShininess;

out vec4 fragColor;

void main()
{
    vec4 surface = texture(uTexture, vUv);
    vec3 normal = normalize(vWorldNormal);
    // The supplied direction follows the rays; lighting needs surface-to-light.
    vec3 lightDirection = -normalize(uLightDirection);
    float diffuseFactor = max(dot(normal, lightDirection), 0.0);
    vec3 ambient = uAmbientColor * surface.rgb;
    vec3 diffuse = diffuseFactor * uLightColor * surface.rgb;

    vec3 specular = vec3(0.0);
    vec3 toCamera = uCameraPosition - vWorldPosition;
    float cameraDistanceSquared = dot(toCamera, toCamera);
    if (diffuseFactor > 0.0 && cameraDistanceSquared > 0.0) {
        vec3 viewDirection = toCamera * inversesqrt(cameraDistanceSquared);
        vec3 reflectedDirection = reflect(-lightDirection, normal);
        float specularFactor = pow(max(dot(reflectedDirection, viewDirection), 0.0), uShininess);
        specular = uSpecularColor * specularFactor * uLightColor;
    }

    fragColor = vec4(ambient + diffuse + specular, surface.a);
}
```

### 4. Direct Uniform Contract and Lifecycle Management

Uniforms are split between one-time initialization uploads and per-frame dynamic uploads:

| Uniform | GLSL Type | Lifecycle | Applied Value in Scene | Description |
| :--- | :--- | :--- | :--- | :--- |
| `uTexture` | `sampler2D` | Static (`initialize`) | `0` | Texture unit 0 |
| `uLightDirection` | `vec3` | Static (`initialize`) | `(0.4, -0.1, -0.6)` | Ray travel direction from source toward scene |
| `uLightColor` | `vec3` | Static (`initialize`) | `(0.8, 0.8, 0.8)` | Directional light intensity and color |
| `uAmbientColor` | `vec3` | Static (`initialize`) | `(0.192, 0.192, 0.192)` | Constant background fill |
| `uSpecularColor` | `vec3` | Static (`initialize`) | `(0.8, 0.8, 0.8)` | Specular highlight strength and tint |
| `uShininess` | `float` | Static (`initialize`) | `16.0` | Phong specular highlight exponent |
| `uModel` | `mat4` | Dynamic (`render`) | Matrix | Translation $\times$ Rotation $\times$ Scale |
| `uView` | `mat4` | Dynamic (`render`) | Matrix | `camera_.view()` |
| `uProjection` | `mat4` | Dynamic (`render`) | Matrix | `camera_.projection()` |
| `uCameraPosition` | `vec3` | Dynamic (`render`) | Vector | `camera_.position()` |

The shallow light direction `(0.4, -0.1, -0.6)` and broader shininess exponent (`16`) ensure a visible specular highlight sweeps across side faces from the default orbit camera view.

### 5. Frame Flow and Teardown

```text
initialize:
    upload 24 position/UV/normal vertices and 36 indices
    configure VertexArray with 32-byte stride layout
    load crate texture (trilinear, 16x anisotropy)
    compile and link cube.vert / cube.frag
    cache all uniform locations; upload static light/surface parameters
update:
    process orbit camera input, pause/reset keys, and model rotation
render:
    set depth testing (LESS) and back-face culling (BACK, CCW)
    clear color buffer and clear depth buffer to 1
    activate ShaderProgram
    upload dynamic transforms and camera position
    bind crate texture to unit 0
    drawTriangles(vertexArray_)
```

Teardown ordering is strictly preserved: `VertexArray` is destroyed before the borrowed `VertexBuffer` and `IndexBuffer`, and all GPU resources are destroyed before the base `Window` OpenGL context.

---

## Alternatives

- **Engine-wide lighting and material systems now:** Rejected. Introducing `Material`, `Mesh`, or `DirectionalLight` classes at this stage would over-engineer a single-cube demo. Architecture extraction is deferred to [RFC 0009](0009-materials-meshes-and-shader-composition.md).
- **View-space lighting:** Evaluating lighting in view (eye) space eliminates the `uCameraPosition` uniform because the camera is at `(0, 0, 0)`. However, it requires transforming the directional light vector into view space on the CPU every frame. World-space lighting was chosen for intuitive direction vector conventions and simpler multi-object mental models.
- **CPU normal matrix calculation:** Calculating the normal matrix on the CPU avoids per-vertex `inverse()` in GLSL, but would require adding a `setMat3` setter to `ShaderProgram`. Accepted as temporary technical debt for a single 24-vertex cube; proper CPU normal matrix extraction is deferred to RFC 0009.
- **Blinn-Phong half-vector:** Modern shading models often prefer Blinn-Phong ($\mathbf{N} \cdot \mathbf{H}$) over classic Phong ($\mathbf{R} \cdot \mathbf{V}$) to avoid specular falloff anomalies when the viewer angle exceeds 90 degrees from reflection. Classic Phong was selected here to explicitly demonstrate the reflection vector $\mathbf{R}$ in education.

---

## Risks and limitations

- **Uncalibrated Color Pipeline:** The implementation operates on standard 8-bit RGBA sRGB texture data without linear-space conversion or HDR tone mapping. Bright sums can clamp to pure white. This is accepted as a documented approximation.
- **Per-Vertex Matrix Inverse:** Computing `inverse(mat3(uModel))` inside the vertex shader is inefficient for dense meshes. For a 24-vertex cube, the GPU cost is negligible, but this should not be carried forward into general mesh rendering.
- **Scope Limits:** No shadows, normal maps, or point/spot lights are supported.

---

## Validation

Implementation verified on native macOS (Apple M1 Pro). Executed results and test records are documented in [development.md](../development.md#cube-lighting-validation-2026-10-22):

1. **Geometry and Normals:** Confirmed 24 distinct vertices with outward face normals, counterclockwise winding, and valid UV mapping.
2. **Lighting Response:** Verified diffuse falloff against surface orientation; unlit faces receive ambient light only.
3. **Specular Dynamics:** Confirmed that pausing cube rotation while orbiting the camera causes the specular highlight to move across faces while ambient and diffuse stay static.
4. **Non-Uniform Scale Test:** Verified that applying non-uniform scale preserves normal perpendicularity via the inverse-transpose matrix.
5. **Numerical Shader Readback:** Offscreen frame validation verified color calculations within expected mathematical tolerances.
6. **Teardown Safety:** Confirmed GPU resources are cleanly released before context destruction on window close.

---

## Implementation plan

1. Obtain implementation authorization from owner (completed 2026-10-22).
2. Update cube vertex data with outward face normals and configure 32-byte stride layout in `CubeDemo.cpp` (completed 2026-10-22).
3. Implement world-space vertex outputs in `cube.vert` (completed 2026-10-22).
4. Implement ambient, diffuse, and guarded specular terms in `cube.frag` (completed 2026-10-22).
5. Cache uniform locations and upload static parameters during `CubeDemo::initialize()` (completed 2026-10-22).
6. Execute native verification checks and document results in `development.md` and `CHANGELOG.md` (completed 2026-10-22).

---

## Open questions

All implementation questions are resolved. The owner supplied a [lit milestone capture](../assets/milestones/textured-cube-lighting-2026-10-22.png) on 2026-10-22. Generalization into reusable engine components is tracked in [RFC 0009](0009-materials-meshes-and-shader-composition.md).

---

## References

- [Basic Lighting Theory](../concepts/lighting-theory.md) – mathematical derivations, vector tripwires, and diagrams.
- [RFC 0003: Frostwind engine and examples](0003-frostwind-engine-and-examples.md) – ownership boundary between engine and examples.
- [RFC 0006: OpenGL-focused rendering](0006-opengl-focused-rendering.md) – direct OpenGL resource owners.
- [RFC 0007: Textured cube, perspective camera, and elapsed-time updates](0007-textured-cube-and-perspective-camera.md) – cube geometry and camera foundation.
- [RFC 0009: Materials, meshes, models, and shader composition](0009-materials-meshes-and-shader-composition.md) – future engine lighting and material abstractions.
- [Filament Materials Guide](https://google.github.io/filament/Materials.md.html) – industry reference for material and lighting separation.

---

## Decision

Accepted and implemented on 2026-10-22. Textured Cube owns face normals, world-space shaders, and direct uniform uploads. Engine public APIs remain unchanged. Reusable material and mesh systems remain deferred to RFC 0009.
