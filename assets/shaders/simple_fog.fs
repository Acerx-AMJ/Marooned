#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D sceneTexture;

uniform vec3 fogColor;      // e.g., vec3(0.1, 0.1, 0.2)
uniform float fogStrength;  // 0.0 to 1.0
uniform float verticalFade; // 0.0 = no fade, 1.0 = full fade at top of screen

#define MAX_LIGHTS 8
uniform vec2 lightPositions[MAX_LIGHTS];
uniform int numLights;

void main() {
    vec3 color = texture(sceneTexture, fragTexCoord).rgb;

    float lightInfluence = 0.0;

    for (int i = 0; i < numLights; i++) {
        float d = distance(fragTexCoord, lightPositions[i]);
        float contribution = 1.0 - smoothstep(0.0, 0.9, d); // 0.25 = light radius
        lightInfluence += contribution;
    }

    lightInfluence = clamp(lightInfluence, 0.0, 1.0);
    float fogAmount = (1.0 - lightInfluence) * fogStrength;

    // Optional vertical fade (based on Y position on screen)
    float verticalFactor = mix(1.0, fragTexCoord.y, verticalFade);
    fogAmount *= verticalFactor;

    vec3 final = mix(color, fogColor, fogAmount);
    finalColor = vec4(final, 1.0);
}


