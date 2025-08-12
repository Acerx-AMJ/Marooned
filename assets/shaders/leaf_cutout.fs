// leaf_cutout.fs
#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;   // raylib binds your material texture here
uniform float alphaCutoff;    // 0.4–0.6 is typical

out vec4 finalColor;

void main() {
    vec4 tex = texture(texture0, fragTexCoord) * fragColor;

    // Hard cut — depth writes happen only for kept pixels
    if (tex.a < alphaCutoff) discard;

    finalColor = tex;
    //finalColor = vec4(1.0, 0.0, 0.0, 1.0); // solid red
}
