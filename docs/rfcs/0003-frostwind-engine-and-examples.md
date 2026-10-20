# RFC 0003: Frostwind engine and examples

- **Status:** Implemented
- **Created:** 2026-10-14
- **Supersedes:** None
- **Superseded by:** None

## Problem

Mini Artillery currently combines reusable window/rendering facilities with a checkerboard demo in one executable. Planned camera, input, audio, and terrain systems would further mix engine development with application behavior. We want reusable engine features and game examples to evolve together with one repository and development history.

The implemented code is a native OpenGL scaffold with shader and texture support. Artillery gameplay, terrain destruction, camera, and audio are not implemented.

## Goals and scope

Make **Frostwind** the main project, with a lightweight C++17 engine library for 2D and 3D games and examples that exercise it. Mini Artillery becomes the initial main game example, driving reusable 2D and destructible-terrain features. Planned capabilities include both 2D and 3D physics integrations and multiple graphics APIs. Keep the existing technology choices and local dependency policy for the initial implementation. Add systems only when an example needs them.

OpenGL and the 2D examples are the starting point, not limits on the engine's scope. Additional graphics APIs and 3D systems are future work; this migration does not introduce backend abstractions or select a 3D physics library in advance.

This decision covers repository identity, code ownership, namespace, build targets, documentation, and migration. It does not implement gameplay, add a general-purpose editor, establish new platform support, or require a separate repository/submodule.

## Proposed design

### Repository and layout

Keep one repository and rename it on GitHub to `frostwind-engine` after the local migration is verified. Use **Frostwind** as the project name and `frostwind` as the engine CMake target.

```text
frostwind-engine/
├── CMakeLists.txt               # Shared project and dependency setup
├── CMakePresets.json
├── cmake/
├── engine/
│   ├── CMakeLists.txt           # Static frostwind library
│   └── src/
│       ├── app/                # Reusable lifecycle and frame coordination
│       ├── platform/           # Window, context, native/browser integration
│       └── rendering/          # ShaderProgram, Texture2D, future rendering tools
├── examples/
│   ├── 01-textured-quad/        # Existing demo and its shader assets
│   └── 04-mini-artillery/       # Game example, added when implementation starts
├── external/                   # Shared pinned dependencies and licenses
├── docs/
├── CHANGELOG.md
└── build/                      # Ignored generated outputs
```

Headers stay beside their `.cpp` files. Each example owns its source, CMake target, runtime assets, and asset license record. Add input, audio, terrain, physics, other examples, and tests as implementation requires them; do not create empty placeholders. The first migration creates the engine library and textured-quad example, not an empty game executable.

### Engine and example boundary

Dependencies flow from examples to the engine. The engine must not include example code, select example assets, or know artillery rules.

- **Engine:** Window/context, rendering resources, and future camera, input, audio, timing, physics integration, terrain carving, chunking, and collision rebuilding.
- **Mini Artillery:** Tanks, weapons, aiming, wind rules, damage, turns, match state, game UI, and game-specific assets.
- **Textured Quad:** Quad geometry, checkerboard pixel generation, demo shaders, and demo frame behavior.

For an explosion, the game decides its position, radius, and damage rules; the engine performs requested terrain carving and collision updates. Terrain geometry and simulation remain testable without a graphics context.

Keep 2D and 3D physics integrations distinct where their capabilities differ; do not force them behind one interface before concrete usage exists. Box2D 3.x remains the planned 2D choice, while a 3D library will be evaluated separately. Additional graphics APIs will receive their own design decisions when implementation begins; reusable game logic should not depend on a particular graphics API.

Refactor `Application` so the engine coordinates lifecycle and frames while a small example-provided interface supplies initialization, events, and frame behavior. Preserve SDL's callback-driven loop. Keep resource ownership explicit: example GPU resources must be released before the window/context. The engine must not own a hardcoded `QuadRenderer` or resolve its shader directory. Keep asset selection in the example and asset copying in its build target.

### Namespace, build, and assets

Put project-owned engine types and functions in namespace `frostwind`. Keep example-specific behavior outside that namespace. Required SDL C callback symbols remain at global scope and delegate to engine code. Update the coding convention's current no-project-namespace rule during migration.

Build the engine as a static library and link each example to it through CMake. Keep one root project, explicit source lists, local dependency targets, and existing native/web build-directory separation. Supply runtime frameworks/libraries and example assets beside each executable, using distinct example output directories to avoid asset collisions. Update presets and verified commands for the new executable targets.

Preserve [RFC 0001](0001-local-dependencies.md)'s dependency policy and [RFC 0002](0002-graphics-integration.md)'s graphics/resource decisions. SDL3 remains the official dynamic macOS framework; GLAD remains native-only. Windows/Linux integration is unverified and browser compilation remains pending.

### Documentation and history

Rewrite the README around the engine, its actual capabilities, and examples. Maintain the implemented layout and ownership in `docs/project-structure.md`; update development instructions, coding conventions, and `AGENTS.md` alongside the code.

Keep one changelog using standard categories, identifying entries as engine features or named example changes where useful. Preserve releases 0.1.0/0.2.0, existing tags, milestone PNGs, captions, and historical Mini Artillery identity. Keep artillery visual references associated with that example. Continue using manually supplied screenshots when requested.

After renaming GitHub, update the local remote and active repository links. The local checkout directory can retain its current name. Do not reset versions or create a release as part of restructuring; collect the migration under Unreleased.

## Alternatives

- **Keep the current application layout:** Minimal migration, but engine features and demo/game behavior remain coupled.
- **Separate engine repository with a submodule:** Independent releases and history, but adds coordination and pinned-reference updates while the engine and first game evolve together.
- **One engine repository with examples:** Keeps coordinated changes in one commit and makes reusable capabilities the main output. Selected for the current workflow.

## Risks and limitations

Extraction can create abstractions before they are useful. Keep the application interface small and let examples drive new capabilities. Moving SDL callbacks and resource owners can break startup or teardown; moving assets can break IDE launches. A working textured quad does not demonstrate artillery gameplay or browser portability.

## Validation

Configure and build the engine and textured-quad executable with the native preset. Verify startup, a rendered frame, cleanup, initialization failures, shader/texture behavior, and asset loading from a working directory outside the repository. Check that engine sources do not depend on examples and that example graphics resources outlive no context. Validate documentation links and preserved screenshots. Report remaining manual/platform checks; do not capture screenshots automatically.

## Implementation plan

1. Record this direction before changing code or repository identity.
2. Extract reusable code into the `frostwind` library, apply the namespace, and move the existing demo/assets into `examples/01-textured-quad/`.
3. Remove the demo dependency from engine application coordination and update CMake targets, asset copying, and presets.
4. Revise project documentation and guidance, then validate the native example and resource lifetimes.
5. Rename GitHub to `frostwind-engine`, update the remote and repository links, and preserve existing history.
6. Develop Mini Artillery under `examples/04-mini-artillery/`, introducing engine features as that game or focused examples need them.

## Open questions

Select the 3D physics library, additional graphics APIs, and any backend interfaces when those features are pursued. Shared installation/export packaging can wait until an external consumer needs it.

## Decision

Accepted and implemented on 2026-10-14 following the project owner's selection of an engine-focused repository with independent examples. Frostwind uses namespace `frostwind` and a static `frostwind` library. The `frostwind-textured-quad` example owns its demo and runtime assets under `examples/01-textured-quad/`, with native output under `build/native/bin/frostwind-textured-quad/`.

The example derives from `frostwind::Application`, supplying initialization, optional event handling, and rendering. A factory returns an owning base pointer; a virtual destructor releases derived GPU members before the base window. Each example supplies its own SDL C adapters. The shared CMake helper is `cmake/FrostwindExample.cmake`; `FROSTWIND_BUILD_EXAMPLES=OFF` builds the engine alone.

The repository is `frostwind-engine`, with remote `git@github.com:vquanghuy/frostwind-engine.git`. The local checkout directory may retain its existing name. The rename preserved project version 0.2.0, prior releases/tags, logos, and milestone captures. Subsequent numbered progress milestones are recorded in the changelog.

Native example and fresh engine-only builds passed. Checks linked against the engine and launched from `/private/tmp` verified example initialization/rendering/assets, shader/texture failures and GPU readback, cached texture limits, callback results and exceptions, failed startup, and derived-resource cleanup before context destruction. The Frostwind executable launched successfully, and the owner supplied the M004 screenshot; M001–M003 remain preserved. The remote was verified over SSH. Interactive resize/Retina and IDE checks remain manual; other platforms remain unverified. Mini Artillery gameplay and future engine systems remain follow-up work.
