#version 330

in vec3 fragNormal;
in vec3 fragPosition;

uniform vec3 lightDir;

out vec4 finalColor;

void main()
{
    vec3 normal = normalize(fragNormal);
    float rawLight = dot(normal, -lightDir);

    // Toon step lighting (3 shades)
    float step1 = step(0.9, rawLight);           // bright highlight
    float step2 = step(0.5, rawLight);           // midtone
    float toonLight = step1 * 1.0 + (step2 - step1) * 0.6 + (1.0 - step2) * 0.3;

    vec3 baseColor = vec3(0.2, 0.8, 0.3); // your grass green
    vec3 litColor = baseColor * toonLight;

    finalColor = vec4(litColor, 1.0);
}
