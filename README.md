# Mini Artillery

A C++ experiment exploring **2D destructible terrain**, inspired by **Gunbound** and **Pocket Tanks**. The focus is terrain rendering, carving, and collision updates as explosions reshape the terrain.

An artillery-style sandbox provides a setting for testing terrain destruction, projectile impacts, and objects settling on the modified ground. Development starts on native macOS, with WebAssembly as a later portability experiment.

> **Status:** Planning an experimental prototype. This repository currently contains documentation and visual references; there is no runnable prototype yet. Scope and implementation may change as the experiments develop.

## Visual references

![Kenney Tanks mockup showing side-view tanks, projectiles, and sloped terrain](docs/assets/kenney-tanks-mockup.png)

*Kenney Tanks asset-pack mockup, used as a visual reference. This is not a Mini Artillery screenshot.*

The planned art uses separate tank chassis and turret sprites, with the chassis following terrain slopes and the turret rotating independently for aiming.

<details>
<summary>Gameplay inspiration and asset preview</summary>

### Gunbound

![Gunbound gameplay reference showing vehicles on destructible terrain](docs/assets/gunbound-reference.png)

Reference for turn-based aiming, wind, and terrain destruction.

### Pocket Tanks

![Pocket Tanks gameplay reference showing craters and angle/power controls](docs/assets/pocket-tanks-reference.png)

Reference for artillery combat, shot controls, and crater formation.

### Kenney Tanks

![Kenney Tanks preview showing chassis, turrets, projectiles, and effects](docs/assets/kenney-tanks-preview.png)

Planned sprite source: [Kenney Tanks](https://kenney.nl/assets/tanks), released under CC0.

</details>

## Experimental scope

- GPU texture masking for visible terrain destruction.
- Polygon clipping and chunked collision-shape rebuilding after explosions.
- Keeping rendered terrain and physical collision boundaries aligned.
- Repeated craters, holes, thin terrain, and chunk-boundary behavior.
- Simple projectiles and tanks or debris interacting with modified terrain.
- Native versus browser behavior and performance, once the core experiment works.

## Technical approach

| Area | Choice |
| --- | --- |
| Language and build | C++ and CMake |
| Native development | macOS on Apple Silicon, SDL2, desktop OpenGL, GLAD |
| Browser target | Emscripten, WebAssembly, WebGL 2 / OpenGL ES 3.0 |
| Physics | Box2D |
| Terrain geometry | Clipper2 |
| Math | GLM |
| UI and debugging | Dear ImGui |
| Sprites | Kenney Tanks, texture atlases, dynamic sprite batching |
| Image loading and packing | stb_image and stb_rect_pack |
| Optional networking experiment | WebSockets with a room-based relay server |

### Destructible terrain

Terrain has two coordinated representations:

- **Visual terrain:** A color texture and a solidity mask. Explosions erase circles from the mask by rendering into a framebuffer object; a shader hides the erased regions.
- **Collision terrain:** Chunked polygons. Clipper2 subtracts the explosion shape from affected chunks, then their Box2D collision shapes are rebuilt.

Both representations receive the same explosion coordinates and radius.

### Optional multiplayer exploration

One possible extension is exchanging shot and explosion events over WebSockets to explore terrain synchronization without streaming textures. Multiplayer and hosting are optional follow-up experiments rather than completion requirements.

Physics runs at a fixed timestep. Cross-platform consistency, authoritative impact resolution, and recovery from state differences still need to be specified and validated before multiplayer implementation.

## Experiment sequence

1. **Terrain sandbox:** Set up CMake and the macOS toolchain; render terrain and erase explosion shapes through a debug UI.
2. **Collision updates:** Add Clipper2 and Box2D, rebuild affected chunks, and inspect how simple bodies interact with carved terrain.
3. **Artillery interactions:** Add basic aiming and projectiles to exercise the destruction pipeline. Expand tank behavior or effects only when useful to the experiment.
4. **Browser portability:** Try the same prototype through Emscripten and WebGL 2, comparing behavior and performance.
5. **Optional extensions:** Explore multiplayer synchronization or host a browser demo if useful.

## Project structure

The following layout is a guide for organizing the experiments. Add modules as they become useful; gameplay, networking, and server folders are optional. The tree describes the intended structure, not an existing implementation.

```text
mini-artillery/
├── README.md
├── LICENSE
├── .gitignore
├── CMakeLists.txt              # Shared native and WebAssembly build
├── CMakePresets.json           # Reproducible build configurations
├── cmake/                     # Dependency setup and build helpers
├── src/
│   ├── main.cpp               # Entry point
│   ├── app/                   # Initialization, frame loop, fixed-step updates
│   ├── game/                  # Simple tanks and projectile experiments
│   ├── terrain/               # Terrain polygons, chunks, explosion carving
│   ├── physics/               # Box2D world and collision-shape rebuilding
│   ├── rendering/             # Terrain masks, textures, sprite batching
│   ├── ui/                    # Dear ImGui experiment controls and debug tools
│   ├── networking/            # Optional synchronization experiments
│   └── platform/              # Native and browser integration
├── assets/                    # Runtime game assets
│   ├── sprites/
│   ├── shaders/
│   ├── audio/
│   └── LICENSES.md            # Runtime asset attribution
├── external/                  # Local third-party headers, source, and libraries
│   └── README.md              # Dependency inventory and integration policy
├── tests/                     # Terrain, gameplay, and protocol verification
├── tools/                     # Asset preparation and development scripts
├── web/                       # Browser HTML shell and web resources
├── server/                    # Optional relay for networking experiments
└── docs/
    ├── README.md              # Documentation index
    ├── architecture.md        # Implemented architecture, added with code
    ├── development.md         # Verified setup and debugging instructions
    ├── roadmap.md             # Detailed milestones, when needed
    ├── rfcs/                  # Numbered proposals and decision records
    │   ├── README.md          # RFC process and index
    │   └── template.md
    └── assets/                # Documentation images and references
        └── README.md          # Reference image attribution
```

Headers live beside their `.cpp` files. Generated outputs go into ignored `build/native/` and `build/web/` directories. Third-party dependencies use pinned local copies under `external/`; see the [dependency policy](external/README.md).

### Component responsibilities

`app` coordinates input, fixed-step simulation, and rendering. Explosions can come from debug controls or simple projectile interactions in `game`. `terrain` applies them to polygon geometry; `physics` rebuilds the affected collision shapes, and `rendering` updates the visual mask from the same explosion descriptor.

Terrain geometry and simulation logic should work without a graphics context so they can be tested independently. `platform` contains native/browser differences. If networking is explored, `networking` handles client messages and `server` contains a separate relay application.

## Development

CMake is the selected build system for both native and browser targets. Native builds will use Apple's compiler toolchain; browser builds will use Emscripten's CMake toolchain through `emcmake`.

Build configuration, dependency versions, and verified setup commands will be added with the native project scaffold.

### Local dependencies

Third-party headers, source, and any supplied static or dynamic libraries live under `external/<library>/`. CMake links local targets without fetching dependencies or using a package manager during configuration.

Static linking is the default for compiled dependencies. Header-only libraries are included directly, and libraries such as Dear ImGui and generated GLAD are compiled from local source. Supplied binaries are separated by platform and architecture; macOS libraries cannot be reused in WASM builds, which require Emscripten-compatible source or archives. Browser SDL2 uses the Emscripten port, whose first build may populate the SDK cache.

Each dependency must include its license, exact version or source revision, origin, and any local changes. Dynamic macOS libraries require runtime bundling and loader-path configuration when packaging the game. See [RFC 0001](docs/rfcs/0001-local-dependencies.md).

## Documentation

Project documentation lives in [`docs/`](docs/README.md). Architecture proposals and decisions are recorded as numbered [RFCs](docs/rfcs/README.md). Reference images and their attribution are stored in [`docs/assets/`](docs/assets/README.md).

## License

Project code is licensed under the [MIT License](LICENSE).

Kenney Tanks assets are CC0. Gunbound and Pocket Tanks screenshots are third-party gameplay references and are not covered by this repository's MIT license. See [reference image attribution](docs/assets/README.md).
