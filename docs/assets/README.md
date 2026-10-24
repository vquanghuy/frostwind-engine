# Documentation images

## Toybox logo

[`toybox-logo.png`](toybox-logo.png) is the original Toybox logo, previously used in the root [README](../../README.md). It was generated with OpenAI imagegen on 2026-10-14 and selected by the project owner. The design shows an open wooden toy chest containing a sword, toy tank, and ball beside a colorful Toybox wordmark. It is a branding asset, not a gameplay screenshot.

## Frostwind logo

[`frostwind-logo.png`](frostwind-logo.png) is the Frostwind logo used in the root README. It was generated with OpenAI imagegen on 2026-10-14 using the direction “wind-carved ice crystal, glacier-blue facets, restrained silver Frostwind wordmark, transparent background.” The original Toybox logo is preserved; the engine has since been renamed to Frostwind.

## Lighting theory

[`lighting/basic-lighting-theory.svg`](lighting/basic-lighting-theory.svg)
illustrates the ambient, diffuse, and specular contributions explained in
[Basic Lighting Theory](../concepts/lighting-theory.md) and implemented in
[RFC 0008](../rfcs/0008-basic-lighting.md), presenting qualitative crate panels alongside per-term formulas.
[`lighting/phong-vectors.svg`](lighting/phong-vectors.svg) illustrates the geometric direction vectors (N, L, D_ray, R, V) and angle relationships (θ, α) at surface fragment P.
[`lighting/normal-matrix-scale.svg`](lighting/normal-matrix-scale.svg) illustrates the effect of non-uniform scaling on surface tangents and normals, showing how the inverse-transpose normal matrix (M⁻¹)ᵀ restores perpendicularity.
The SVG diagrams are self-contained and need no external image loading.
The [illustration record](lighting/basic-lighting-theory-prompt.md) preserves
its prompts and the correction rationale. These are qualitative theory
illustrations, not screenshots, measured shader outputs, or runtime assets.
Example-owned cube lighting is implemented; the illustrations remain conceptual, not engine captures. The owner-supplied lit capture is recorded below.

## Planned scene features

- [`lighting/normal-mapping-skybox-concept-v1.png`](lighting/normal-mapping-skybox-concept-v1.png): expected output for the normal-mapping + skybox example, with no cast ground shadow.
- [`lighting/shadow-mapping-concept-v1.png`](lighting/shadow-mapping-concept-v1.png): expected output for the shadow-mapping example, adding a directional cast shadow to the scene.

- [`lighting/imgui-concept-v1.png`](lighting/imgui-concept-v1.png): Dear ImGui expected-output concept.
- [`lighting/post-processing-concept-v1.png`](lighting/post-processing-concept-v1.png): 2026-10-24 post-processing expected-output concept, retaining the crate/skybox scene with HDR, exposure, tone-mapping and FXAA controls.
- [`lighting/physics-simulation-concept-v1.png`](lighting/physics-simulation-concept-v1.png): Physics Simulation expected-output concept.
- [`lighting/water-simulation-concept-v2.png`](lighting/water-simulation-concept-v2.png): bounded shallow-water pool with ripples, simulation controls, and the mountain-and-sky background; version 1 is preserved in the provenance record.
- [`lighting/instanced-grass-concept-v2.png`](lighting/instanced-grass-concept-v2.png): continuous wind-swept grassland to the horizon, combining grass-textured ground and instanced blades, mountain-and-sky background, and ImGui controls. The earlier small-patch concept is preserved in the provenance record.
- [`lighting/instanced-particles-concept-v1.png`](lighting/instanced-particles-concept-v1.png): 2026-10-24 one-emitter spark concept with billboard sprites, additive blending controls, and the mountain-and-sky background.
- [`lighting/pbr-concept-v2.png`](lighting/pbr-concept-v2.png): PBR sphere grid with the mountain-and-sky background; version 1 is preserved in the provenance record.
- [`lighting/gltf-model-rendering-concept-v2.png`](lighting/gltf-model-rendering-concept-v2.png): generated Nissan R34 expected-output concept for the glTF loader target, based on the [owner-selected model listing](https://sketchfab.com/3d-models/nissan-r34-brians-fast-furious-wwwvecarzcom-412aa249c2f548eebf64bdb35f3553af). The earlier rover concept remains preserved in the provenance record; this is not a render of the downloaded GLB.
- [`lighting/skeletal-animation-concept-v1.png`](lighting/skeletal-animation-concept-v1.png): Skeletal Animation expected-output concept.

- [`lighting/deferred-rendering-concept-v2.png`](lighting/deferred-rendering-concept-v2.png): 2026-10-24 future many-light scene using the same Nissan R34 visual target as the glTF concept, retaining the mountain skybox and G-buffer/forward-comparison controls. Earlier rover concepts remain preserved in the provenance record.

These concepts, generated on 2026-10-23 and 2026-10-24, appear as individual previews in the root README. The first two and ImGui reuse the existing crate design; the others illustrate candidate technical scenes. They are planned visual targets, not screenshots. See [exact prompts, provenance, and limitations](lighting/example-output-concepts.md).

The earlier [`lighting/scene-features-concept-v1.png`](lighting/scene-features-concept-v1.png) triptych remains preserved as a feature comparison; it is not an individual example preview. Its [original provenance](lighting/scene-features-concept.md) records the comparison’s different panel order and limitations.

## Milestone captures

`milestones/` preserves project screenshots, including the original Mini Artillery identity, separate from the third-party references below. Historical images and captions are retained without relabeling. Screenshots appear directly in the [changelog](../../CHANGELOG.md). M001 and M002 were supplied manually from macOS window screenshots on 2026-10-13. M002 shows the native OpenGL checkerboard quad. [`milestones/m003-file-texture.png`](milestones/m003-file-texture.png) shows the Toybox textured-quad example loading the generated crate PNG. M003 was supplied by the owner from `Screenshot 2026-10-14 at 14.15.24.png` on macOS Apple M1 Pro on 2026-10-14 and is preserved as a historical capture.

[`milestones/m004-frostwind-textured-quad.png`](milestones/m004-frostwind-textured-quad.png) shows the same example after the Frostwind rename. The owner supplied it from `Screenshot 2026-10-14 at 14.56.04.png` on macOS Apple M1 Pro on 2026-10-14. It is embedded in the changelog and [example README](../../examples/01-textured-quad/README.md); M001–M003 remain unchanged.

[`milestones/m005-frostwind-textured-cube.png`](milestones/m005-frostwind-textured-cube.png)
shows the running Textured Cube example on macOS Apple M1 Pro. The owner supplied
it from Desktop `Screenshot 2026-10-20 at 15.11.17.png`, captured manually on
2026-10-20. The PNG is preserved byte-for-byte without cropping or re-encoding
and retained in the changelog's earlier-capture section with its original caption.

[`milestones/m005-frostwind-textured-cube-updated.png`](milestones/m005-frostwind-textured-cube-updated.png)
is the updated capture supplied from Desktop `Screenshot 2026-10-20 at 15.49.58.png`
on 2026-10-20, showing the running cube on macOS Apple M1 Pro. Its original PNG
is preserved byte-for-byte in the cube README history and the 2026-10-20
changelog entry. The earlier 15:11:17 capture remains preserved.
These images record rendered output, not frame-rate measurements or proof of
interactive checks. M001–M004 and their captions remain unchanged.

[`milestones/textured-cube-lighting-2026-10-22.png`](milestones/textured-cube-lighting-2026-10-22.png)
shows the lit Textured Cube on macOS Apple M1 Pro. The owner supplied Desktop
`Screenshot 2026-10-22 at 21.15.03.png`, captured manually on 2026-10-22 at
21:15:03. The original PNG is preserved byte-for-byte, without cropping or
re-encoding, and appears in the root README preview, cube README, and changelog.
It shows the example after lighting implementation and ambient/specular tuning;
it does not establish interactive behavior or performance. Earlier unlit
captures and their captions remain preserved. This adds no release or numbered
milestone designation.

## Example concepts and references

Planning artwork, generated sprite prototypes, and attribution live with the example that owns them:

- [Asteroids concept and reference inventory](../../examples/03-asteroids/docs/assets/README.md)
- [Mini Artillery concept and reference inventory](../../examples/04-mini-artillery/docs/assets/README.md)

Each example README embeds its preferred concept and links older versions. Concept images are labeled as design references and are separate from runtime textures and their future `assets/LICENSES.md` records. Historical milestone PNGs, captions, and branding remain here unchanged.
