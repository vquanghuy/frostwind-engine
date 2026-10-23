# Scene feature concept

[scene-features-concept-v1.png](scene-features-concept-v1.png) was generated with the built-in OpenAI image generation tool on 2026-10-23 at the owner’s request. It is planning artwork, not an engine screenshot, measured shader comparison, or runtime asset. This historical comparison is preserved separately from the [individual expected-output concepts](example-output-concepts.md) now used in the README. Distributed as a project documentation asset under the [MIT license](../../../LICENSE).

The visual reference is the project’s existing [crate texture](../../../examples/02-textured-cube/assets/textures/crate.png); its [asset record](../../../examples/02-textured-cube/assets/LICENSES.md) preserves provenance. The first comparison was corrected to flatten the shadow-only panel, then edited to use the teal-and-gold crate design.

## Final generation instructions

Edit the three-panel concept using the actual crate-face texture as reference. Preserve the layout, headings, camera, ground, light, and cast shadow. Apply the honey-gold planks, worn teal perimeter/diagonal brace, corner plates, orange plate, and upward arrow to each visible face of a simple cube. The shadow-only panel uses the existing color texture with face normals and a flat color-textured ground; the other panels illustrate hypothetical matching normal maps, with the final panel adding a static skybox. Preserve straight geometry silhouettes. Do not add environment lighting, reflections, UI, or extra objects. Label the comparison as an illustrative concept.

## Interpretation and limitations

- Panel order illustrates feature contributions; the selected implementation order is normal mapping + skybox, then shadows. See the [roadmap](../../../ROADMAP.md#current-focus---opengl-scenes).
- The generated panels are qualitative and do not isolate shader terms numerically. Texture details and illumination can vary between panels.
- The existing color texture contains baked shading. A matching normal map is not implemented or supplied by this concept; clean albedo/normal assets still need preparation and validation.
- Normal maps change surface lighting, not geometry silhouettes or shadow-map geometry. Depicted fine relief is illustrative; braces/rivets would need geometry or a separately selected self-shadowing method to cast small physical shadows.
- The skybox is background imagery. Reflections and environment lighting are separate features.
