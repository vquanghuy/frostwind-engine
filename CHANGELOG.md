# Changelog

All notable changes to this experiment are documented in this file.

## [Unreleased]

### Added

- Planned audio technology noted in the README technical approach.
- C++ coding conventions covering ownership, initialization, callback flow, naming, and validation, linked from project guidance.
- RFC 0002 for native OpenGL integration, generated GLAD 2 files, and the planned WebGL 2 rendering path.
- Generated GLAD 2.0.8 OpenGL 3.3 Core loader, license and provenance record, and local native static CMake target. Native GLAD function loading uses SDL_GL_GetProcAddress.
- Native OpenGL Core context and procedural checkerboard quad, with shader diagnostics, drawable-pixel viewport sizing, VSync, and graphics cleanup before context destruction.

![Mini Artillery OpenGL checkerboard quad](docs/assets/milestones/m002-opengl-quad.png)

*Native OpenGL quad on macOS Apple M1 Pro, captured manually on 2026-10-13. Configure, build, launch, and checkerboard output verified with an OpenGL 4.1 context. Interactive resize/Retina and exit checks remain pending.*

### Changed

- Managed application callback ownership with `std::unique_ptr`, including automatic cleanup on initialization failure.
- Moved lifecycle and frame coordination into static `Application` callbacks, with `Window` owning SDL video/window lifetime, OpenGL initialization, and presentation. Kept native window access private, removed the project namespace, and documented the SDL-driven loop and resource lifetimes. The quad remains a temporary demo.

## [0.1.0] - 2026-10-13

### Added

- Native macOS SDL3 window with a plain background, resizing, event handling, and Escape or close-button exit.
- Local official SDL3 3.4.18 macOS universal framework, copied beside the executable during the CMake build.
- CMake configuration and Debug presets for macOS, Windows MSVC x64, and Linux x86_64. Windows and Linux SDKs are pending; those platforms remain unverified.
- Project documentation, RFC process, local dependency records, and platform SDK layout.
- Progress screenshots and an optional macOS window-capture helper.

![Mini Artillery native macOS window](docs/assets/milestones/m001-native-window.png)

*Native window on macOS Apple Silicon, captured on 2026-10-13. Native configuration, build, and launch verified; terrain rendering remains planned.*

[Unreleased]: https://github.com/vquanghuy/mini-artillery/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/vquanghuy/mini-artillery/tree/v0.1.0

Format based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
