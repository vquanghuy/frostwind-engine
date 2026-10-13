# Working on Mini Artillery

## Project context

Mini Artillery is a personal C++ experiment in 2D destructible terrain, inspired by Gunbound and Pocket Tanks. Prioritize terrain rendering, carving, collision updates, and a simple native macOS Apple Silicon sandbox. Artillery mechanics support the experiments. WebAssembly is a later portability experiment; multiplayer and deployment are optional extensions. Keep the scope lightweight and add modules only as needed for the experiment.

The repository contains a native SDL3/OpenGL scaffold with a temporary checkerboard quad, the official SDL3 macOS dynamic framework, generated GLAD files, documentation, and reference images. Terrain and gameplay are not implemented. Keep descriptions of planned behavior distinct from implemented behavior. Read `README.md`, `docs/coding-conventions.md`, and relevant documents under `docs/` before making changes.

## Technology decisions

- Use CMake for native and browser builds; keep shared build configuration in one project.
- Use SDL3 3.4.18 from `external/sdl3/` and GLAD 2.0.8 from `external/glad/` for the native OpenGL Core scaffold. Browser rendering through Emscripten's GLES3 interface to WebGL 2 remains planned.
- Use Box2D 3.x for physics and Clipper2 for terrain polygon operations. Box2D 3 uses a C API callable from C++; do not mix it with Box2D 2.x examples or APIs.
- Use GLM for graphics math, Dear ImGui for UI and debug tools, and stb_image for image loading.
- Use pinned local dependency copies under `external/<library>/`; follow `external/README.md` and RFC 0001. Do not introduce FetchContent downloads or a package manager into the build unless the dependency policy changes.
- Prefer static linking for compiled dependencies. SDL3 is an explicit exception: use the official macOS universal dynamic framework, including its packaged headers, and copy it beside the executable at build time. Do not substitute a locally built binary or restore the full SDL3 source tree. Native macOS binaries cannot be linked into WebAssembly.
- Preserve dependency licenses and record exact versions, origins, local patches, and the toolchain/configuration of supplied binaries. Keep generated outputs under `build/`; SDL3 SDKs belong under `external/sdl3/<platform>-<architecture>/`; follow `docs/project-structure.md` for each SDK layout.
- Record significant architecture decisions as RFCs under `docs/rfcs/` using the supplied template and process.

## Code organization

Follow `docs/coding-conventions.md` and the responsibilities in `docs/project-structure.md`, with the planned folder layout summarized in `README.md`. Add directories when implementation needs them rather than adding empty placeholders.

- `src/app/`: startup, frame loop, and fixed-step scheduling.
- `src/game/`: tanks, weapons, damage, match rules, and turn state.
- `src/terrain/`: terrain polygons, chunk ownership, and explosion carving.
- `src/physics/`: Box2D integration and collision-shape rebuilding.
- `src/rendering/`: terrain masks, shaders, textures, and sprite batching.
- `src/ui/`: game screens and debug tools.
- `src/networking/`: client transport, protocol, and synchronization.
- `src/platform/`: native/browser-specific integration.
- `assets/`: runtime assets; track their sources and licenses in `assets/LICENSES.md` when introduced.
- `docs/assets/`: documentation images, with attribution in its README.
- `server/`: the separate multiplayer relay application and its own dependencies.

Keep headers beside their `.cpp` files. Keep gameplay and terrain geometry independent of a graphics context so they can be tested without opening a window. Keep platform conditionals concentrated in platform and build integration code where practical.

## Terrain and simulation rules

- Treat terrain geometry and its GPU mask as coordinated representations. Feed both the same explosion coordinates and radius.
- Rebuild collision shapes only for affected terrain chunks. Account for holes, contour winding, small edges, and chunk boundaries.
- Use explicit conversions between render pixels, physics units, and Clipper2 integer coordinates.
- Run gameplay physics at a fixed timestep, independently of render frequency. Use the browser-compatible frame callback for web builds.
- Do not assume a fixed timestep or integer clipping makes the whole game deterministic. Preserve consistent simulation inputs and object creation order, and validate native versus WASM behavior before relying on deterministic multiplayer.
- Keep authoritative impact resolution and state recovery explicit in the multiplayer design.

## Build and validation

Use `cmake --preset native-debug` and `cmake --build --preset native-debug --parallel` for the native window scaffold. Run `./build/native/mini-artillery`. See `docs/development.md` for prerequisites and manual checks. Windows x64 and Linux x86_64 CMake branches and presets are prepared but unverified, and their SDKs are not yet included. Do not describe them as validated support. Browser builds and automated tests have not been established. Do not report a build or test as passing unless the command has actually run.

When scaffolding the project, use separate generated output directories: `build/native/` and `build/web/`. Configure web builds with Emscripten's toolchain through `emcmake`. Document working commands and prerequisites in `docs/development.md` once verified.

Run checks appropriate to the change:

- Documentation: check local links, image paths, and formatting.
- Gameplay and terrain: test meaningful behavior such as crater geometry, chunk boundaries, collision rebuilding, and turn transitions.
- Rendering and platform integration: build affected targets and visually inspect the behavior when possible.
- Multiplayer: validate event ordering, invalid actions, and state consistency with multiple clients.

Report what was checked and any checks that could not run. Update documentation when implementation changes the architecture or setup.

## Repository hygiene

Maintain `CHANGELOG.md` using Keep a Changelog 1.1.0: collect notable changes under `Unreleased`, use only relevant standard categories, and add dated version sections when a release is explicitly made. Do not invent release tags or treat every commit as a milestone. The project owner captures screenshots; do not capture automatically. When asked to retrieve a supplied milestone screenshot, preserve the PNG under `docs/assets/milestones/` and embed it directly in `CHANGELOG.md` alongside the relevant changes, with a brief date and platform caption. Keep progress tracking in the changelog; do not add a separate milestone document. If a capture is pending, record that limitation rather than fabricating an image.

Do not commit generated builds, local environment files, editor state, or `.serena/`. Preserve unrelated changes. Keep project code licensing separate from third-party asset licensing, and label reference images as references rather than game screenshots.
