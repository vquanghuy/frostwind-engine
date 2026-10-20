# GLM integration

- Upstream: https://github.com/g-truc/glm
- Release: **1.0.3**; tag commit `8d1fd52e5ab5590e2c81768ace50c72bae28f2ed`.
- Archive: https://codeload.github.com/g-truc/glm/tar.gz/refs/tags/1.0.3
- Archive SHA-256: `6775e47231a446fd086d660ecc18bcd076531cfedd912fbd66e576b118607001`.
- Imported: complete upstream `glm/` directory and `copying.txt`; no local patches.
- License: upstream dual license in [copying.txt](copying.txt); use under its MIT option.

`cmake/GLM.cmake` supplies the local header-only `GLM::GLM` interface target.
It propagates through `frostwind` because camera headers expose GLM types.
`GLM_FORCE_RADIANS` is consistent across consumers; no aligned, experimental,
left-handed, or zero-to-one depth configuration is enabled. Camera functions
explicitly select right-handed transforms and OpenGL negative-one-to-one depth.
There is no supplied binary or build toolchain requirement beyond the project's
C++17 compiler. Configuration and builds never retrieve dependencies.

To update, choose an exact upstream release/commit, download its source archive
under ignored `build/`, record its checksum, and replace only the imported
upstream files. Preserve the license and update this record. Build the engine
and all examples, then verify camera math and rendering before accepting it.
