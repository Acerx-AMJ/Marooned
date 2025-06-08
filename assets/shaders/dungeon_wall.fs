#version 330

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

uniform sampler2D texture0;
uniform vec3 lightPos;
uniform vec3 cameraPos;

out vec4 finalColor;

void main() {
    vec3 texColor = texture(texture0, fragTexCoord).rgb;

    float dist = distance(fragPosition, lightPos);
    float brightness = clamp(1.0 - (dist / 800.0), 0.2, 1.0);

    vec3 lightDir = normalize(lightPos - fragPosition);
    float diffuse = max(dot(fragNormal, lightDir), 0.2);

    vec3 color = texColor * brightness * diffuse;
    finalColor = vec4(color, 1.0);
    //finalColor = vec4(1.0, 1.0, 1.0, 1.0); // fully white
}
