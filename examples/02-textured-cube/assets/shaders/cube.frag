#version 330 core

in vec2 vUv;
in vec3 vWorldPosition;
in vec3 vWorldNormal;

uniform sampler2D uTexture;
uniform vec3 uCameraPosition;
uniform vec3 uLightDirection;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;
uniform vec3 uSpecularColor;
uniform float uShininess;

out vec4 fragColor;

void main()
{
    vec4 surface = texture(uTexture, vUv);
    vec3 normal = normalize(vWorldNormal);
    // The supplied direction follows the rays; lighting needs surface-to-light.
    vec3 lightDirection = -normalize(uLightDirection);
    float diffuseFactor = max(dot(normal, lightDirection), 0.0);
    vec3 ambient = uAmbientColor * surface.rgb;
    vec3 diffuse = diffuseFactor * uLightColor * surface.rgb;

    vec3 specular = vec3(0.0);
    vec3 toCamera = uCameraPosition - vWorldPosition;
    float cameraDistanceSquared = dot(toCamera, toCamera);
    if (diffuseFactor > 0.0 && cameraDistanceSquared > 0.0) {
        vec3 viewDirection = toCamera * inversesqrt(cameraDistanceSquared);
        vec3 reflectedDirection = reflect(-lightDirection, normal);
        float specularFactor = pow(max(dot(reflectedDirection, viewDirection), 0.0), uShininess);
        specular = uSpecularColor * specularFactor * uLightColor;
    }

    fragColor = vec4(ambient + diffuse + specular, surface.a);
}
