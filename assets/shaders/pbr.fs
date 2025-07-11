#version 330

in vec3 fragPos;
in vec3 normal;
in vec2 texCoord;
in vec3 tangent;
in vec3 bitangent;

out vec4 finalColor;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

void main() {
    // Fetch base color
    vec3 albedo = texture(albedoMap, texCoord).rgb;

    // Normal mapping
    vec3 n = normalize(normal);
    vec3 t = normalize(tangent);
    vec3 b = normalize(bitangent);
    mat3 TBN = mat3(t, b, n);
    vec3 norm = texture(normalMap, texCoord).rgb;
    norm = normalize(norm * 2.0 - 1.0);
    norm = normalize(TBN * norm);

    // Metallic and roughness
    float metallic = texture(metallicMap, texCoord).r;
    float roughness = texture(roughnessMap, texCoord).r;

    // Ambient occlusion
    float ao = texture(aoMap, texCoord).r;

    // Lighting calculations (simple PBR approximation)
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Fake specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 halfway = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfway), 0.0), 16.0 * (1.0 - roughness));
    vec3 specular = spec * lightColor * metallic;

    vec3 color = (albedo * diffuse + specular) * ao;

    finalColor = vec4(color, 1.0);
}
