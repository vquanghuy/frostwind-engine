# Local dependencies

Frostwind stores third-party dependencies locally in this directory.

| Dependency | Version | Integration |
| --- | --- | --- |
| [SDL3](sdl3/README.integration.md) | 3.4.18 | Official macOS universal dynamic framework with packaged headers |
| [GLAD](glad/README.integration.md) | 2.0.8 | Generated OpenGL 3.3 Core C loader; local static target for native builds |
| [stb_image](stb/README.integration.md) | 2.28 | Pinned local header; PNG/JPEG implementation compiled once into the engine |

## Layout

Use one directory per library, with separate SDK folders for each platform and architecture. SDL3 uses this layout:

```text
external/sdl3/
├── LICENSE.txt
├── README.integration.md
├── macos-universal/SDL3.framework/  # Supplied, including its public headers
├── windows-x64/                    # Pending; MSVC x64
│   ├── include/SDL3/
│   ├── lib/SDL3.lib
│   └── bin/SDL3.dll
└── linux-x64/                      # Pending; distribution-specific SDK
    ├── include/SDL3/
    └── lib/libSDL3.so.0            # Include symlink targets when applicable
```

The Windows and Linux entries describe expected files, not imported packages. See [project structure](../docs/project-structure.md) for runtime layout and header lookup.

Only create directories that contain a dependency's actual files. Build outputs produced by this project belong in `build/`, not here.

## Integration

- CMake integrates local source with `add_subdirectory()` or project-owned targets. Supplied binaries use imported targets with matching include directories and link requirements.
- Prefer static libraries for compiled dependencies. Dynamic libraries are allowed when useful, with runtime bundling and loader paths handled explicitly.
- Keep headers matched to the exact source or binary version. Separate binary platforms, architectures, and debug/release configurations.
- Supply a separate compatible archive for each target. An archive ending in `.a` is not necessarily WASM-compatible.
- Use the official SDL3 macOS framework from the release DMG for native builds. Preserve the framework layout and symlinks. A separate WASM-compatible library will be required for browser compilation.
- GLM and stb use local headers. Compile stb implementations once in designated translation units.
- Build Dear ImGui core plus its SDL3/OpenGL3 backends from matching local source. Compile generated GLAD source for native builds only.
- Add optional native IXWebSocket only when needed; browser networking uses the Emscripten API.

## Inventory and updates

For every imported dependency, add `README.integration.md` recording its upstream URL, exact release and/or commit, license, imported files, local patches, and update procedure. For supplied binaries, also record compiler/SDK versions, architecture, build flags, configuration, linkage, and reproduction commands.

Commit the intended dependency files and their licenses. `.gitignore` allows supplied library files under each dependency's `lib/` directory and the named SDL3 Windows/Linux SDK locations while ignoring ordinary generated artifacts.

Update dependencies deliberately, then validate affected native and browser builds. See [RFC 0001](../docs/rfcs/0001-local-dependencies.md) for the decision.
