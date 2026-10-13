# Project structure

The root [README](../README.md#project-structure) describes application modules. This document defines dependency packaging and platform integration. Only the macOS SDK is supplied and validated; Windows and Linux paths describe the prepared integration for later verification.

## Platform SDK layout

```text
external/sdl3/
├── LICENSE.txt
├── README.mini-artillery.md
├── macos-universal/                 # Present: official upstream framework
│   └── SDL3.framework/
│       ├── Headers -> Versions/Current/Headers
│       ├── Resources -> Versions/Current/Resources
│       ├── SDL3 -> Versions/Current/SDL3
│       └── Versions/
├── windows-x64/                     # SDK pending; MSVC x64 only
│   ├── include/SDL3/               # Matching public headers
│   ├── lib/SDL3.lib                # DLL import library, not a static library
│   └── bin/SDL3.dll                # Runtime library
└── linux-x64/                       # SDK pending; distribution to be selected
    ├── include/SDL3/               # Matching public headers
    └── lib/
        ├── libSDL3.so.0            # Runtime SONAME; may be a symlink
        └── <versioned binary>      # Required if the SONAME path is a symlink
```

Windows and Linux SDK directories will be created when actual dependency files are imported. Use SDL3 3.4.18 consistently and preserve licenses. Record each package's source, checksum, architecture, ABI requirements, and any local modifications in the dependency record. No upstream source tree is required for normal application builds.

## Header lookup

Application source uses the same include on every target:

```cpp
#include <SDL3/SDL.h>
```

| Platform | Compiler search configuration | Header location |
| --- | --- | --- |
| macOS | Framework search path supplied by the imported framework target | `SDL3.framework/Headers/SDL.h` |
| Windows | Include directory `windows-x64/include` | `include/SDL3/SDL.h` |
| Linux | Include directory `linux-x64/include` | `include/SDL3/SDL.h` |

`cmake/SDL3.cmake` selects the platform SDK and exposes `SDL3::SDL3`. Linking that target supplies the platform's header and library settings. CMake-aware editor tools can use those same settings for code completion.

## Build and runtime layout

| Preset | Build directory | Runtime dependency placement | Status |
| --- | --- | --- | --- |
| `native-debug` | `build/native/` | `Frameworks/SDL3.framework` beside the executable | macOS arm64 validated |
| `windows-debug` | `build/windows/` | `SDL3.dll` beside `Debug/mini-artillery.exe` | SDK pending; unverified |
| `linux-debug` | `build/linux/` | `lib/` beside the executable | SDK pending; unverified |

Presets are visible only on their matching host OS. Windows selects Visual Studio 2022 and x64; Linux selects Unix Makefiles. macOS runtime lookup uses `@loader_path/Frameworks`, and the prepared Linux path uses `$ORIGIN/lib`. Keep the copied runtime dependency directories with the executable.

CMake does not download SDKs or fall back to system SDL installations. Missing dependency files produce an error describing the expected paths. Browser support requires its own WASM-compatible SDL library and is not configured by these native branches.

See [development instructions](development.md) for build commands and deferred platform checks.
