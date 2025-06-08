#version 330

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;

out vec3 fragWorldPos;
out vec3 fragNormal;
out vec2 fragTexCoord;

void main()
{
    vec4 worldPos = model * vec4(vertexPosition, 1.0);
    fragWorldPos = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    fragNormal = normalize(normalMatrix * vertexNormal);

    fragTexCoord = vertexTexCoord;
    gl_Position = projection * view * worldPos;
}
