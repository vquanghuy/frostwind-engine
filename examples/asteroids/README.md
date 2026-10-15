# Asteroids (planned)

A planned Asteroids-themed 2D technical example for Frostwind. A spacecraft and asteroid scene provides a practical setting for developing and validating reusable engine features.

**Status:** planning and art exploration only. This directory contains documentation and reference artwork; there is no Asteroids implementation, CMake target, executable, or imported runtime asset set yet. The working engine demo remains [Textured Quad](../textured-quad/README.md).

![Selected detailed Asteroids scene concept](docs/assets/asteroids-realistic-concept-v1.png)

*Selected art direction: detailed 2D sprites with runtime normal-map lighting. Generated concept, 2026-10-14; this is a visual reference, not an implemented game screenshot or validated lighting result.*

## Purpose and scope

This example exists to answer concrete engine questions: how to position and render objects consistently in world space, handle keyboard movement and mouse aiming, light detailed flat sprites, and scroll a background convincingly. The ship and rocks make those behaviors easy to inspect and compare.

A fully functional Asteroids game is **not an expected deliverable**. The example may stop at a small interactive scene once its technical goals are demonstrated. Bullets, collisions, asteroid splitting, waves, score, lives, menus, audio, and a complete game loop are optional follow-up work when they serve another specific engine experiment. Concept artwork illustrates a visual direction, not a feature checklist.

**Current evidence:** concept generation, source-archive inspection, and prototype image transparency checks have been completed. The engine challenges below remain planned; the artwork does not establish that camera, input, sprite rendering, parallax, or normal-map lighting work in Frostwind.

## Technical challenges to validate

| Challenge | Proposed approach | Evidence of success |
| --- | --- | --- |
| World coordinates, automatic camera following, and high-DPI resizing | GLM orthographic camera follows the ship; fixed rotation and zoom; floating-point positions and drawable-pixel viewport | Known points and sprite extents map correctly; ship following and resizing preserve proportions; Retina dimensions are handled explicitly |
| Reliable keyboard state | Held/pressed/released state from SDL callbacks; ignore repeat edges; clear held state on focus loss | Continuous input is independent of OS repeat; press/release edges last one update; focus changes never leave movement stuck |
| Accurate mouse aiming through camera transforms | Convert logical cursor coordinates to drawable pixels, then unproject through the current camera; point the ship’s +Y axis toward the world-space cursor | Aim remains correct during automatic camera following, resize, and Retina scaling; invalid or coincident cursor positions preserve orientation |
| Consistent motion at different FPS caps | Monotonic elapsed-time updates, heading-relative forward/backward/strafe vectors with normalized diagonal movement, and a deliberate stall/restore policy | Controlled 30/60/120 FPS schedules produce equivalent travel; stalls and restore do not cause a large jump |
| Reusable sprite rendering and resource lifetime | Shared quad geometry, model/view/projection transforms, tint, straight-alpha blending, and scoped GPU ownership | Orientation, sizing, overlap, and transparency match known inputs; failed startup and teardown keep GPU operations within context lifetime |
| Detailed lighting on flat sprites | Matching base-color/normal textures, explicit channel/color-space conventions, and world-space lighting | Sprite rotation and light movement produce consistent highlights/shading; ship rotation does not rotate the world light; baked shadows do not conflict with relighting |
| Convincing background motion | Example-owned tiled starfield with subtle parallax and stable world offsets | Scrolling is frame-rate-independent, negative positions and tile crossings do not pop, and following/resize leave no uncovered areas |
| Repeatable asset preparation | Preserve inputs, licenses, prompts/bake or height-map settings, dimensions, padding, and orientation | A ship/meteor material pair can be reproduced and checked; approximated normals are distinguishable from validated common-source output |

The lighting experiment compares a common-source model bake with an authored-height-map route; NormalMap-Online is a candidate converter for the latter. Direct color-to-normal conversion is only an approximation because paint and baked lighting do not uniquely describe surface height. The experiment may conclude that a workflow needs revision; documenting that result is useful evidence too.

## Completion criteria for the example

The initial target is a runnable scene with one controllable ship and asteroid landmarks, a working orthographic camera, dependable keyboard movement, mouse aiming, and delta-time behavior, transparent sprite rendering, and an automatically scrolling background. A focused ship/meteor lighting comparison should establish whether the proposed normal-map workflow is viable. A working unlit scene remains useful if lighting requires further work.

Record executed math/input/timing/rendering/lifecycle checks, actual lighting findings, asset provenance, and remaining manual checks. The example meets its purpose when those results make the reusable engine behavior reviewable; a win/lose condition, progression system, or complete playable game is not required.

## Concept progression

The following images show the art exploration in order. They are references and generated concepts, not screenshots of an implemented Frostwind game. Original files and earlier versions remain preserved.

### 1. Initial pack reference

![Initial Asteroids reference from the Kenney Space Shooter pack](docs/assets/kenney-space-shooter-reference.png)

*Owner-supplied Kenney pack scene: the starting point for composition, ship colors, asteroid placement, and readable gameplay silhouettes. This is third-party reference artwork; see [attribution](docs/assets/README.md).*

### 2. Detailed realistic concept

The detailed realistic scene at the top of this README is the second concept and the selected material reference.

*Generated concept v1, 2026-10-14: weathered metal hulls, dark cockpit glass, rocky craters, and stronger lighting. Its material richness became the preferred direction. Ships, weapons, shields, and HUD elements illustrate possible future behavior.*

### 3. Softer-light variant

![Third concept exploring softer lighting and a flatter sprite appearance](docs/assets/asteroids-soft-concept-v2.png)

*Generated concept v2, 2026-10-14: diffuse overhead illumination, reduced contrast, and simplified shading. Preserved as an alternative; its softer appearance was not selected.*

### Sprite prototypes

Retain the realistic concept’s detailed materials and clear silhouettes, using a strict overhead orthographic view and flat sprite geometry. The target is weathered off-white metal, burnt-orange player markings, dark cockpit glass, and warm gray-brown rocks. Matching base-color and normal textures will let surface illumination respond to world-space light as sprites rotate; the softer concept’s painted treatment is not the selected direction.

These individual generated prototypes illustrate the selected ship and meteor design language:

| Player ship prototype | Meteor prototype |
| --- | --- |
| ![Orange-and-white player ship art prototype](docs/assets/asteroids-sprites/player-ship-v1.png) | ![Detailed rocky meteor art prototype](docs/assets/asteroids-sprites/meteor-v1.png) |

*Generated transparent color prototypes, 2026-10-14. They still contain baked reflections/shading and have no matching normal maps. They demonstrate the intended art style, not finished relightable materials or a fourth completed game-scene render.*

Keep large shapes readable at gameplay size. A sparse dark plum/charcoal starfield scrolls with camera movement; subtle parallax adds depth while distant stars remain unlit. Production material choices depend on the normal-map prototype proposed in [RFC 0005](../../docs/rfcs/0005-camera-keyboard-and-sprites.md).

## First implementation

[RFC 0005](../../docs/rfcs/0005-camera-keyboard-and-sprites.md) proposes the foundation sprint:

- A GLM-backed orthographic 2D camera that follows the ship automatically at fixed rotation 0 and zoom 1. One world unit maps to one drawable pixel. Interactive camera movement, rotation, and zoom are deferred to another example.
- Keyboard held/pressed/released state and elapsed-time updates for frame-rate-independent movement. Mouse position is converted to world coordinates to set ship heading; WASD movement uses that heading.
- Reusable sprite rendering with position, size, rotation, tint, transparency, and submission order.
- A separate Asteroids scene with a movable ship and stationary rock landmarks, plus an automatically scrolling/parallax background.
- A small lighting prototype using one ship, one asteroid, and controlled light directions in validation before expanding custom asset production.

Optional follow-up experiments may use thrust/inertia, moving asteroids, bullets, collisions, splitting, or screen wrapping to exercise additional engine behavior. Score, lives, and restart rules are optional presentation/gameplay work, not completion requirements. If collision work is introduced, simple example-owned circle checks may suffice before considering a physics library.

## Engine and game responsibilities

The engine owns reusable camera, input, timing, texture, sprite, and any validated lighting facilities. This example owns its controls, camera behavior, scene layout, and assets. Any future arena/wrapping rules, asteroid behavior, bullets, collision responses, scoring, waves, or match state also belong here rather than in the engine.

Runtime geometry and simulation should remain independent of a graphics context where practical. Introduce fixed-step scheduling when gameplay simulation needs it; the proposed variable-delta camera update is not a physics timestep or a determinism guarantee.

## Proposed controls

These are the only planned controls for Asteroids; implementation remains pending.

| Controls | Action |
| --- | --- |
| Mouse position | Set ship heading by aiming its nose toward the cursor |
| WASD | W moves forward along the heading; S moves backward; A strafes left; D strafes right |
| Escape | Quit |

Movement is relative to the current mouse-controlled heading. Normalize combined input so diagonal movement is no faster than movement along one axis; opposite keys cancel. Apply elapsed time to translation. Keyboard input does not independently rotate the ship, and mouse aiming does not apply thrust or movement. This first example uses direct translation, not an inertia simulation.

The camera follows the ship automatically with fixed rotation and zoom, allowing the starfield to scroll without additional bindings. Camera manipulation belongs in a separate example. Reset, background toggles, light adjustment, shooting, mouse buttons, and wheel controls are outside this example’s control scheme; diagnostic comparisons use controlled validation fixtures or configuration.

## Asset sources and provenance

- [Sprite generation prompts and provenance](docs/assets/asteroids-sprites/README.md).
- [Full concept/reference attribution](docs/assets/README.md), including third-party licensing and generated-image provenance.

The owner-supplied Space Shooter Remastered archive has been inspected; RFC 0005 records its checksum, license findings, and candidate ship/meteor filenames. Runtime imports remain pending. NormalMap-Online is a candidate offline height-to-normal tool; the RFC records the proposed workflow and validation requirements.

## Build and assets

There is no Asteroids build/run command yet. See [development instructions](../../docs/development.md) for building and running the implemented Textured Quad demo. Add a separate target and verified commands when Asteroids code is introduced.

`docs/assets/` holds planning artwork only and is not packaged with an executable. Future runtime textures and shaders belong in this example’s `assets/`, with their sources/licenses in `assets/LICENSES.md` when imported. Concept images do not establish production asset readiness or licensing for an unspecified future asset set.
