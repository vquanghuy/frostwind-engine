# Documentation images

## Toybox logo

[`toybox-logo.png`](toybox-logo.png) is the original Toybox logo, previously used in the root [README](../../README.md). It was generated with OpenAI imagegen on 2026-10-14 and selected by the project owner. The design shows an open wooden toy chest containing a sword, toy tank, and ball beside a colorful Toybox wordmark. It is a branding asset, not a gameplay screenshot.

## Frostwind logo

[`frostwind-logo.png`](frostwind-logo.png) is the Frostwind logo used in the root README. It was generated with OpenAI imagegen on 2026-10-14 using the direction “wind-carved ice crystal, glacier-blue facets, restrained silver Frostwind wordmark, transparent background.” The original Toybox logo is preserved; the engine has since been renamed to Frostwind.

## Lighting theory

[`lighting/basic-lighting-theory.svg`](lighting/basic-lighting-theory.svg)
illustrates the ambient, diffuse, and specular contributions proposed in
[RFC 0008](../rfcs/0008-basic-lighting.md), including normal, light, reflection,
and viewing directions. Combines detailed imagegen crate panels with SVG
vector diagrams constructed on 2026-10-21, so reflection geometry has exact
equal angles about the normal. The cropped `crate-lighting-panels.png` is embedded in the SVG; its inaccurate generated vector diagrams
are excluded. The SVG is self-contained and needs no external image loading.
The [illustration record](lighting/basic-lighting-theory-prompt.md) preserves
its prompts and the correction rationale. This is a qualitative theory
illustration, not a screenshot, measured shader output, or runtime asset.
Example-owned cube lighting is implemented; the illustration remains conceptual, not an engine capture. The owner-supplied lit capture is recorded below.

## Milestone captures

`milestones/` preserves project screenshots, including the original Mini Artillery identity, separate from the third-party references below. Historical images and captions are retained without relabeling. Screenshots appear directly in the [changelog](../../CHANGELOG.md). M001 and M002 were supplied manually from macOS window screenshots on 2026-10-13. M002 shows the native OpenGL checkerboard quad. [`milestones/m003-file-texture.png`](milestones/m003-file-texture.png) shows the Toybox textured-quad example loading the generated crate PNG. M003 was supplied by the owner from `Screenshot 2026-10-14 at 14.15.24.png` on macOS Apple M1 Pro on 2026-10-14 and is preserved as a historical capture.

[`milestones/m004-frostwind-textured-quad.png`](milestones/m004-frostwind-textured-quad.png) shows the same example after the Frostwind rename. The owner supplied it from `Screenshot 2026-10-14 at 14.56.04.png` on macOS Apple M1 Pro on 2026-10-14. It is embedded in the changelog and [example README](../../examples/01-textured-quad/README.md); M001–M003 remain unchanged.

[`milestones/m005-frostwind-textured-cube.png`](milestones/m005-frostwind-textured-cube.png)
shows the running Textured Cube example on macOS Apple M1 Pro. The owner supplied
it from Desktop `Screenshot 2026-10-20 at 15.11.17.png`, captured manually on
2026-10-20. The PNG is preserved byte-for-byte without cropping or re-encoding
and retained in the changelog's earlier-capture section with its original caption.

[`milestones/m005-frostwind-textured-cube-updated.png`](milestones/m005-frostwind-textured-cube-updated.png)
is the updated capture supplied from Desktop `Screenshot 2026-10-20 at 15.49.58.png`
on 2026-10-20, showing the running cube on macOS Apple M1 Pro. Its original PNG
is preserved byte-for-byte in the cube README history and the 2026-10-20
changelog entry. The earlier 15:11:17 capture remains preserved.
These images record rendered output, not frame-rate measurements or proof of
interactive checks. M001–M004 and their captions remain unchanged.

[`milestones/textured-cube-lighting-2026-10-22.png`](milestones/textured-cube-lighting-2026-10-22.png)
shows the lit Textured Cube on macOS Apple M1 Pro. The owner supplied Desktop
`Screenshot 2026-10-22 at 21.15.03.png`, captured manually on 2026-10-22 at
21:15:03. The original PNG is preserved byte-for-byte, without cropping or
re-encoding, and appears in the root README preview, cube README, and changelog.
It shows the example after lighting implementation and ambient/specular tuning;
it does not establish interactive behavior or performance. Earlier unlit
captures and their captions remain preserved. This adds no release or numbered
milestone designation.

## Example concepts and references

Planning artwork, generated sprite prototypes, and attribution live with the example that owns them:

- [Asteroids concept and reference inventory](../../examples/03-asteroids/docs/assets/README.md)
- [Mini Artillery concept and reference inventory](../../examples/04-mini-artillery/docs/assets/README.md)

Each example README embeds its preferred concept and links older versions. Concept images are labeled as design references and are separate from runtime textures and their future `assets/LICENSES.md` records. Historical milestone PNGs, captions, and branding remain here unchanged.
