// #version 330

// in vec3 fragPosition;
// out vec4 finalColor;

// uniform float time;

// float hash(vec2 p) {
//     return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
// }

// float noise(vec2 p) {
//     vec2 i = floor(p);
//     vec2 f = fract(p);
//     float a = hash(i);
//     float b = hash(i + vec2(1.0, 0.0));
//     float c = hash(i + vec2(0.0, 1.0));
//     float d = hash(i + vec2(1.0, 1.0));
//     vec2 u = f * f * (3.0 - 2.0 * f);
//     return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
// }

// void main() {
//     vec3 dir = normalize(fragPosition);
//     float t = dir.y * 0.5 + 0.5;

//     vec3 topColor = vec3(0.2, 0.4, 0.8);
//     vec3 bottomColor = vec3(0.9, 0.9, 1.0);
//     vec3 color = mix(bottomColor, topColor, t);

//     // Procedural cloud layer based on xz direction
//     vec2 uv = dir.xz * 5.0 + vec2(time * 0.05, 0.0);
//     float cloud = noise(uv);

//     // Sharpen and blend the clouds
//     cloud = smoothstep(0.4, 0.7, cloud);
//     color = mix(color, vec3(1.0), cloud * 0.4); // blend clouds into sky

//     finalColor = vec4(color, 1.0);
// }


#version 330

in vec3 fragPosition;
out vec4 finalColor;

void main() {
    vec3 dir = normalize(fragPosition);

    float t = dir.y * 0.5 + 0.5; // blend based on "upness"

    // Top = deep sky blue, bottom = near-white horizon
    vec3 topColor = vec3(0.2, 0.4, 0.8);   // sky blue
    vec3 bottomColor = vec3(0.6, 0.9, 1.0); // pale horizon

    vec3 color = mix(bottomColor, topColor, t);
    finalColor = vec4(color, 1.0);
}
