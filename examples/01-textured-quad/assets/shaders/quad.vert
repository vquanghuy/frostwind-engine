#version 330 core
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aUV;
uniform vec2 uScale;
out vec2 vUV;
void main() {
    vUV = aUV;
    gl_Position = vec4(aPosition * uScale, 0.0, 1.0);
}
