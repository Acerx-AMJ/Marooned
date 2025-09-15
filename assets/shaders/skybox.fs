#version 330
in vec3 vDir;
out vec4 finalColor;

uniform float time;       // seconds
uniform int   isDungeon;  // 0 = overworld/day; 1 = dungeon (starfield)

// --------- small helpers ----------
float hash31(vec3 p) {
    return fract(sin(dot(p, vec3(127.1,311.7,74.7))) * 43758.5453123);
}

float noise3(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f*f*(3.0 - 2.0*f);
    float n000 = hash31(i + vec3(0,0,0));
    float n100 = hash31(i + vec3(1,0,0));
    float n010 = hash31(i + vec3(0,1,0));
    float n110 = hash31(i + vec3(1,1,0));
    float n001 = hash31(i + vec3(0,0,1));
    float n101 = hash31(i + vec3(1,0,1));
    float n011 = hash31(i + vec3(0,1,1));
    float n111 = hash31(i + vec3(1,1,1));

    float nx00 = mix(n000, n100, f.x);
    float nx10 = mix(n010, n110, f.x);
    float nx01 = mix(n001, n101, f.x);
    float nx11 = mix(n011, n111, f.x);
    float nxy0 = mix(nx00, nx10, f.y);
    float nxy1 = mix(nx01, nx11, f.y);
    return mix(nxy0, nxy1, f.z);
}

// cheap fractal brownian motion
float fbm(vec3 p) {
    float a = 0.5, s = 0.0;
    for (int i=0;i<4;i++) { s += a*noise3(p); p*=2.0; a*=0.5; }
    return s;
}

// -----------------------------------

void main() {
    vec3 dir = normalize(vDir);

    if (isDungeon == 1) {
        // --- NIGHT: minimal starfield ---
        // base night tint (very dark blue)
        vec3 night = vec3(0.001, 0.002, 0.004);



        // small stars
        float n1 = noise3(dir * 80.0);
        float starsSmall = step(0.97, n1) * pow(n1, 40.0);

        // rare bright stars
        float n2 = noise3(dir * 40.0);
        float starsBig = step(0.98, n2) * pow(n2, 10.0) * 3.0;

        vec3 col = night + vec3(1.0)*(starsSmall + starsBig);



        // Brighter, larger stars
        //vec3 col = night + vec3(1.0) * stars * tw * 2.5;
        finalColor = vec4(pow(col, vec3(1.0/2.2)), 1.0);
        return;
    }

    // --- DAY: blue sky + soft clouds ---
    // sky gradient from zenith
    float h = clamp(dir.y*0.5 + 0.5, 0.0, 1.0);
    vec3 top = vec3(0.0, 0.10, 1.00); // zenith blue
    vec3 hor = vec3(0.0, 0.60, 1.00); // near horizon
    vec3 sky = mix(hor, top, pow(h, 1.2));

    // simple moving clouds (soft fbm mask)
    float cloud = fbm(dir * 3.0 + vec3(0.0, time*0.01, 0.0));
    float mask = smoothstep(0.55, 0.70, cloud); // threshold for puffy shapes
    vec3 clouds = vec3(1.0);

    vec3 col = mix(sky, clouds, mask * 0.8); // 0.8 = cloud opacity
    finalColor = vec4(pow(col, vec3(1.0/2.2)), 1.0);
}


// #version 330

// in vec3 fragPosition;
// out vec4 finalColor;

// uniform float time;
// uniform int isDungeon;
// uniform samplerCube environmentMap;

// // 3D Tileable Noise (simple)
// float hash(vec3 p) {
//     return fract(sin(dot(p ,vec3(127.1, 311.7, 74.7))) * 43758.5453);
// }

// float noise(vec3 p) {
//     vec3 i = floor(p);
//     vec3 f = fract(p);
//     f = f*f*(3.0 - 2.0*f);

//     float n = mix(mix(mix( hash(i + vec3(0,0,0)), 
//                            hash(i + vec3(1,0,0)), f.x),
//                        mix( hash(i + vec3(0,1,0)), 
//                            hash(i + vec3(1,1,0)), f.x), f.y),
//                    mix(mix( hash(i + vec3(0,0,1)), 
//                            hash(i + vec3(1,0,1)), f.x),
//                        mix( hash(i + vec3(0,1,1)), 
//                            hash(i + vec3(1,1,1)), f.x), f.y), f.z);
//     return n;
// }

// void main() {
//     if (isDungeon == 1) {
//         finalColor = vec4(0.0, 0.0, 0.0, 1.0);
//         return;
//     }

//     vec3 dir = normalize(fragPosition);
//     float cloudFreq = 2.0;
//     float cloudSpeed = 0.01;

//     float cloud = noise(dir * cloudFreq + vec3(0.0, time * cloudSpeed, 0.0));
//     float cloudAlpha = smoothstep(0.5, 0.8, cloud);

//     vec3 sky = vec3(0.3, 0.6, 1.0);
//     vec3 clouds = vec3(1.0);
//     vec3 result = mix(sky, clouds, cloudAlpha);

//     finalColor = vec4(result, 1.0);
// }
