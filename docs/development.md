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

The preset selects a Debug build for macOS arm64 using Unix Makefiles. Generated files and the executable stay under `build/native/`. CMake copies the official framework from `external/sdl3/macos-universal/` into `build/native/Frameworks/`. CMake also copies runtime shaders to `build/native/assets/shaders/`. Keep the executable, `assets/`, and `Frameworks/` together when moving a build. Shader-only edits are copied on the next build without recompiling C++. CMake does not rebuild SDL3.

The application opens a resizable 960 × 540 window titled **Mini Artillery** with a centered green/teal checkerboard quad on a dark background. Close the window or press Escape to exit.

## VS Code on macOS

Open the repository root and use Microsoft's C/C++ (`ms-vscode.cpptools`) and CMake Tools (`ms-vscode.cmake-tools`) extensions. Select `native-debug` with **CMake: Select Configure Preset**, then run **CMake: Configure**.

Set the following workspace settings in the ignored `.vscode/settings.json`:

```json
{
    "cmake.useCMakePresets": "always",
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "C_Cpp.default.compileCommands": "${workspaceFolder}/build/native/compile_commands.json"
}
```

The preset already exports `build/native/compile_commands.json`, including the compiler, C++17 mode, GLAD headers, and SDL framework search path. IntelliSense should obtain these from CMake rather than a separately maintained `includePath`. See Microsoft's [IntelliSense configuration guide](https://code.visualstudio.com/docs/cpp/configure-intellisense).

If include errors persist after configuring, run **C/C++: Reset IntelliSense Database**, then **Developer: Reload Window**. Native builds and the compile database have been verified; the editor's diagnostic refresh still needs confirmation in the VS Code UI. Local editor settings remain untracked.

### Debugging

Use a `cppdbg` launch configuration with `MIMode` set to `lldb`. Microsoft's C/C++ extension supplies the `lldb-mi` adapter; do not set `miDebuggerPath` to `/usr/bin/lldb`, which does not accept the adapter's `--interpreter=mi` option. See Microsoft's [LLDB-MI setup](https://code.visualstudio.com/docs/cpp/lldb-mi).

The local `.vscode/launch.json` selects **Mini Artillery (native-debug)** and launches `build/native/mini-artillery` with the repository root as its working directory. Its **Build native-debug** pre-launch task in `.vscode/tasks.json` runs the native configure preset followed by the build preset. Select this configuration in **Run and Debug**, then press F5; use it instead of an automatically generated debugger configuration.

When recreating these ignored local files, use `type: cppdbg`, `request: launch`, `MIMode: lldb`, `program: ${workspaceFolder}/build/native/mini-artillery`, `cwd: ${workspaceFolder}`, and `preLaunchTask: Build native-debug`. Define that task to run `cmake --build --preset native-debug --parallel`, depending on a configure task running `cmake --preset native-debug`.

The installed adapter accepted MI commands, loaded the native executable, and resolved a breakpoint in `Application::onInit`. An interactive F5 launch and breakpoint stop still need verification in VS Code.

## Current implementation

The executable links the imported `SDL3::SDL3` target from `cmake/SDL3.cmake`. The official framework contains both arm64 and x86_64 binaries and matching headers. The native preset builds the application for arm64; no locally compiled SDL archive is used. SDL3 main callbacks handle initialization, events, drawing, and shutdown. The window uses a forward-compatible OpenGL Core context requesting version 3.3. GLAD loads functions through SDL_GL_GetProcAddress. A GLSL 330 program draws a procedural RGBA8 checkerboard texture on two triangles. Frames render continuously with VSync; a small delay limits rendering if VSync is unavailable or the window is minimized. Terrain rendering is not implemented.

Terrain, physics, ImGui, and browser compilation are later milestones. No automated test suite has been added for the window scaffold.

The demo owns reusable `ShaderProgram` and `Texture2D` objects for linked shaders and in-memory RGBA8 textures. Shader compilation/link diagnostics and texture upload validation live in those support classes. `ShaderProgram::initializeFromFiles` loads the demo’s `.vert` and `.frag` files. The application locates its runtime shaders relative to the executable, so CLion, VS Code, and terminal launches can use different working directories. Image-file decoding is not implemented. See [RFC 0002](rfcs/0002-graphics-integration.md#rendering-support-classes) for their contracts.

OpenGL 3.3 Core is the desktop API baseline, not an exact runtime version requirement. macOS currently supplies a compatible 4.1 context. The `GLAD loaded 4.1` log reports the detected context version; the generated loader still contains only the requested API through 3.3. GLAD's generator version is independently pinned at 2.0.8.

GLAD 2.0.8 is compiled from local generated C source as a static library and linked through `GLAD::GLAD` in native builds. CMake requires both C and C++ compilers. No generator, Python package, or network access is needed during configuration. `src/app/Application.*` contains static SDL callbacks and owns `Window` and the temporary quad demo. `Window` manages SDL video initialization, the native window, its OpenGL context, GLAD loading, and presentation, with private native handles; `main.cpp` only enables SDL callback mode. SDL pumps events and repeatedly invokes the application’s frame callback. Their responsibilities and destruction order are recorded in [project structure](project-structure.md). See the [GLAD dependency record](../external/glad/README.mini-artillery.md).

## Verification record

On 2026-10-13, the native application configured and rebuilt with CMake 4.4.2, Apple Clang 21.0.0, and macOS SDK 27.0. The final build uses the official SDL3 3.4.18 macOS framework. Its DMG checksum matched the upstream release metadata. The arm64 executable launched and reported successful window creation. `otool -L` confirmed the dependency on `@rpath/SDL3.framework/Versions/A/SDL3`.

Interactive resize and exit checks below remain manual verification steps. Browser builds have not been validated.

On 2026-10-13, native configuration and build also passed with the generated GLAD 2.0.8 source compiled as C and its static library on the executable's link command. The app launched on Apple M1 Pro and reported OpenGL 4.1 Metal, successful GLAD loading, and quad initialization. A manually supplied full-window screenshot confirms checkerboard output and is preserved in the changelog. Automated capture returned a Stage Manager thumbnail while the app was in the background. Interactive resize, minimize/restore, and Escape/close-button exit remain manual checks.

After the ownership refactor on 2026-10-13, native configuration and build passed. A temporary check under `build/native/` exercised `Application` static callbacks for startup, frame presentation, synthetic Escape and quit events, and teardown, confirming that destruction released the current context and SDL video subsystem. It also verified graceful startup failure with an invalid SDL video driver. These checks do not replace interactive window checks. Future milestone screenshots are supplied manually by the project owner and copied into documentation assets when requested.

On 2026-10-13, the `ShaderProgram`/`Texture2D` extraction passed native configuration and build. A temporary `build/native/rendering-resource-check` verified shader compile/link failure recovery, cached uniform values, invalid texture inputs, RGBA8 GPU readback with non-default unpack layout and a bound unpack buffer, repeated-initialization rejection, and resource destruction. It also drew the demo and read back the expected checkerboard/background colors without OpenGL errors. This check requires a native graphics session; browser support remains unverified.

The same check was extended for shader files and passed when launched with `/private/tmp` as its working directory. It verified missing vertex/fragment files, empty input, file-based compile/link failures followed by successful retry, paths containing spaces, and matching demo pixels using copied executable-relative shaders. Build output confirmed runtime asset copying on subsequent builds.

## Manual verification

After changes to the scaffold:

1. Configure and build successfully.
2. Launch and verify the window title, dark background, and crisp checkerboard quad. Check the log for the actual OpenGL version and renderer.
3. Resize to wide and tall shapes; verify the quad stays square and centered. Check Retina drawable sizing, minimize, and restore; verify responsiveness.
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
