# Asteroids sprite prototypes

Generated on 2026-10-14 with the built-in OpenAI image generation tool for the owner’s request to try a player ship and meteor in the detailed Asteroids art style. These are exploratory color-and-alpha sprites, not validated base-color/normal texture pairs or implemented game screenshots. Files are preserved unchanged from the tool outputs. They are not copies of the original Kenney sprite files.

| File | Dimensions | Direction and inputs |
| --- | --- | --- |
| [player-ship-v1.png](player-ship-v1.png) | 1441 × 1091 | Upward-facing orange/off-white spacecraft; realistic concept v1 and Kenney `PNG/playerShip1_orange.png` used as visual references |
| [meteor-v1.png](meteor-v1.png) | 1438 × 1094 | Irregular rocky obstacle; realistic concept v1, Kenney `PNG/Meteors/meteorBrown_big2.png`, and generated ship v1 used as visual references |

Both outputs were visually inspected and their RGBA transparency was verified. The generated images retain baked shading/reflections despite neutral-lighting instructions. They require asset cleanup and an independently validated common-source color/normal workflow before being treated as relightable materials. Neither includes a height map or normal map. Silhouette/padding, small-size readability, texture alignment, and rotating-light behavior remain runtime validation work.

Prompt directions: isolated direct-overhead orthographic game sprites, genuine transparent backgrounds, realistic controlled surface detail, neutral diffuse illumination, no scene/HUD/effects, matched orange-white metal and warm gray-brown rock materials. The full submitted prompts are recorded in [prompts.md](prompts.md).

See [RFC 0005](../../../../../docs/rfcs/0005-camera-keyboard-and-sprites.md) for the planned game, renderer, and normal-map prototype. The prior [pack reference](../kenney-space-shooter-reference.png), [realistic scene concept](../asteroids-realistic-concept-v1.png), and [softer scene concept](../asteroids-soft-concept-v2.png) remain unchanged.
