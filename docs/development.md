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

The preset configures Debug for macOS arm64 using Unix Makefiles and builds all three example targets, each linked to the static `frostwind` library:

| Target | Window title | Current behavior |
| --- | --- | --- |
| `frostwind-textured-quad` | Frostwind - Textured Quad | 800 × 600 window with a centered crate texture |
| `frostwind-asteroids` | Frostwind - Asteroids | 960 × 540 window with a plain dark-blue background |
| `frostwind-mini-artillery` | Frostwind - Mini Artillery | 960 × 540 window with a plain dark-green background |

Run the scaffolds separately:

```sh
./build/native/bin/frostwind-asteroids/frostwind-asteroids
./build/native/bin/frostwind-mini-artillery/frostwind-mini-artillery
```

All windows are resizable; press Escape or close the window to exit. Asteroids scene features and Mini Artillery terrain/gameplay remain planned. To build only one example after configuring:

```sh
cmake --build --preset native-debug --target frostwind-asteroids --parallel
cmake --build --preset native-debug --target frostwind-mini-artillery --parallel
```

Each example output directory contains `Frameworks/SDL3.framework`. Textured Quad also contains `assets/`; its assets are copied after the executable is linked. Shader lookup is executable-relative, so launches from an IDE or another working directory use the same assets. The new scaffolds use no runtime assets and do not package planning artwork. Keep each entire output directory together when relocating it. Each example README describes ownership and controls.

After changing shaders or textures, use **Rebuild** on `frostwind-textured-quad` in the IDE. From the terminal, force a rebuild with:

```sh
cmake --build --preset native-debug --target frostwind-textured-quad --clean-first --parallel
```

The example's own `POST_BUILD` step copies its shaders, texture, and license file beside the executable after linking. An ordinary incremental Build with no C++ changes may do nothing, so use Rebuild for asset-only edits. Select the example executable as the build/run target.

A CMake cache contains absolute checkout paths. After moving/renaming a checkout, configure a fresh output directory; preserve or remove old generated output deliberately rather than reusing its stale cache.

### Engine-only build

To build the library without any examples on this development Mac:

```sh
cmake -S . -B build/engine-only -DFROSTWIND_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Debug -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build/engine-only --parallel
```

This produces `build/engine-only/engine/libfrostwind.a` and the local GLAD archive. Runtime dependencies are packaged when an executable is built; the library alone opens no window.

## IDE setup

CLion can open the root `CMakeLists.txt` and use the native preset. Select `frostwind-textured-quad`, `frostwind-asteroids`, or `frostwind-mini-artillery` as the executable target. CMake supplies the C++17, GLAD, and SDL framework settings; no separate include paths are needed.

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

The static engine library contains Application, Window, Renderer, Texture2D, ShaderProgram, VertexBuffer, IndexBuffer, VertexArray, CPU Image, and Application-owned viewport setup. Window owns SDL video/window/context, GLAD loading, VSync, and presentation. Texture/program/vertex classes perform their own OpenGL operations with private handles; there is no device/backend/pipeline/frame/surface hierarchy. See [project structure](project-structure.md), the [rendering guide](rendering-guide.md), and [RFC 0006](rfcs/0006-opengl-focused-rendering.md).

Examples override `initialize()`, `handleEvent(event)`, and `render(Renderer& renderer, int width, int height)`. Application owns one persistent Renderer, makes the context current, sets the viewport from drawable pixels, passes the renderer by reference, and presents continuing callbacks. Renderer clears and submits draws; examples do not retain the callback reference or own another renderer. Resources require their creating context to remain current and outlive them. Derived members are destroyed before the base Window/context, including after failed startup. Context checks diagnose misuse without switching contexts or retaining the window. Native graphics calls stay in engine implementations; GLAD includes remain private.

The baseline is OpenGL 3.3 Core with GLAD 2.0.8; the development Mac supplies a compatible 4.1 context. Textured Quad uses GLSL 330, a regular `vec2 uScale`, a unit-0 sampler, four vertices/six indices, and the packaged crate image. ShaderProgram provides activation, cached uniform lookup, integer/float/vec2/3/4 and column-major mat4 writes. VertexBuffer owns vertex bytes; IndexBuffer owns u32 indices and cached bounds; VertexArray owns the layout/buffer associations and stores vertex/index counts. Renderer draws the whole array through one `drawTriangles(vertexArray)` method, selecting indexed or sequential drawing automatically and restoring the prior VAO. Texture2D validates/upload RGBA8 bytes or composes CPU PNG/JPEG decoding and upload, preserving unpack state. No static texture-support setup or padded quad parameter block is needed.

All examples default to a 60 FPS software cap with VSync requested. `ApplicationConfig::maxFps` chooses another cap; 0 disables software pacing. Set `config.window.vsync = false` too when measuring uncapped rendering. Render/swap time counts toward the callback budget and slow frames do not create catch-up work. Minimized windows use a separate idle delay; zero-sized targets use the remaining budget or a 16 ms uncapped fallback. See [RFC 0004](rfcs/0004-frame-pacing.md).

Camera, audio, physics, terrain destruction, and depth/instancing features remain planned. Multi-backend work is deferred; the prior implementation is archived at `e9cf616` on `archive/multi-backend-experiment`, with research in RFC 9999. Windows/Linux and browser support remain unverified. No automated test suite is established; temporary validation artifacts remain under ignored `build/`.

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

### Example scaffold verification (2026-10-17)

The native preset configured and built all three examples on Apple M1 Pro. A fresh `build/example-scaffolds-engine-only/` configuration with `FROSTWIND_BUILD_EXAMPLES=OFF` also built successfully. Packaging checks verified separate executable directories, copied SDL framework symlinks, Textured Quad's retained runtime assets, and no runtime assets or planning artwork in either new scaffold. All build preset target lists and local links/images in the updated documentation were checked.

Temporary checks under `build/example-scaffolds-check/` compiled the actual Asteroids and Mini Artillery entry points and called their SDL adapters from `/private/tmp`. Both initialized an OpenGL 4.1 context, presented frames without GL errors, returned success for synthetic Escape and window-close events, and released the context during teardown. The sandboxed attempt could not access macOS displays; the successful checks used approved display access outside the sandbox. No screenshots were captured. Interactive titles/backgrounds, resize/Retina, minimize/restore, IDE behavior, and other platforms remain manual checks.

### Numbered example directory verification (2026-10-18)

After renaming source directories to `01-textured-quad`, `02-asteroids`, and `03-mini-artillery`, the native preset reconfigured and rebuilt all three examples successfully. Target names and executable output paths remain unchanged. Checks validated 134 local documentation links/images and confirmed that the moved C++, CMake files, shaders, and image assets were byte-for-byte unchanged. Historical changelog sections were preserved. This directory-only change did not repeat the runtime checks recorded above.

### Graphics ownership validation (2026-10-19)

The native preset configured and built all three examples before and after the first shared-rendering ownership refactor. A fresh engine-only configuration under `build/ownership-engine-only/` also built successfully. Temporary checks under `build/ownership-check/` compiled the actual example entry points and linked the static engine, running from `/private/tmp` with approved native display access.

All three examples initialized, presented three frames per run, and handled synthetic Escape/window-close events. Pre-swap framebuffer hashes matched the pre-refactor baseline exactly for Textured Quad, Asteroids, and Mini Artillery. Pixel checks confirmed the expected backgrounds and a textured quad center, with no GL errors. These were in-memory GPU readback checks; no screenshots were captured.

Fault injection verified SDL video/attribute/window/context/make-current failures, GLAD loading failure, invalid texture-support limits, presentation failure, and nonfatal swap-interval failure. Callback factory/startup/event/frame results and exceptions, initialization guards, and retry after failed device initialization were exercised. Native destruction checks verified texture deletion before context destruction, context destruction before the SDL window, and video cleanup after failed startup. A real resize verified that the callback received the current drawable dimensions.

Controlled-clock checks with a mocked graphics device verified remaining-budget waits, render/presentation time accounting, 60/120 FPS caps, uncapped and extreme-cap handling, slow/stalled frames without catch-up, minimized/zero-size skips and restore, and callback/presentation/drawable failure results. Native automated minimization requested successfully but `SDL_SyncWindow` did not confirm the transition; interactive minimize/restore remains manual, as do visual resize/Retina assessment and IDE checks. Other platforms remain unverified.

### Shared rendering validation (2026-10-19)

After migrating frames/resources and all three examples, the native preset configured and built successfully, as did a fresh engine-only configuration under `build/shared-rendering-engine-only/`. Temporary checks under `build/rendering-check/` compiled the actual example entry points, linked the static engine, and ran from `/private/tmp` with approved display access. All three examples initialized, rendered three frames per run, and exited through synthetic Escape/window-close events. Full pre-swap framebuffer hashes matched the pre-migration baseline exactly for every example; no screenshots were captured.

Native resource checks rendered an untextured three-vertex triangle through the same draw interface and verified two successive textured draws using reused CPU parameter storage. They confirmed local shader-owner cleanup leaves a live pipeline usable, resource owners can die before frame submission without invalidating work, and retained native objects are deleted after completion. Checks rejected nested/reused/completed frames, foreign-device submission/resources, stale backend identities, out-of-range draws, missing bindings, wrong parameter sizes, invalid attributes/block/sampler layouts, wrong shader stages, repeated initialization, and invalid shader/files/pixel dimensions. Failed draws/clears prevented presentation, and exception unwinding abandoned frames. Injected texture-upload failure preserved unpack state, cleaned partial resources, and permitted retry.

CPU image checks loaded PNG/JPEG before graphics initialization, verified asymmetric pixel orientation, and exercised missing/corrupt files and excessive dimensions. Native lifecycle fault injection repeated video/attribute/window/context/make-current/GLAD/limit failures, presentation failure, VSync fallback, callback results/exceptions, initialization guards, real drawable resize, and texture/context/window destruction ordering. Controlled-clock checks using a fake backend verified render/swap time accounting, capped/uncapped/extreme-rate pacing, slow frames/stalls, minimized/unavailable skips, and callback/submission/acquisition failures. Unavailable targets now receive the configured frame budget or an uncapped 16 ms fallback.

Source checks found no native graphics calls/types in examples or public rendering headers; all five example compilation commands omitted GLAD include directories. Documentation links/anchors, source organization, and whitespace were checked. These checks are temporary validation, not a committed automated test suite. Native minimize/restore still requires manual confirmation because `SDL_SyncWindow` did not confirm the requested transition. Visual Retina/resize assessment, IDE behavior, Windows/Linux, browser integration, and additional backends remain unverified.

### Textured Quad simplification validation (2026-10-20)

After folding the example-owned `QuadRenderer` into `QuadDemo`, the native preset configured and built all three examples successfully. The focused native check under `build/rendering-check/` compiled the updated actual entry point and `QuadDemo`, then ran from `/private/tmp` with approved display access. It rendered three frames per run and exited through synthetic Escape/window-close events. Both framebuffer hashes matched the previous quad baseline exactly, with no GL errors; context/video teardown passed. No screenshots were captured. The example README ownership diagram and updated local documentation links/anchors were checked. Interactive checks and other platforms retain the limitations recorded above.

### OpenGL-focused simplification validation (2026-10-20)

Native preset configure/build and a fresh engine-only build under `build/opengl-focus-engine-only/` passed. Temporary checks under `build/opengl-focus-check/` compiled actual example entry points and ran from `/private/tmp` with approved native display access after the sandboxed attempt could not access displays. Each example rendered three frames in two runs with synthetic Escape/close exits. Full pre-swap framebuffer hashes matched the archived baseline exactly: Textured Quad `1129920399774065595`, Asteroids `13647017018793777957`, and Mini Artillery `14525124151450262309`. No screenshots were captured.

Direct resource checks passed for shader compilation/link/file failure recovery, repeated initialization rejection, cached integer/float/vec2/3/4/mat4 writes and matrix readback, missing uniforms, wrong-active-program writes, and two shader programs drawing shared geometry with distinct expected pixel colors. Texture checks covered PNG/JPEG CPU decoding before graphics initialization, corrupt/missing/limited input, exact asymmetric PNG upload/readback orientation, RGBA8 validation, invalid units, injected upload failure/retry, and texture/pixel-unpack/buffer state preservation. Vertex checks covered duplicate/invalid layout, invalid ranges, missing active programs, injected buffer-upload failure/retry, and partial object cleanup. Foreign-context operations were rejected without changing the owning context; resource cleanup left no GL errors.

Lifecycle checks exercised factories, attribute/window/context/make-current/GLAD/texture-limit failures, failed video initialization, nonfatal VSync failure, presentation failure, standard/unknown callback exceptions, success/failure returns without presentation, Window initialization guards/retry, and resource destruction before context/window/video teardown. A real resize confirmed callback dimensions match drawable pixels. Controlled-clock checks verified render/swap budget accounting, 60/120 FPS, uncapped/extreme caps, slow frames/stalls, minimized skips, zero-sized targets including the uncapped 16 ms fallback, and callback/query/presentation failures.

Native automated minimize requested successfully but SDL_SyncWindow did not confirm it; interactive minimize/restore remains manual. Visual Retina/resize assessment, IDE use, Windows/Linux, and browser behavior remain unverified. Earlier verification sections describe historical implementations; they do not establish current device/frame APIs.

After the usage review, color clearing moved into protected `Application::clear(Color)` and viewport setup moved directly into application iteration. The separate render-command module was removed; private resource context/error helpers were renamed OpenGLChecks. Native builds and the temporary resource/lifecycle/controlled-pacing checks passed again. All three actual example checks retained the exact framebuffer hashes above after switching to inherited `clear(...)`. Documentation links and source boundaries were rechecked; the manual/platform limitations remain unchanged.

### Renderer callback validation (2026-10-20)

After moving clear/draw submission to Renderer and passing the Application-owned persistent instance into all render callbacks, native builds and the temporary resource/lifecycle/pacing checks passed. Actual Textured Quad, Asteroids, and Mini Artillery entry points each rendered three frames in two runs and retained their exact archived framebuffer hashes listed above. Checks confirmed that successive callbacks receive the same renderer address, geometry is shared between shader programs through Renderer::drawTriangles, invalid ranges/missing programs are rejected, and successful draws preserve a pre-existing VAO binding. A focused foreign-context check with an active foreign shader also rejected the borrowed buffer and preserved that context's existing VAO without GL errors. VertexBuffer contains no draw operation or native draw call. No screenshots were captured; native minimize/restore still requires manual verification.

## Manual checks

1. Configure/build successfully and launch each example.
2. Verify title, dark background, and centered crate image with the arrow pointing up; check the reported OpenGL context/renderer.
3. Resize wide/tall, check Retina drawable sizing, minimize and restore; the quad should stay square and centered.
4. Exit with Escape; relaunch and exit with the close button.
5. Launch from outside the repository to check asset lookup; move an intact example output directory when testing relocation.
6. Check the Asteroids and Mini Artillery scaffold titles/backgrounds, resize/minimize/restore, and Escape/close exit; no scene or gameplay should be present yet.

Resize/Retina, interactive exit, and IDE behavior remain manual checks unless explicitly recorded as verified.

To inspect the macOS runtime dependency:

```sh
otool -L build/native/bin/frostwind-textured-quad/frostwind-textured-quad
```

The executable should reference `@rpath/SDL3.framework/Versions/A/SDL3`, with the example's copied `Frameworks/` directory available through `@loader_path/Frameworks`.

## Other platforms (prepared, unverified)

Windows MSVC x64 and Linux x86_64 presets select all three example targets, but their SDKs are not supplied. The expected SDL3 layouts are recorded in [project structure](project-structure.md#platform-sdks-and-header-lookup). Each platform must validate compilation, dynamic loading, rendering, input, and cleanup before being described as supported.

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

### Exception-based rendering validation (2026-10-20)

The native preset built all three examples after converting rendering/resource commands and CPU Image loading to exception-based failures. Temporary checks under `build/opengl-focus-check/` were adapted to the new API and ran from `/private/tmp` with approved display access. Each actual example entry point rendered three frames in two runs, handled synthetic Escape/close exits, and retained its exact framebuffer hash listed above. No screenshots were captured.

Resource checks exercised failed shader compilation/linking/file loading and retry, missing required uniforms, wrong-active-program writes, native uniform type errors, texture/image validation, injected texture/buffer upload failures and retry, unpack/binding preservation, shared-geometry draws, foreign-context rejection, and resource deletion. A focused injected native draw error verified an operation-specific exception, drained GL errors, and restoration of the previous VAO; foreign-context binding failures also preserved that binding. GPU owner destructors were checked as non-throwing at compile time, and failed initialization unwound while the context was still available.

Lifecycle checks confirmed that real resource exceptions during initialization, events, and rendering reach the SDL callback boundary and produce one diagnostic each. Failed frames returned `SDL_APP_FAILURE` without swapping; failed startup left `appstate` null. Standard/unknown callback exceptions, Window failures/retry, real drawable resize, VSync fallback, presentation failure, and texture-before-context/window/video teardown also passed. Controlled-clock pacing checks passed with the updated private check signatures.

Local documentation links/images/anchors and code fences, source boundaries, and diff whitespace were checked. These remain temporary checks rather than a committed automated suite. Native minimize/restore still requires manual confirmation because SDL did not synchronize the transition; visual Retina/resize assessment, IDE use, Windows/Linux, and browser behavior remain unverified.

The private helpers were subsequently renamed from OpenGLChecks to `GraphicsChecks` without behavior changes. All three native examples rebuilt successfully; documentation links and diff whitespace checks passed. Runtime checks were not repeated for this rename.

### Indexed triangle drawing validation (2026-10-20)

The native preset and temporary engine-only verification project under `build/opengl-focus-check/` built successfully. Native checks ran from `/private/tmp` with approved display access on Apple M1 Pro/OpenGL 4.1 using the engine's GLAD 3.3 interface. Actual Textured Quad, Asteroids, and Mini Artillery entry points each rendered three frames in two runs and handled synthetic Escape/close exits. All retained their exact earlier framebuffer hashes, including the quad's migration to four vertices and six indices. No screenshots were captured.

Focused indexed checks exercised four vertices with six indices, shared geometry with two shader programs and expected red/blue pixels, selected ranges with nonzero byte offsets, valid subranges excluding invalid indices elsewhere, maximum-u32 indices, invalid count/range/vertex references, missing programs, no-context and foreign-context operations, and retained CPU/GPU index contents after the caller's array changed. An injected index-upload error restored bindings, deleted the partial buffer, left metadata empty, and allowed retry. Successful and failed draws preserved the caller's VAO/EBO, the geometry VAO's previous element-buffer binding, and array/copy-write bindings. Injected native draw errors were drained and reported through an operation-specific exception. Sequential triangle rendering still produced the expected pixels.

Resource and lifecycle checks passed with IndexBuffer ownership: injected index-upload startup failure left appstate null, an invalid indexed frame returned failure without swapping, and texture/index-buffer deletion preceded context/window/video teardown. Existing shader/texture/uniform, Window fault/retry, real drawable resize, and exception checks also passed. Documentation links/images/anchors, fences, source boundaries, and whitespace were checked. Temporary validation code remains under ignored `build/`; no permanent automated suite was added. Interactive minimize/restore, visual Retina/resize assessment, IDE use, Windows/Linux, and browser behavior retain the limitations recorded above.

### Separate VertexArray validation (2026-10-20)

The native preset and the temporary engine-only verification project under `build/opengl-focus-check/` built successfully after separating raw VBO bytes from VAO layout/associations. Native checks ran from `/private/tmp` with approved display access. All three actual example entry points retained their earlier framebuffer hashes through the VertexArray migration; after adding stored-count overloads, the quad was rerun and again matched `1129920399774065595` in two three-frame runs with synthetic Escape/close exits. No screenshots were captured.

Focused checks verified indexed and non-indexed arrays, shared VBO storage with separate layouts/index associations, stride-derived vertex counts and stored index counts, full-range draw overloads, explicit subranges/byte offsets, and expected pixel colors. Missing indices produce a clear indexed-draw exception; uninitialized supplied buffers, invalid/duplicate attributes, invalid stride, repeated initialization, and foreign-context initialization/use were rejected. Injected attribute-layout errors deleted the partial VAO, restored prior VAO/array-buffer bindings, kept metadata empty, and permitted retry. Array configuration and draws preserved caller VAO/EBO state and configured associations, including native draw failures.

Lifecycle checks added injected VAO creation failure, verified failed startup leaves appstate null, checked missing-index frame failure without presentation, and confirmed VAO/VBO/IBO/texture deletion before context/window/video teardown. Existing shader, texture, uniform, image, upload retry, and native Window fault/resize checks passed with the revised API. Local documentation links/images/anchors, fences, source boundaries, and whitespace were checked. The checks remain temporary under ignored `build/`. Interactive minimize/restore, visual Retina/resize assessment, IDE use, Windows/Linux, and browser integration remain unverified as recorded above.

### Unified triangle drawing validation (2026-10-20)

The native preset built all three examples, and the temporary engine-only verification project under `build/opengl-focus-check/` built against the single `Renderer::drawTriangles(VertexArray)` API. Focused native checks ran from `/private/tmp` with approved display access. Indexed arrays used their stored index counts automatically; arrays without indices used their stored vertex counts. The checks verified expected pixels for both paths, geometry/program sharing, cached count/maximum metadata after caller index data changed, invalid referenced vertices, incomplete/uninitialized geometry, and foreign-context rejection. Buffer/VAO layout failures, initialization retry, binding preservation, and injected native draw errors passed after removing count/offset arguments and the CPU index copy.

Resource/lifecycle checks passed, including frame failure without presentation and VAO/buffer/texture cleanup before context/window/video teardown. The actual quad rendered three frames in two runs, handled synthetic Escape/close exits, and retained framebuffer hash `1129920399774065595`. No screenshots were captured. Local links/images/anchors, fences, source boundaries, and whitespace checks passed. Earlier validation sections describe prior APIs; indexed/sequential selection is now automatic and public subranges are absent. Existing manual/platform limitations remain unchanged.

### Post-build asset copying validation (2026-10-20)

The native preset configured and built all three examples after removing the separate asset target. An incremental quad build left a deliberately stale copied shader untouched; a clean rebuild relinked the example and restored all four runtime assets to their source bytes. The asset helper was absent from CMake's target list, all three executables were rebuilt, and SDL framework symlinks remained intact. Documentation links and whitespace checks passed. No rendering code changed; runtime/visual checks were not repeated.
