#version 330 core

in vec2 vUv;
uniform sampler2D uTexture;
out vec4 fragColor;

void main()
{
    fragColor = texture(uTexture, vUv);
}
