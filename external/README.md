# Local dependencies

Mini Artillery stores third-party dependencies locally in this directory. No libraries have been imported yet.

## Layout

Use one directory per library. Preserve upstream source layouts when practical; the following shows the conventions for supplied headers and binaries:

```text
external/
├── README.md
└── <library>/
    ├── README.mini-artillery.md  # Version, origin, patches, build details
    ├── LICENSE                  # Upstream license, preserving its filename
    ├── include/                 # Public headers, if provided separately
    ├── src/                     # Source, or the original upstream source tree
    └── lib/                     # Supplied binaries only, when needed
        ├── macos-arm64/
        │   ├── debug/
        │   └── release/
        └── wasm32/
            ├── debug/
            └── release/
```

Only create directories that contain a dependency's actual files. Build outputs produced by this project belong in `build/`, not here.

## Integration

- CMake integrates local source with `add_subdirectory()` or project-owned targets. Supplied binaries use imported targets with matching include directories and link requirements.
- Prefer static libraries for compiled dependencies. Dynamic libraries are allowed when useful, with runtime bundling and loader paths handled explicitly.
- Keep headers matched to the exact source or binary version. Separate binary platforms, architectures, and debug/release configurations.
- Retain source for dependencies needed on both native and WASM targets. An archive ending in `.a` is not necessarily WASM-compatible.
- Browser SDL2 is supplied by Emscripten's SDL2 port; native SDL2 is local. The SDK may need network access to populate its port cache on first use.
- GLM and stb use local headers. Compile stb implementations once in designated translation units.
- Build Dear ImGui core plus its SDL2/OpenGL3 backends from matching local source. Compile generated GLAD source for native builds only.
- Add optional native IXWebSocket only when needed; browser networking uses the Emscripten API.

## Inventory and updates

For every imported dependency, add `README.mini-artillery.md` recording its upstream URL, exact release and/or commit, license, imported files, local patches, and update procedure. For supplied binaries, also record compiler/SDK versions, architecture, build flags, configuration, linkage, and reproduction commands.

Commit the intended dependency files and their licenses. `.gitignore` allows supplied library files under each dependency's `lib/` directory while ignoring ordinary generated artifacts.

Update dependencies deliberately, then validate affected native and browser builds. See [RFC 0001](../docs/rfcs/0001-local-dependencies.md) for the decision.
