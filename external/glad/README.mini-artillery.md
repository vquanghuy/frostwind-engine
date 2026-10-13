# GLAD dependency record

- **Version:** GLAD 2.0.8, generated on 2026-10-13 at 03:52:31 (timestamp from the generated header).
- **Origin:** [GLAD web generator](https://gen.glad.sh/); files supplied by the project owner. [Upstream release](https://github.com/Dav1dde/glad/tree/v2.0.8).
- **Settings:** C/C++, OpenGL 3.3 Core, zero extensions, all optional features disabled. No internal loader; runtime integration uses SDL_GL_GetProcAddress.
- **License:** Generated files declare `(WTFPL OR CC0-1.0) AND Apache-2.0`. Preserve those declarations and the Khronos notice in `khrplatform.h`. [LICENSE.txt](LICENSE.txt) is the unmodified upstream v2.0.8 license inventory; the generator's MIT license does not replace the generated files' license declarations.
- **Local patches:** None.
- **Imported files:** `include/glad/gl.h`, `include/KHR/khrplatform.h`, and `src/gl.c`. No generator source or prebuilt binary is included.

## Integration

`cmake/GLAD.cmake` compiles `src/gl.c` as C99 into the native static target `GLAD::GLAD`, supplying its public include directory. The executable links this target. No downloads or generation occur during CMake configuration. Browser builds exclude GLAD and will use Emscripten's GLES3 headers. Native context creation and function loading are implemented in [RFC 0002](../../docs/rfcs/0002-graphics-integration.md).

## Updates

Use the [saved generator settings](https://gen.glad.sh/#generator=c&api=gl%3D3.3&profile=gl%3Dcore%2Cgles1%3Dcommon) with no extensions or options selected. The inactive `gles1` profile does not request a GLES1 API. Replace only the generated include and source files, preserve licenses, and update this record with the version, timestamp, settings, and checksums. Online specifications may change; retain the checked-in output as the pinned copy rather than assuming regeneration will be byte-identical.

Run `cmake --preset native-debug` and `cmake --build --preset native-debug --parallel` after updates. Launch the application and verify the reported OpenGL version, GLAD loading, and textured quad.

## SHA-256 of imported files

```text
8462620e59c148e8272893ac430c69f74cd2e4552f94182a425f382d2f63a707  include/glad/gl.h
7b1e01aaa7ad8f6fc34b5c7bdf79ebf5189bb09e2c4d2e79fc5d350623d11e83  include/KHR/khrplatform.h
279b14b388d27c58688655b0211f25109273294033462cc0b07228f1fa6be4a6  src/gl.c
cbb325cd4ac5bd06717ffa34a322b984f9d285e4c67c8ca1b7380d6ef08c2a77  LICENSE.txt
```
