# Development

## Native macOS prerequisites

- Apple Silicon Mac. The application currently uses the installed SDK's default deployment target; older macOS versions have not been validated.
- Xcode or Command Line Tools providing Apple Clang, the macOS SDK, and Make.
- CMake 3.22 or newer, available on `PATH`.

The official SDL3 3.4.18 macOS universal dynamic framework is included in `external/sdl3/`; no system SDL installation or dependency download is required during CMake configuration.

## Configure, build, and run

From the repository root:

```sh
cmake --preset native-debug
cmake --build --preset native-debug --parallel
./build/native/mini-artillery
```

The preset selects a Debug build for macOS arm64 using Unix Makefiles. Generated files and the executable stay under `build/native/`. CMake copies the official framework from `external/sdl3/macos-universal/` into `build/native/Frameworks/`. Keep the executable and that `Frameworks/` directory together when moving a build. CMake does not rebuild SDL3.

The application opens a resizable 960 × 540 window titled **Mini Artillery** with a plain dark background. Close the window or press Escape to exit.

## Current implementation

The executable links the imported `SDL3::SDL3` target from `cmake/SDL3.cmake`. The official framework contains both arm64 and x86_64 binaries and matching headers. The native preset builds the application for arm64; no locally compiled SDL archive is used. SDL3 main callbacks handle initialization, events, drawing, and shutdown. The empty window uses an SDL window surface; there is no application OpenGL context or terrain renderer yet. Native callbacks wait for events while idle.

Terrain, physics, ImGui, and browser compilation are later milestones. No automated test suite has been added for the window scaffold.

## Verification record

On 2026-10-13, the native application configured and rebuilt with CMake 4.4.2, Apple Clang 21.0.0, and macOS SDK 27.0. The final build uses the official SDL3 3.4.18 macOS framework. Its DMG checksum matched the upstream release metadata. The arm64 executable launched and reported successful window creation. `otool -L` confirmed the dependency on `@rpath/SDL3.framework/Versions/A/SDL3`.

Interactive resize and exit checks below remain manual verification steps. Browser builds have not been validated.

## Manual verification

After changes to the scaffold:

1. Configure and build successfully.
2. Launch and verify the window title and plain background.
3. Resize, minimize, and restore the window; verify background updates and responsiveness.
4. Exit with Escape, relaunch, then exit with the close button; verify clean termination.

To inspect runtime linkage on macOS:

```sh
otool -L build/native/mini-artillery
```

The executable should reference `@rpath/SDL3.framework/Versions/A/SDL3`. Its runtime search path points to the `Frameworks/` directory beside the executable. No system SDL installation is required.

## Windows x64 (prepared, unverified)

The `windows-debug` preset uses Visual Studio 2022 with the C++ desktop workload and targets x64. It requires CMake 3.22 or newer and a Windows SDK. MinGW and Windows ARM64 are not configured.

Before configuring, supply matching SDL3 3.4.18 headers, the MSVC x64 import library, and the runtime DLL from the official Visual C++ development package. Arrange them under `external/sdl3/windows-x64/` as described in [project structure](project-structure.md). No Windows SDK files have been imported yet; configuration reports the missing dependency rather than downloading it.

Commands to verify later from the repository root in PowerShell:

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug --parallel
.\build\windows\Debug\mini-artillery.exe
```

The build copies `SDL3.dll` beside the executable. Keep both together when moving the build. Verify headers, MSVC linkage, DLL loading, window updates, and shutdown on Windows before marking this platform validated.

## Linux x86_64 (prepared, unverified)

The `linux-debug` preset uses Unix Makefiles with GCC or Clang and CMake 3.22 or newer. Supply a local SDL3 3.4.18 SDK compatible with the selected distribution and libc under `external/sdl3/linux-x64/`. Headers and shared libraries are not included yet. An upstream source build outside the repository may be required; no general-purpose official Linux binary is assumed.

The SDK must provide `include/SDL3/SDL.h` and `lib/libSDL3.so.0`, including any symlink targets. Record the chosen distribution, libc requirements, architecture, license, and binary origin before verification.

Commands to verify later:

```sh
cmake --preset linux-debug
cmake --build --preset linux-debug --parallel
./build/linux/mini-artillery
```

The build copies the SDK's `lib/` directory beside the executable and uses `$ORIGIN/lib` for runtime lookup. Linux system dependencies are not bundled automatically. Verify linking, shared-library loading, the selected X11/Wayland backend, window updates, and shutdown on the target distribution.

## Platform status

| Target | Dependency supplied | Verification |
| --- | --- | --- |
| macOS arm64 | Official universal framework | Built and launched |
| macOS x86_64 | Included in universal framework | Not verified; preset targets arm64 |
| Windows MSVC x64 | Pending | CMake scaffold only |
| Linux x86_64 | Pending | CMake scaffold only |
| Browser/WASM | Pending | Not configured |
