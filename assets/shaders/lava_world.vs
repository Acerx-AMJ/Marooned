// lava_world.vs
#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;

out vec3 vWorldPos;
out vec4 vColor;

void main() {
    vec4 wp = matModel * vec4(vertexPosition, 1.0);
    vWorldPos = wp.xyz;        // pass world position to fragment
    vColor = vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
