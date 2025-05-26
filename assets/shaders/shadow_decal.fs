#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D texture0;

void main() {
    vec4 texColor = texture(texture0, fragTexCoord);
    if (texColor.a < 0.1) discard; // kill transparent pixels (no depth write)

    finalColor = vec4(0.0, 0.0, 0.0, texColor.a * 0.25); // darken ground
}
