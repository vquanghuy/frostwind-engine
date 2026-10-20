#version 330 core
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aUV;
layout(std140) uniform QuadParameters {
    vec4 scaleAndPadding;
};
out vec2 vUV;
void main() {
    vUV = aUV;
    gl_Position = vec4(aPosition * scaleAndPadding.xy, 0.0, 1.0);
}
