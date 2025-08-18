#version 330

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D sceneTexture;
uniform vec2 resolution;
uniform float bloomStrength;
uniform vec3 bloomColor;
uniform float vignetteStrength;
uniform float aaStrength;

// Saturation control (post-bloom)
uniform float uSaturation; // 1.0 = neutral, 0 = gray, 1.1–1.25 = punchier

// Helpers
vec3 applySaturation(vec3 rgb, float sat) {
    float luma = dot(rgb, vec3(0.299, 0.587, 0.114));
    return mix(vec3(luma), rgb, sat);
}

// Approx sRGB <-> linear (good enough for post)
vec3 toLinear(vec3 c) { return pow(c, vec3(2.2)); }
vec3 toSRGB (vec3 c) { return pow(c, vec3(1.0/2.2)); }

void main()
{
    vec2 texelSize = 1.0 / resolution;
    vec3 original = texture(sceneTexture, fragTexCoord).rgb;

    // --- Bloom blur pass (5x5 box) ---
    vec3 blur = vec3(0.0);
    float weightSum = 0.0;
    for (int y = -2; y <= 2; y++) {
        for (int x = -2; x <= 2; x++) {
            vec2 offset = vec2(x, y) * texelSize;
            vec3 sample = texture(sceneTexture, fragTexCoord + offset).rgb;
            blur += sample;
            weightSum += 1.0;
        }
    }
    blur /= weightSum;

    // Slightly desaturate the glow to avoid "milky" bloom
    float preBloomSat = 0.95;
    blur = applySaturation(blur, preBloomSat);

    // Tint the blur with bloomColor
    blur *= bloomColor;

    // Mix original and tinted blur
    vec3 result = mix(original, blur, bloomStrength * 0.7);
    result += blur * bloomStrength * 0.3; // additive boost

    // --- Vignette ---
    vec2 centeredCoord = fragTexCoord - vec2(0.5);
    float dist = clamp(length(centeredCoord) / 0.7071, 0.0, 1.0);
    float vignette = smoothstep(0.4, 1.0, dist);
    result = mix(result, vec3(0.0), vignette * vignetteStrength);

    // --- Bootleg AA (in linear space) ---
    // Convert the current result to linear
    vec3 resultLin = toLinear(result);

    // Sample neighborhood in linear
    vec3 aaBlurLin = vec3(0.0);
    float aaWeightSum = 0.0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            vec2 offset = vec2(x, y) * texelSize;
            vec3 sRGB = texture(sceneTexture, fragTexCoord + offset).rgb;
            aaBlurLin += toLinear(sRGB);
            aaWeightSum += 1.0;
        }
    }
    aaBlurLin /= aaWeightSum;

    // Mix AA blur in linear, then go back to sRGB
    resultLin = mix(resultLin, aaBlurLin, aaStrength);
    result = toSRGB(resultLin);

    // Post-bloom saturation (last shaping step before output)
    result = applySaturation(result, uSaturation);

    // Clamp & output
    result = clamp(result, 0.0, 1.0);
    finalColor = vec4(result, 1.0);
}



// #version 330

// in vec2 fragTexCoord;
// out vec4 finalColor;

// uniform sampler2D sceneTexture;
// uniform vec2 resolution;
// uniform float bloomStrength;    // 0.0 = no bloom, 1.0 = max
// uniform vec3 bloomColor;        // e.g., vec3(1.0, 0.7, 0.7) for warm reddish
// uniform float vignetteStrength; // 0.0 = none, 1.0 = strong vignette
// uniform float aaStrength;       // 0.0 = no AA, 1.0 = full AA

// void main()
// {
//     vec2 texelSize = 1.0 / resolution;
//     vec3 original = texture(sceneTexture, fragTexCoord).rgb;
//     vec3 blur = vec3(0.0);
//     float weightSum = 0.0;

//     // --- Bloom blur pass ---
//     for (int y = -2; y <= 2; y++) {
//         for (int x = -2; x <= 2; x++) {
//             vec2 offset = vec2(x, y) * texelSize;
//             vec3 sample = texture(sceneTexture, fragTexCoord + offset).rgb;
//             blur += sample;
//             weightSum += 1.0;
//         }
//     }
//     blur /= weightSum;

//     // Tint the blur with bloomColor
//     blur *= bloomColor;

//     // Mix original and tinted blur
//     vec3 result = mix(original, blur, bloomStrength * 0.7);

//     // Optional additive boost
//     result += blur * bloomStrength * 0.3;

//     // --- Vignette pass ---
//     vec2 centeredCoord = fragTexCoord - vec2(0.5);
//     float dist = length(centeredCoord) / 0.7071; // normalized to 0..1
//     dist = clamp(dist, 0.0, 1.0);
//     float vignette = smoothstep(0.4, 1.0, dist);
//     result = mix(result, vec3(0.0), vignette * vignetteStrength);

//     // --- Bootleg AA pass ---
//     // Very subtle 3x3 blur for fake anti-aliasing
//     vec3 aaBlur = vec3(0.0);
//     float aaWeightSum = 0.0;
//     for (int y = -1; y <= 1; y++) {
//         for (int x = -1; x <= 1; x++) {
//             vec2 offset = vec2(x, y) * texelSize;
//             vec3 sample = texture(sceneTexture, fragTexCoord + offset).rgb;
//             aaBlur += sample;
//             aaWeightSum += 1.0;
//         }
//     }
//     aaBlur /= aaWeightSum;

//     // Blend in the blur gently
//     result = mix(result, aaBlur, aaStrength);

//     // Clamp final color
//     result = clamp(result, 0.0, 1.0);
//     finalColor = vec4(result, 1.0);


// }
