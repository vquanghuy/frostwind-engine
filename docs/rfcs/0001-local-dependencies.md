# RFC 0001: Local third-party dependencies

- **Status:** Accepted
- **Created:** 2026-10-12
- **Author:** Mini Artillery maintainers
- **Supersedes:** None
- **Superseded by:** None

## Problem

The native macOS and planned WASM builds need a consistent way to integrate third-party libraries, including custom ImGui backends and platform-specific graphics loading.

## Goals and scope

Keep dependency files inspectable within the repository and use CMake to integrate local copies. Preserve version information and the ability to build both targets. This decision covers game-client dependencies; a future relay server may use the package manager appropriate to its language.

## Proposed design

Store pinned headers, source, and any supplied binaries in `external/<library>/`. Prefer static linking, compile header/source-only dependencies as appropriate, and allow dynamic native libraries with explicit packaging requirements. Preserve upstream licenses and record provenance, patches, and binary reproduction details.

Separate supplied binaries by platform, architecture, and configuration. Following the SDL3 packaging revision on 2026-10-13, use the official macOS universal dynamic framework, including its packaged headers, without the full source tree. Copy the framework beside the executable and use a relative runtime loader path. Supply a separate compatible library for each additional target. Generated project outputs remain in `build/`.

The detailed conventions are maintained in `external/README.md`.

## Alternatives

- **vcpkg:** Provides package recipes, version baselines, and binary caching, but adds another tool and may require custom packaging for our backend selection.
- **CMake FetchContent:** Automates source acquisition with pinned revisions, but requires dependency retrieval and integration during configuration.
- **Local copies:** Requires manual updates and a larger repository, but matches the selected workflow and gives direct control over imported files and compilation.

## Risks and limitations

Maintainers own updates, patch tracking, and binary compatibility. Native binaries do not work in WASM. Dynamic libraries add runtime packaging work. Local copies do not eliminate toolchain installation.

## Validation

When dependencies are introduced, verify a clean native build, matching headers and binaries, license/provenance records, and dynamic loading if used. Before accepting browser compatibility, build with Emscripten and run the result in a browser.

## Implementation plan

1. Establish the dependency policy and directory conventions.
2. Import exact dependency revisions as the native scaffold needs them.
3. Add local CMake targets and document verified build commands.
4. Validate the WASM equivalents during the browser phase.

## Open questions

Exact versions, supplied binary choices, and any use of dynamic linking will be chosen during scaffolding.

## Decision

Accepted on 2026-10-12 following the project owner's selection of local third-party libraries under `external/`.

Updated on 2026-10-13 to use SDL3 3.4.18. Following the project owner's request for an official prebuilt binary, the final native dependency is the macOS universal dynamic framework from the release DMG. The locally compiled archive and full source tree are excluded. CMake uses an imported shared framework target and copies the framework beside the executable. Browser integration needs a separate WASM-compatible library and remains pending.

Platform SDK organization is recorded in `docs/project-structure.md`. CMake now selects a shared `SDL3::SDL3` target for macOS, Windows MSVC x64, or Linux x86_64 and copies the appropriate runtime dependency. Windows and Linux SDKs are not supplied, and their branches and presets await native verification. Only macOS arm64 has been built and launched.
