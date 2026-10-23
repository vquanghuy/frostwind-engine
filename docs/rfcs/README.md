# RFCs

Use RFCs to propose significant architecture changes and record the reasoning behind project decisions.

## Process

1. Copy [template.md](template.md) into the next sequential filename, such as `0001-project-architecture.md`. Reserve 9990–9999 for deferred research so it sorts after active RFCs; exclude that range from normal sequential numbering. The next regular RFC is currently 0010.
2. Start with **Draft**, explain the problem and alternatives, and record unresolved questions.
3. Mark the RFC **Accepted** once the decision is agreed. Mark it **Implemented** when the code fulfills the decision.
4. Use **Deferred research** for preserved proposals outside the active direction; use **Rejected** or **Superseded** when appropriate. A superseded RFC links to its replacement; retain the original for history.
5. Add the RFC to the index below and update implementation documentation when behavior changes.

## Index

| RFC | Status |
| --- | --- |
| [0001: Local third-party dependencies](0001-local-dependencies.md) | Accepted |
| [0002: Graphics integration](0002-graphics-integration.md) | Implemented |
| [0003: Frostwind engine and examples](0003-frostwind-engine-and-examples.md) | Implemented |
| [0004: Frame pacing](0004-frame-pacing.md) | Implemented |
| [0005: 2D camera, keyboard/mouse input, and sprites for Asteroids](0005-camera-keyboard-and-sprites.md) | Draft |
| [0006: OpenGL-focused rendering](0006-opengl-focused-rendering.md) | Implemented |
| [0007: Textured cube, perspective camera, and elapsed-time updates](0007-textured-cube-and-perspective-camera.md) | Implemented |
| [0008: Basic lighting theory in Textured Cube](0008-basic-lighting.md) | Implemented |
| [0009: Materials, meshes, models, and shader composition](0009-materials-meshes-and-shader-composition.md) | Draft |
| [9998: WebGPU migration research](9998-webgpu-migration-research.md) | Deferred research |
| [9999: Multi-backend rendering research](9999-multi-backend-rendering-research.md) | Deferred research |
