# Asteroids example (planned)

A planned Asteroids-themed 2D technical example for Frostwind. A spacecraft and asteroid scene provides a practical setting for developing and validating reusable engine features.

**Status:** planning and art exploration only. This directory contains documentation and reference artwork; there is no Asteroids implementation, CMake target, executable, or imported runtime asset set yet. The working engine demo remains [Textured Quad](../textured-quad/README.md).

## Concept progression and art lineage

Tracing the visual evolution from initial third-party reference to the selected 2D normal-mapped art direction:

| 1. Pack reference (Origin) | 2. Realistic concept (Selected target) | 3. Softer alternative (Explored) |
| :---: | :---: | :---: |
| ![Kenney pack scene](docs/assets/kenney-space-shooter-reference.png) | ![Realistic concept v1](docs/assets/asteroids-realistic-concept-v1.png) | ![Soft concept v2](docs/assets/asteroids-soft-concept-v2.png) |
| [Kenney Space Shooter](https://kenney.nl/assets/space-shooter-remastered) (CC0): starting point for composition, colors, and readable silhouettes. | **Selected direction:** weathered metal, dark glass, and rocky asteroids designed for dynamic normal-map lighting. | Explored diffuse overhead light and simplified shading; preserved for comparison. |

### Sprite prototypes

| Player ship prototype | Meteor prototype |
| :---: | :---: |
| ![Orange-and-white player ship prototype](docs/assets/asteroids-sprites/player-ship-v1.png) | ![Detailed rocky meteor prototype](docs/assets/asteroids-sprites/meteor-v1.png) |
| Transparent color prototype; baked shading to be replaced by matching base-color and normal maps. | Chipped rock plates and craters; candidate for height-map and bake experiments. |

*Generated prototypes (2026-10-14). See [sprite prompts and provenance](docs/assets/asteroids-sprites/README.md) and [full reference attribution](docs/assets/README.md).*

## Technical purpose and scope

This example exists to validate concrete engine systems rather than deliver a complete game loop:
- **World coordinates & camera following:** An orthographic `Camera2D` following the ship at fixed rotation 0 and zoom 1, mapping 1 world unit to 1 drawable pixel on standard and Retina viewports.
- **Input & mouse aiming:** SDL keyboard state tracking and cursor unprojection to orient the ship (+Y) toward the cursor in world space.
- **Frame timing:** Monotonic elapsed-time updates producing consistent motion across 30, 60, and 120 FPS caps.
- **Sprite rendering:** Reusable immediate `SpriteRenderer` with shared quad geometry, straight-alpha blending, and scoped GPU ownership.
- **Parallax background:** An example-owned tiled starfield that scrolls convincingly with camera motion.
- **Lighting prototype:** A focused normal-mapped lighting comparison (1 ship, 1 asteroid) evaluating dynamic relighting against baked shading.

Gameplay features like bullets, collision responses, asteroid splitting, waves, scoring, and audio are non-goals for this initial phase. See [RFC 0005](../../docs/rfcs/0005-camera-keyboard-and-sprites.md) for the detailed design, math formulas, and technical verification matrix.

## Engine and game responsibilities

- **Engine owns:** Reusable `Camera2D`, `Keyboard`, `Mouse`, monotonic timing, `SpriteRenderer`, and shader/material lighting behavior.
- **Example owns:** Ship controls/heading, camera follow logic, starfield parallax pass, scene layout, and assets.

Runtime geometry and simulation remain independent of a graphics context. The variable-delta update is designed for smooth rendering and camera motion, not as a fixed physics timestep.

## Proposed controls

These are the only planned controls for Asteroids; implementation remains pending.

| Controls | Action |
| --- | --- |
| Mouse position | Set ship heading by aiming its nose (+Y) toward the cursor |
| WASD | W forward, S backward, A strafe left, D strafe right (relative to heading) |
| Escape | Quit |

Movement uses direct translation scaled by delta seconds (no inertia simulation). The camera follows the ship automatically. Light inspection and diagnostic comparisons use test fixtures or configuration rather than runtime hotkeys.

## Build and assets

There is no Asteroids build/run command yet. See [development instructions](../../docs/development.md) for building and running the implemented Textured Quad demo.

`docs/assets/` holds planning artwork only and is not packaged with an executable. Future runtime textures and shaders belong in this example’s `assets/`, with their sources/licenses in `assets/LICENSES.md` when imported.
