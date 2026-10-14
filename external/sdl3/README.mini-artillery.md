# SDL3 dependency record

- **Version:** 3.4.18
- **Upstream:** https://github.com/libsdl-org/SDL
- **Official package:** https://github.com/libsdl-org/SDL/releases/download/release-3.4.18/SDL3-3.4.18.dmg
- **DMG SHA-256:** `c1adf3da91108dbc3e4205da5dacebc203085d283fa89e86eb2c889abd03a853`
- **Upstream package slice:** `SDL3.xcframework/macos-arm64_x86_64/SDL3.framework`.
- **Repository location:** `macos-universal/SDL3.framework`.
- **Framework binary SHA-256:** `626285366f11d66a70912cc767f00330b985e618b696aee1cfe0ca2f41357f4c`
- **License:** zlib; see `LICENSE.txt`.
- **Included files:** Complete macOS framework slice, including headers, resources, signatures, and symlinks; upstream license; this record.
- **Local upstream modifications:** None. Other XCFramework platform slices and separate debug symbols are excluded.
- **Binary origin:** Official upstream prebuilt binary. No local SDL compilation.
- **Architecture:** Universal arm64 and x86_64.
- **Framework deployment targets:** macOS 11.0 for arm64 and 10.13 for x86_64, inspected from the supplied Mach-O binary. Application deployment settings remain separate.
- **Linkage:** Dynamic framework; install name `@rpath/SDL3.framework/Versions/A/SDL3`.
- **Integration:** Imported `SDL3::SDL3` target in `cmake/SDL3.cmake`. `cmake/ToyboxExample.cmake` copies the framework into each example’s output directory (currently `build/native/bin/toybox-textured-quad/Frameworks/`); the executable uses `@loader_path/Frameworks` as its runtime search path.

Headers live inside `SDL3.framework/Headers` and are made available to the compiler through the framework search path. There is no separate header copy or static archive. This framework cannot be used for WASM builds.

## Updating

1. Download the selected official macOS DMG and compare its SHA-256 with the release asset metadata.
2. Mount the image read-only and copy only its macOS framework slice, preserving symlinks and resources.
3. Replace the framework and license together, then update the version, source URL, checksums, and binary details in this record.
4. Unmount the disk image.
5. Configure and rebuild the application. Verify `otool -L` reports the SDL3 framework dependency and launch the executable to confirm the copied framework loads successfully.

No installation into `/Library/Frameworks` or `~/Library/Frameworks` is required.

## Additional platform SDKs

Windows x64 and Linux x86_64 CMake branches are prepared but unverified. Their headers and binaries are not included yet. See [project structure](../../docs/project-structure.md) for the required paths and [development instructions](../../docs/development.md) for deferred verification. Each supplied SDK must include its own origin, checksum, license, architecture, and compatibility details here before use.
