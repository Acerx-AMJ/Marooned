#version 330
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;  // raylib default
layout(location = 2) in vec3 vertexNormal;    // (unused here, fine to keep)

uniform mat4 mvp;
uniform mat4 matModel;

out vec2 fragUV;
out vec3 fragPosition;

void main() {
    vec4 worldPos   = matModel * vec4(vertexPosition, 1.0);
    fragPosition    = worldPos.xyz;
    fragUV          = vertexTexCoord;
    gl_Position     = mvp * vec4(vertexPosition, 1.0); // raylib mvp already includes model
}
