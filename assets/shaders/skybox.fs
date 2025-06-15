#version 330

in vec3 fragPosition;
out vec4 finalColor;

uniform float time;
uniform int isDungeon;
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
    if (isDungeon == 1) {
        finalColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 dir = normalize(fragPosition);
    float cloudFreq = 2.0;
    float cloudSpeed = 0.01;

    float cloud = noise(dir * cloudFreq + vec3(0.0, time * cloudSpeed, 0.0));
    float cloudAlpha = smoothstep(0.5, 0.8, cloud);

    vec3 sky = vec3(0.3, 0.6, 1.0);
    vec3 clouds = vec3(1.0);
    vec3 result = mix(sky, clouds, cloudAlpha);

    finalColor = vec4(result, 1.0);
}
