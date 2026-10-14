# Development

## Native macOS prerequisites

- Apple Silicon Mac with Xcode or Command Line Tools providing Apple Clang and the macOS SDK.
- CMake 3.22 or newer on `PATH`.

The official SDL3 3.4.18 macOS universal framework and generated GLAD 2.0.8 files are included under `external/`. CMake needs both C and C++ compilers and performs no dependency downloads. Older macOS deployment targets have not been validated.

## Configure, build, and run

From the repository root:

```sh
cmake --preset native-debug
cmake --build --preset native-debug --parallel
./build/native/bin/frostwind-textured-quad/frostwind-textured-quad
```

The preset configures Debug for macOS arm64 using Unix Makefiles and builds the `frostwind-textured-quad` target, which depends on the static `frostwind` library. The example opens a resizable 800 × 600 window titled **Frostwind - Textured Quad**. Press Escape or close the window to exit.

The example output directory contains `assets/` and `Frameworks/SDL3.framework`. Keep the entire directory together when relocating it. Asset copying runs on each example build, including shader-only edits. Shader lookup is executable-relative, so launches from an IDE or another working directory use the same assets. The [example README](../examples/textured-quad/README.md) describes ownership and controls.

A CMake cache contains absolute checkout paths. After moving/renaming a checkout, configure a fresh output directory; preserve or remove old generated output deliberately rather than reusing its stale cache.

### Engine-only build

To build the library without any examples on this development Mac:

```sh
cmake -S . -B build/engine-only -DFROSTWIND_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Debug -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build/engine-only --parallel
```

This produces `build/engine-only/engine/libfrostwind.a` and the local GLAD archive. Runtime dependencies are packaged when an executable is built; the library alone opens no window.

## IDE setup

CLion can open the root `CMakeLists.txt` and use the native preset. Select `frostwind-textured-quad` as the executable target. CMake supplies the C++17, GLAD, and SDL framework settings; no separate include paths are needed.

For VS Code, install Microsoft's C/C++ and CMake Tools extensions, select `native-debug` with **CMake: Select Configure Preset**, then run **CMake: Configure**. The preset exports `build/native/compile_commands.json`.

Suggested local `.vscode/settings.json`:

```json
{
    "cmake.useCMakePresets": "always",
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "C_Cpp.default.compileCommands": "${workspaceFolder}/build/native/compile_commands.json"
}
```

Use **C/C++: Reset IntelliSense Database** and **Developer: Reload Window** if stale diagnostics remain. See Microsoft's [IntelliSense guide](https://code.visualstudio.com/docs/cpp/configure-intellisense).

For local macOS debugging, use `type: cppdbg`, `request: launch`, `MIMode: lldb`, and program `${workspaceFolder}/build/native/bin/frostwind-textured-quad/frostwind-textured-quad`. A pre-launch task can run the configure/build preset commands above. The C/C++ extension supplies `lldb-mi`; do not set `miDebuggerPath` to `/usr/bin/lldb`, which does not accept MI commands. See Microsoft's [LLDB-MI guide](https://code.visualstudio.com/docs/cpp/lldb-mi). Editor files remain ignored; interactive IDE refresh/debugging requires local verification.

## Current implementation

The static engine library contains namespaced `Application`, `Window`, `ShaderProgram`, and `Texture2D` classes. `Application` hosts a derived example through SDL callbacks; example resources are destroyed before its base window/context. SDL video initialization and graphics presentation stay in `Window`. See [project structure](project-structure.md) and [RFC 0003](rfcs/0003-frostwind-engine-and-examples.md).

The OpenGL API baseline is 3.3 Core, not an exact runtime context version requirement. The development Mac supplies a compatible 4.1 context. GLAD's generated API is through 3.3, independently of generator version 2.0.8. The example uses GLSL 330 and a PNG crate texture decoded to RGBA8. The example defaults to a 60 FPS software cap with VSync requested. `ApplicationConfig::maxFps` selects another cap; 0 disables software pacing. Set `config.window.vsync = false` as well to request uncapped rendering. Rendering and swap time count toward the frame budget; SDL precise delays wait only for the remainder. VSync, rendering load, and OS scheduling can lower achieved FPS. Minimized windows use a separate idle delay. See [RFC 0004](rfcs/0004-frame-pacing.md).

Shader source/file loading, compilation/link diagnostics, uniform updates, and RGBA8 upload validation are implemented. Camera, audio, physics, terrain destruction, and other graphics backends remain planned. No automated test suite is established; temporary validation artifacts remain under ignored `build/`.

## Verification record

Before the engine migration, the native scaffold was configured, built, and launched on 2026-10-13 with CMake 4.4.2, Apple Clang 21.0.0, and macOS SDK 27.0. The supplied SDL framework's release checksum and relative loading were checked. The M001/M002 screenshots in the changelog preserve that Mini Artillery history.

Shader/texture checks verified compilation/link failure recovery, file errors and paths with spaces, cached uniforms, RGBA8 upload/readback with non-default unpack state, resource destruction, and checkerboard/background pixel colors. The file-based demo was also exercised from `/private/tmp`. These observations apply to the original scaffold before extraction.

On 2026-10-14, the engine/example migration passed native configuration/build and an independent `TOYBOX_BUILD_EXAMPLES=OFF` build. Temporary native checks linked against the static engine and ran from `/private/tmp`. They verified shader/texture failure recovery, readback and matching demo colors; application factories and callback results; standard/unknown callback exceptions; failed video/startup handling; and derived-resource destruction while the context was still current. The actual `QuadDemo` also initialized, presented a frame, and handled a synthetic Escape event. These were executed checks, not a committed automated test suite.

The built `toybox-textured-quad` executable also launched through its SDL entry point and reported successful initialization with the Apple M1 Pro's OpenGL 4.1 context. Interactive window checks remain manual.

Browser builds and Windows/Linux native behavior have not been validated. The owner supplies screenshots manually; do not capture them automatically.

### Texture-file validation (2026-10-14)

Native configuration/build and focused OpenGL checks passed on Apple M1 Pro for PNG/JPEG loading, RGBA/gray-alpha conversion, exact pixel orientation and alpha readback, Unicode filenames, missing/empty/invalid/truncated input, excessive dimensions, retry after failure, repeated initialization rejection, pixel-unpack state restoration, resource destruction, and a rendered quad pixel matching the uploaded texture. Checks loaded the packaged crate texture from `/private/tmp`; tutorial JPEG input was used only for a local check and was not imported. The generated crate replaces the running example's checkerboard; the owner supplied its M003 screenshot on 2026-10-14, preserved in the changelog and example README.

The updated example also launched successfully from `/private/tmp` through its SDL callbacks. Interactive resize/Retina checks remain manual.

Texture support setup was subsequently moved behind `Texture2D::initializeSupport()`. Native build and focused checks passed for rejecting loads before support initialization, support failure/recovery, orientation and rendering, and reusing the cached limit with no `GL_MAX_TEXTURE_SIZE` queries during texture loads.

### Frostwind rename validation (2026-10-14)

The renamed native preset and a fresh engine-only build with `FROSTWIND_BUILD_EXAMPLES=OFF` passed. Native texture/rendering and lifecycle checks linked against `libfrostwind.a` and passed from `/private/tmp`, including asset lookup, orientation/readback, cached texture limits, failure recovery, callback exceptions, and GPU cleanup before context teardown. The renamed `frostwind-textured-quad` executable launched successfully with the Frostwind window title. The owner supplied the M004 screenshot, now embedded in the changelog and example README. The new GitHub remote was verified over SSH. Existing Toybox validation records above describe the historical implementation.

### Frame pacing validation (2026-10-14)

Native configuration/build passed. Temporary controlled-clock checks under `build/frame-pacing-check/` verified remaining-budget waits including render/swap time, 60/120 FPS caps, uncapped mode, slow frames and stalls without catch-up, minimized/zero-size handling and restore, callback results/exceptions, presentation/drawable failures, and startup cleanup. Real SDL timer checks with simulated 2 ms rendering and 1 ms swap work measured 29.43, 58.95, and 111.21 callbacks/sec for 30, 60, and 120 FPS caps respectively; OS scheduling may reduce the achieved rate.

The actual textured-quad example loaded its packaged assets from `/private/tmp`, presented 120 frames per run on Apple M1 Pro, and handled synthetic Escape/teardown with a 60 FPS cap. Measured callback throughput was 56.98/sec with VSync disabled and 59.15/sec with VSync requested. The initial sandboxed launch could not access macOS displays; the successful run used approved display access outside the sandbox. No screenshots were captured. Interactive smoothness, resize/minimize/restore, and other platforms remain manual checks.

## Manual checks

1. Configure/build successfully and launch the textured-quad example.
2. Verify title, dark background, and centered crate image with the arrow pointing up; check the reported OpenGL context/renderer.
3. Resize wide/tall, check Retina drawable sizing, minimize and restore; the quad should stay square and centered.
4. Exit with Escape; relaunch and exit with the close button.
5. Launch from outside the repository to check asset lookup; move an intact example output directory when testing relocation.

Resize/Retina, interactive exit, and IDE behavior remain manual checks unless explicitly recorded as verified.

To inspect the macOS runtime dependency:

```sh
otool -L build/native/bin/frostwind-textured-quad/frostwind-textured-quad
```

The executable should reference `@rpath/SDL3.framework/Versions/A/SDL3`, with the example's copied `Frameworks/` directory available through `@loader_path/Frameworks`.

## Other platforms (prepared, unverified)

Windows MSVC x64 and Linux x86_64 presets select the new example target, but their SDKs are not supplied. The expected SDL3 layouts are recorded in [project structure](project-structure.md#platform-sdks-and-header-lookup). Each platform must validate compilation, dynamic loading, rendering, input, and cleanup before being described as supported.

Windows needs Visual Studio 2022 with C++ desktop tools, a Windows SDK, and matching SDL3 3.4.18 MSVC headers/import library/DLL:

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug --parallel
.\build\windows\bin\frostwind-textured-quad\Debug\frostwind-textured-quad.exe
```

Linux needs a compatible SDL3 3.4.18 x86_64 SDK, GCC or Clang, and Make. Distribution/libc requirements and SDK provenance must be recorded:

```sh
cmake --preset linux-debug
cmake --build --preset linux-debug --parallel
./build/linux/bin/frostwind-textured-quad/frostwind-textured-quad
```

Windows packages the DLL beside its executable; Linux packages `lib/` and uses `$ORIGIN/lib`. Linux system dependencies are not bundled automatically. Browser compilation still requires a separate WASM-compatible SDL3 library and Emscripten integration; no working browser build command is claimed.
