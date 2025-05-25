#version 330

in vec3 fragNormal;
in vec3 fragPosition;

uniform vec3 lightDir;

out vec4 finalColor;

void main()
{

    vec3 baseColor = vec3(0.0, 0.4, 0.1);   // green terrain
    float diffuse = clamp(dot(normalize(fragNormal), -lightDir), 0.0, 1.0);
    float ambient = 0.01;

    vec3 lightColor = vec3(1.0, 0.95, 0.8); // warm sunlight


    finalColor = vec4((baseColor * lightColor) * diffuse + ambient, 1.0);

    //finalColor = vec4(baseColor * diffuse + ambient, 1.0);

}
