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

Separate supplied binaries by platform, architecture, and configuration. Retain source for dependencies that need compilation to WASM. Use the Emscripten SDL2 port for the browser target and local SDL2 for native builds. Generated project outputs remain in `build/`.

The detailed conventions are maintained in `external/README.md`.

## Alternatives

- **vcpkg:** Provides package recipes, version baselines, and binary caching, but adds another tool and may require custom packaging for our backend selection.
- **CMake FetchContent:** Automates source acquisition with pinned revisions, but requires dependency retrieval and integration during configuration.
- **Local copies:** Requires manual updates and a larger repository, but matches the selected workflow and gives direct control over imported files and compilation.

## Risks and limitations

Maintainers own updates, patch tracking, and binary compatibility. Native binaries do not work in WASM. Dynamic libraries add runtime packaging work. Local copies do not eliminate toolchain installation or Emscripten port-cache downloads.

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

Accepted on 2026-10-12 following the project owner's selection of local third-party libraries under `external/`. Dependency import and build validation have not started.
