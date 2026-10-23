# Basic lighting theory generation record

Generated with the built-in OpenAI imagegen tool on 2026-10-21 for RFC 0008.
The PNG supplies the detailed crate panels, not an engine screenshot,
runtime asset, or numerically evaluated rendering. No reference images or
third-party artwork were supplied for the initial generation.

## Replacement with precise vector geometry

The generated PNG's reflection geometry was inaccurate. Two further imagegen
edits failed to preserve equal reflection angles, so neither was adopted.
The RFC embeds `basic-lighting-theory.svg`, which combines the detailed
generated crate illustrations with diagrams constructed directly from vector
coordinates. The top 535 pixels are cropped without resizing to `crate-lighting-panels.png`
and embedded as a data URI inside the SVG; the inaccurate generated vector
panels are excluded. The lower diagrams remain native SVG paths, arrows, arcs,
and text. The SVG requires no external images or downloaded fonts.

For the reflection panel, screen-space point P is (875, 620), the incoming
ray starts at (725, 470), and R ends at (1025, 470). Both diagonals therefore
make exactly 45 degrees with vertical N. In world coordinates, normalized
I = (1, -1) / sqrt(2), N = (0, 1), and
R = I - 2 * dot(N, I) * N = (1, 1) / sqrt(2).
The independent camera vector points from P toward (967, 435) on screen.
Angle arcs are centered on P and terminate on the incident/reflected lines,
not the camera vector.

## Initial prompt

Use case: scientific-educational
Asset type: landscape educational illustration embedded in a C++ OpenGL lighting RFC.
Create a polished, clear lighting theory infographic on a light ivory background with dark navy readable typography. Title "Basic lighting on a textured cube"; subtitle "One directional light • world-space Phong shading". Wide landscape, approximately 3:2. A designed textbook figure, generous spacing, no branding.
TOP HALF: Four equal panels with identical isometric cubes carrying a simple warm brown wooden crate texture, same orientation and scale, identical fixed light from upper left and camera direction. Labels exactly "Ambient", "Diffuse", "Specular", "Combined". Ambient cube dim uniformly visible with texture; diffuse cube brightness depends on face orientation with dark faces away from light and no highlight; specular cube black with a compact white highlight on the lit face and no texture color; combined cube shows crate texture with ambient + diffuse + restrained white highlight. Put plus, plus, equals between panels. No floor shadows or cast shadows. Captions respectively "Constant fill", "Surface faces light", "View-dependent highlight", "Sum of all three".
BOTTOM HALF: two clearly separated panels. Left panel titled "Diffuse: surface orientation". Accurate 2D flat horizontal surface at bottom, a point P on it, normal N arrow straight upward, L arrow from P toward upper left, an incoming light ray arrow parallel to -L pointing down-right toward P, with that ray spatially separated from L. A small angle arc between N and L labelled theta. Caption "L = -normalize(lightDirection)" and "diffuse = max(dot(N, L), 0)". Label N "surface normal". Explicit arrowheads and no arrow direction ambiguity.
Right panel titled "Specular: viewing direction". Flat horizontal surface with P, N straight up, incident light ray from upper left toward P, reflected R from P toward upper right at equal angle to N, and distinct V from P to a small camera at upper right slightly steeper than R. Small angle arc between R and V. Caption "R = reflect(-L, N)" and "specular follows max(dot(R, V), 0)". V points from surface to camera, R points away from surface. Label the incoming arrow "light ray". Color N teal, incoming rays and L amber, R coral, V blue. Keep formulas exact and readable.
Footer exact "Theory illustration — lighting is not implemented yet". Do not present this as an engine screenshot. No point light bulb, no light-distance falloff, no shadows, no extra equations. Prioritize accurate vector directions and clean readable labels.

## Correction prompt

The initial generated image was supplied as the edit target to straighten the
camera-direction vector in the specular panel.

Edit this lighting theory infographic. Preserve all layout, typography, text, cubes, colors, and all other elements. Correct only the vector geometry in the bottom-right specular panel: replace the bent blue V arrow with ONE straight blue arrow segment starting exactly at P and pointing directly to the camera. V is a straight surface-to-camera direction. Place the V label beside that straight arrow. Preserve the straight coral R arrow from P upward-right and the upward teal N arrow. Draw the alpha angle arc between the straight R and straight V rays with its center at P; it may be a small arc near P. The incoming orange ray must be a single straight diagonal down-right toward P, and R must be its mirror reflection about the vertical N (equal incident and reflected angles). Remove the existing kink and the displaced angle arc entirely. No other changes.
