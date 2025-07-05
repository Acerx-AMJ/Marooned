#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D sceneTexture;
uniform sampler2D sceneDepth;

uniform float cameraNear;
uniform float cameraFar;

uniform float fogNear;
uniform float fogFar;

uniform float fogAmount;

float LinearizeDepth(float z, float near, float far)
{
    return (near * far) / (far - z * (far - near));
}

void main()
{
    // 1️⃣ Base scene color
    vec3 sceneColor = texture(sceneTexture, fragTexCoord).rgb;

    // 2️⃣ Sample depth
    float rawDepth = texture(sceneDepth, fragTexCoord).r;

    // Discard skybox (no fog)
    if (rawDepth >= 1.0)
    {
        finalColor = vec4(sceneColor, 1.0);
        return;
    }

    // 3️⃣ Linearize depth to view space units
    float linearDepth = LinearizeDepth(rawDepth, cameraNear, cameraFar);

    // 4️⃣ Compute fog factor
    fogFactor = smoothstep(fogNear, fogFar, linearDepth);

    // 5️⃣ Scale by user-controlled amount
    fogFactor = clamp(fogFactor * fogAmount, 0.0, 1.0);

    // 6️⃣ Mix with black
    vec3 foggedColor = mix(sceneColor, vec3(0.0), fogFactor);




    finalColor = vec4(foggedColor, 1.0);
    //finalColor = vec4(1.0, 0.0, 0.0, 1.0);
}
