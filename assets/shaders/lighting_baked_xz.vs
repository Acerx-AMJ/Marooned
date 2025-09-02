#version 330

// Raylib default attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Raylib default uniforms
uniform mat4 mvp;
uniform mat4 matModel;

out vec2 vUV;
out vec3 vWorldPos;
out vec4 vColor;

void main() {
    gl_Position = mvp * vec4(vertexPosition, 1.0);

    vUV       = vertexTexCoord;
    vWorldPos = (matModel * vec4(vertexPosition, 1.0)).xyz; // for XZ lookup
    vColor    = vertexColor;
}
