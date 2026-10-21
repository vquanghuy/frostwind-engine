# GLAD dependency record

- **Version:** GLAD 2.0.8, generated on 2026-10-21 at 03:27:35 (timestamp from the generated header).
- **Origin:** [GLAD web generator](https://gen.glad.sh/); files supplied by the project owner. [Upstream release](https://github.com/Dav1dde/glad/tree/v2.0.8).
- **Settings:** C/C++, OpenGL 3.3 Core, 18 explicitly selected extensions, all generator options disabled. No internal loader; runtime integration uses SDL_GL_GetProcAddress.
- **License:** Generated files declare `(WTFPL OR CC0-1.0) AND Apache-2.0`. Preserve those declarations and the Khronos notice in `khrplatform.h`. [LICENSE.txt](LICENSE.txt) is the unmodified upstream v2.0.8 license inventory; the generator's MIT license does not replace the generated files' license declarations.
- **Local patches:** None.
- **Imported files:** `include/glad/gl.h`, `include/KHR/khrplatform.h`, and `src/gl.c`. No generator source or prebuilt binary is included.

## Integration

`cmake/GLAD.cmake` compiles `src/gl.c` as C99 into the native static target `GLAD::GLAD`, supplying its public include directory. The static `frostwind` engine target links it privately; GLAD headers remain internal to engine implementations, while the native archive is linked into example executables. No downloads or generation occur during CMake configuration. Browser builds exclude GLAD and will use Emscripten's GLES3 headers. Native context creation and function loading are implemented in [RFC 0002](../../docs/rfcs/0002-graphics-integration.md).

## Selected extensions

The exact selection is:

```text
GL_ARB_base_instance
GL_ARB_buffer_storage
GL_ARB_compute_shader
GL_ARB_debug_output
GL_ARB_draw_indirect
GL_ARB_internalformat_query
GL_ARB_internalformat_query2
GL_ARB_multi_draw_indirect
GL_ARB_shader_draw_parameters
GL_ARB_shader_image_load_store
GL_ARB_shader_storage_buffer_object
GL_ARB_texture_compression_bptc
GL_ARB_texture_cube_map_array
GL_ARB_texture_filter_anisotropic
GL_ARB_texture_storage
GL_EXT_texture_compression_s3tc
GL_EXT_texture_filter_anisotropic
GL_KHR_debug
```

See [RFC 0006's area/purpose table](../../docs/rfcs/0006-opengl-focused-rendering.md#glad-extension-selection)
for the rationale. Definitions and function loading do not guarantee runtime
support or enable the selected features. Texture2D uses generated anisotropy
constants and support flags; the remaining extension APIs are reserved for
future example-driven work, with their own capability/prerequisite checks.
The OpenGL 3.3 Core baseline remains unchanged.

## Updates

Use the [saved generator settings](https://glad.sh/#api=gl%3Acore%3D3.3&extensions=GL_ARB_base_instance%2CGL_ARB_buffer_storage%2CGL_ARB_compute_shader%2CGL_ARB_debug_output%2CGL_ARB_draw_indirect%2CGL_ARB_internalformat_query%2CGL_ARB_internalformat_query2%2CGL_ARB_multi_draw_indirect%2CGL_ARB_shader_draw_parameters%2CGL_ARB_shader_image_load_store%2CGL_ARB_shader_storage_buffer_object%2CGL_ARB_texture_compression_bptc%2CGL_ARB_texture_cube_map_array%2CGL_ARB_texture_filter_anisotropic%2CGL_ARB_texture_storage%2CGL_EXT_texture_compression_s3tc%2CGL_EXT_texture_filter_anisotropic%2CGL_KHR_debug&generator=c&options=) with the 18 extensions listed above and all generator options disabled. Replace only the generated include and source files, preserve licenses, and update this record with the version, timestamp, settings, and checksums. Online specifications may change; retain the checked-in output as the pinned copy rather than assuming regeneration will be byte-identical.

Run `cmake --preset native-debug` and `cmake --build --preset native-debug --parallel` after updates. Launch `build/native/bin/frostwind-textured-quad/frostwind-textured-quad` and verify the reported OpenGL version, GLAD loading, and textured quad.

## SHA-256 of imported files

```text
8af1ca4039c075a552d27734d4d01450e1e5c274e809580e44e461ed17325815  include/glad/gl.h
7b1e01aaa7ad8f6fc34b5c7bdf79ebf5189bb09e2c4d2e79fc5d350623d11e83  include/KHR/khrplatform.h
097ba0fb1acc930511f85fb1c9c3c06ffcfdec7d09d0ac89386a69dcfe802aed  src/gl.c
cbb325cd4ac5bd06717ffa34a322b984f9d285e4c67c8ca1b7380d6ef08c2a77  LICENSE.txt
```
