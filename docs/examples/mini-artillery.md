# Mini Artillery example (planned)

Mini Artillery is a proposed Frostwind example inspired by Gunbound and Pocket Tanks. It will exercise 2D destructible terrain, projectile impacts, and bodies settling on modified ground. No gameplay or `examples/mini-artillery/` executable exists yet. The earlier Mini Artillery window/checkerboard scaffold is now the [textured-quad example](../../examples/textured-quad/README.md).

## Engine and game responsibilities

The example will own tanks, aiming, weapons, damage, turns, wind rules, match state, UI, and game-specific assets. The engine can supply terrain geometry, GPU masks, chunk ownership, carving, and physics integration. For an explosion, the example selects impact position, radius, and damage rules; the engine applies carving and rebuilds affected collision chunks.

Terrain experiments will coordinate visible masks and collision geometry, using explicit render-pixel/physics/integer-coordinate conversions. Physics will use fixed timesteps. Multiplayer and hosting are optional later experiments, requiring explicit authority, event ordering, and state recovery; deterministic behavior must not be assumed.

## Visual references

![Kenney Tanks mockup](../assets/kenney-tanks-mockup.png)

*Kenney Tanks asset-pack mockup used as a reference, not a Frostwind or Mini Artillery screenshot.*

The intended tank art uses separate chassis and turret sprites, with chassis following terrain slopes and independent turret aiming.

![Gunbound gameplay reference](../assets/gunbound-reference.png)

*Gunbound reference for turn-based aiming, wind, and destructible terrain.*

![Pocket Tanks gameplay reference](../assets/pocket-tanks-reference.png)

*Pocket Tanks reference for craters and shot controls.*

![Kenney Tanks asset preview](../assets/kenney-tanks-preview.png)

Planned sprite source: [Kenney Tanks](https://kenney.nl/assets/tanks), released under CC0. See [reference attribution](../assets/README.md); third-party game images are not covered by the project's MIT license.
