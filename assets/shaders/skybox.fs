// Skybox Fragment Shader
#version 330

in vec3 fragPosition;
out vec4 finalColor;

uniform float time;
uniform samplerCube environmentMap;

// 3D Tileable Noise (simple)
float hash(vec3 p) {
    return fract(sin(dot(p ,vec3(127.1, 311.7, 74.7))) * 43758.5453);
}

float noise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f*f*(3.0 - 2.0*f);

    float n = mix(mix(mix( hash(i + vec3(0,0,0)), 
                           hash(i + vec3(1,0,0)), f.x),
                       mix( hash(i + vec3(0,1,0)), 
                           hash(i + vec3(1,1,0)), f.x), f.y),
                   mix(mix( hash(i + vec3(0,0,1)), 
                           hash(i + vec3(1,0,1)), f.x),
                       mix( hash(i + vec3(0,1,1)), 
                           hash(i + vec3(1,1,1)), f.x), f.y), f.z);
    return n;
}

void main() {
    vec3 dir = normalize(fragPosition);
    float cloudFreq = 2.0; // higher = more clouds
    float cloudSpeed = 0.01;

    // Move through noise space over time
    float cloud = noise(dir * cloudFreq + vec3(0.0, time * cloudSpeed, 0.0));

    // Cloud alpha ramp
    float cloudAlpha = smoothstep(0.5, 0.8, cloud);

    // Blend sky color + cloud
    vec3 sky = vec3(0.3, 0.6, 1.0);  // base sky blue
    vec3 clouds = vec3(1.0);         // white clouds
    vec3 result = mix(sky, clouds, cloudAlpha);

    finalColor = vec4(result, 1.0);
}
