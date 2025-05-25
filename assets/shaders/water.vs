#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;

out vec3 fragPosition;
out vec2 fragTexCoord;

uniform mat4 mvp;
uniform mat4 model;

void main()
{
    fragPosition = vec3(model * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
