# RFC 0004: Frame pacing

- **Status:** Implemented
- **Created:** 2026-10-14
- **Supersedes:** None
- **Superseded by:** None

## Problem

The OpenGL-focused design in [RFC 0006](0006-opengl-focused-rendering.md) uses `WindowConfig::vsync`, restoring the setting from the temporary device-owned configuration while retaining the pacing policy below. Zero-sized targets use the remaining configured frame budget, or 16 ms when uncapped; minimized windows retain a separate 16 ms delay. See the [current development guide](../development.md#current-implementation).

VSync alone follows the display refresh rate. When unavailable, the previous 16 ms post-swap delay added to rendering time, producing a variable rate without an explicit FPS cap.

## Design

`ApplicationConfig` groups window settings and `maxFps`, defaulting to 60. Zero disables software pacing. `WindowConfig::vsync` independently requests swap interval 1 or 0, defaulting to enabled. Failure to set the interval is logged; software pacing still applies when configured.

`Application` owns pacing, while `Window` owns presentation and VSync. Measure callback work with `SDL_GetTicksNS`, including drawable queries, rendering, and buffer swap. Round the nanosecond frame interval up and call `SDL_DelayPrecise` only for the remaining budget after successful presentation. Each callback starts a fresh budget, so slow frames and stalls never cause catch-up rendering. SDL remains responsible for callback delivery and events. Minimized windows retain a separate 16 ms idle delay. Zero-sized windows skip rendering and use the remaining configured frame budget, or 16 ms when uncapped.

## Alternatives

- A fixed delay after every swap ignores work already done.
- VSync alone cannot enforce a chosen ceiling on faster displays or when unavailable.
- SDL's callback-rate hint schedules callbacks but can interact with VSync blocking. Explicit elapsed-time budgeting makes the engine's pacing and slow-frame behavior directly testable.

## Risks and limitations

The cap limits callback throughput; it cannot guarantee the requested FPS or perfectly even display presentation. VSync, slow rendering, and OS scheduling can lower the achieved rate. SDL precise waits may briefly busy-wait for accuracy. Non-matching cap/display rates can produce uneven presentation. Low caps delay event delivery while a callback waits. This rendering limiter does not establish fixed-step simulation, determinism, FPS instrumentation, or browser support.

## Validation

Build the native library and example. Exercise controlled timing for fast/slow frames, swap blocking, uncapped mode, stalls, minimized windows, and callback errors/exits. Measure actual callback throughput with the native timer. Verify example initialization, presentation, and teardown. Interactive display smoothness, resize/minimize/restore, and other platforms remain manual checks.

## Decision

Implemented on 2026-10-14 following the owner's request for proper FPS limiting. Keep the timing policy in application coordination without adding a simulation scheduler or a rendering backend abstraction.
