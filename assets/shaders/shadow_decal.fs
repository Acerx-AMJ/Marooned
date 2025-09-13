#version 330
in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;
// New: 0..1 (or higher if you want)
uniform float shadowStrength;  // e.g. 0.25 for trees, 0.6 for enemies

void main() {
    vec4 texColor = texture(texture0, fragTexCoord);
    if (texColor.a < 0.1) discard;

    float a = texColor.a * shadowStrength;
    finalColor = vec4(0.0, 0.0, 0.0, a);
}
