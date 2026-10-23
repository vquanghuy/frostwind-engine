#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec2 vUv;
out vec3 vWorldPosition;
out vec3 vWorldNormal;

void main()
{
    vec4 worldPosition = uModel * vec4(aPosition, 1.0);
    vWorldPosition = worldPosition.xyz;
    // Inverse transpose keeps normals perpendicular under nonuniform scale.
    vWorldNormal = transpose(inverse(mat3(uModel))) * aNormal;
    vUv = aUv;
    gl_Position = uProjection * uView * worldPosition;
}
