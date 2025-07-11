#version 330

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
in vec4 vertexTangent;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

out vec3 fragPos;
out vec3 normal;
out vec2 texCoord;
out vec3 tangent;
out vec3 bitangent;

void main() {
    fragPos = vec3(matModel * vec4(vertexPosition, 1.0));
    normal = mat3(matNormal) * vertexNormal;
    texCoord = vertexTexCoord;
    tangent = mat3(matModel) * vertexTangent.xyz;
    bitangent = cross(normal, tangent) * vertexTangent.w;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
