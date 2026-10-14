# stb_image

- Version: **2.28**, as identified by the supplied header.
- Upstream: [nothings/stb](https://github.com/nothings/stb), `stb_image.h`.
- Imported on 2026-10-14 from the owner's `LearnOpenGL/External/stb/stb_image.h`, last changed there in commit `a9da7e966620267faffb7e3ff151ccbb88b991b3`.
- SHA-256 of the unmodified header: `38e08c1c5ab8869ae8d605ddaefa85ad3fea24a2964fd63a099c0c0f79c70bcc`.
- The upstream commit of this supplied copy was not recorded; its version and exact content hash pin this import.
- Imported files: `stb_image.h`; `LICENSE.txt` reproduces the header's dual MIT/public-domain license notice. Toybox uses the MIT option.
- Local patches: none.

The engine privately includes this header and compiles its implementation exactly once in `engine/src/rendering/StbImage.cpp`. Configuration enables PNG and JPEG only and disables stb's filename-based I/O. `Texture2D` reads files through C++17 filesystem paths and decodes from memory. `Application::onInit` calls `Texture2D::initializeSupport()` after window initialization and before example initialization. That rendering-owned setup sets `stbi_set_flip_vertically_on_load(true)` and caches `GL_MAX_TEXTURE_SIZE` for the single current context; application code does not include stb_image. Texture loads reuse this engine-wide bottom-left convention without resetting the global setting. Loading currently runs on the context thread; revisit decoder settings if asynchronous loading is introduced. stb_image is compiled into the static engine library with the project's native toolchain; no supplied binary or runtime library is required.

For updates, deliberately replace the header with a selected upstream version, preserve its license, update this record and checksum, then build and validate file failures, PNG/JPEG decoding, channel conversion, orientation, and upload/cleanup. CMake never downloads or updates it.
