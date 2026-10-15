# Asteroids concept and reference attribution

These planning images belong to the [Asteroids example](../../README.md). They are documentation assets and must not be copied into an executable’s runtime `assets/` directory.

## Asteroids visual reference

[`kenney-space-shooter-reference.png`](kenney-space-shooter-reference.png) is the pack example supplied by the owner on 2026-10-14 as Image #1 for the [planned Asteroids foundation](../../../../docs/rfcs/0005-camera-keyboard-and-sprites.md). It shows top-down spacecraft, brown faceted asteroids, projectiles, effects, and a HUD on a muted plum background. It is third-party reference artwork associated with [Kenney’s Space Shooter Remastered](https://kenney.nl/assets/space-shooter-remastered), whose pack page lists CC0. The owner-supplied archive was subsequently inspected: its `sample.png` matches this reference pixel-for-pixel, confirming the scene’s pack provenance. The supplied PNG is preserved unchanged and is not a Frostwind screenshot, a milestone capture, or project-authored MIT artwork. Exact runtime sprite files and their bundled license will be recorded separately in the future example’s `assets/LICENSES.md` when imported.

## Generated Asteroids concepts

The following concepts were generated on 2026-10-14 using the built-in OpenAI image generation tool during art exploration for [RFC 0005](../../../../docs/rfcs/0005-camera-keyboard-and-sprites.md). They are generated design references, not engine screenshots, milestone captures, runtime sprite sheets, or validated normal maps. The original Kenney pack reference above remains unchanged. No final production asset/license selection is implied by these exploratory images.

| File | Input and prompt direction | Review outcome |
| --- | --- | --- |
| [`asteroids-realistic-concept-v1.png`](asteroids-realistic-concept-v1.png) | Owner-supplied pack scene; preserve its top-down layout and HUD while replacing cartoon surfaces with detailed weathered metal, reflective cockpit glass, realistic rocky asteroids, and controlled cinematic lighting | Owner liked the detailed look, then asked whether it could work in 2D; retained as the material/detail target |
| [`asteroids-soft-concept-v2.png`](asteroids-soft-concept-v2.png) | Concept v1; preserve composition while using a direct overhead sprite-like view, matte surfaces, diffuse overhead illumination, reduced contrast, restrained glow, and a quieter background | Softer baked appearance did not meet the owner’s preference; preserved for comparison |

Both PNGs were copied byte-for-byte from the tool outputs and preserved alongside their input reference. The current proposal is to evaluate matched base-color and normal textures with runtime lighting; these generated scenes are references for that experiment, not its output.

## Asteroids sprite prototypes

[Generated ship and meteor prototypes](asteroids-sprites/README.md) preserve the first individual transparent color sprites and their submitted prompts. They use the detailed concept and original pack assets as references. These outputs still have baked lighting and are not validated normal-mapped materials.
