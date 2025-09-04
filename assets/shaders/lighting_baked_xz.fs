#version 330

in vec2 vUV;
in vec3 vWorldPos;
in vec4 vColor;         // For floors, set tint to WHITE and ignore it here

out vec4 finalColor;

uniform sampler2D texture0;        // floor albedo
uniform vec4      colDiffuse;      // set to WHITE for floors

uniform sampler2D dynamicGridTex;  // the single XZ lightmap you rebuild each frame

// gridBounds = { minX, minZ, invSizeX, invSizeZ } for the *same* mapping used to stamp
uniform vec4  gridBounds;

// Simple knobs
uniform float dynStrength;   // 0..2 (start 1.0)
uniform float ambientBoost;  // small base so pure dark isn’t ink (e.g., 0.02..0.08)

void main() {
    // Albedo only (no per-tile tint for floors)
    vec4 baseS = texture(texture0, vUV);
    vec3 base  = baseS.rgb * colDiffuse.rgb;
    float alpha = baseS.a * colDiffuse.a;

    // World XZ -> dynamic lightmap UV
    float u = (vWorldPos.x - gridBounds.x) * gridBounds.z;
    float v = (vWorldPos.z - gridBounds.y) * gridBounds.w;
    vec2  lmUV = clamp(vec2(u, v), vec2(0.0), vec2(1.0));

    // Dynamic light (the only light)
    vec3 dyn = texture(dynamicGridTex, lmUV).rgb;

    // Final lighting = scaled dynamic + tiny ambient floor
    vec3 L = clamp(dyn * dynStrength + vec3(ambientBoost), 0.0, 1.0);

    // Shade
    finalColor = vec4(base * L, alpha);
}


// #version 330
// in vec2 vUV;
// in vec3 vWorldPos;
// in vec4 vColor;

// out vec4 finalColor;

// uniform sampler2D texture0;       // base/albedo
// uniform vec4      colDiffuse;     // set to WHITE for floors

// uniform sampler2D lightGridTex;   // baked XZ
// uniform sampler2D dynamicGridTex; // dynamic XZ

// uniform vec4  gridBounds;         // { minX, minZ, invSizeX, invSizeZ }
// uniform float bakedStrength;      // keep what worked for you
// uniform float ambientBoost;       // small, e.g. 0.02–0.10
// uniform float dynGain;            // NEW: emissive gain, e.g. 0.5–1.5

// void main() {
//     // Albedo (drop vColor for floors so tint can’t fight lighting)
//     vec4 baseS = texture(texture0, vUV);
//     vec3 base  = baseS.rgb * colDiffuse.rgb;
//     float alpha = baseS.a * colDiffuse.a;

//     // Shared UV for both lightmaps
//     float u = (vWorldPos.x - gridBounds.x) * gridBounds.z;
//     float v = (vWorldPos.z - gridBounds.y) * gridBounds.w;
//     vec2  lmUV = clamp(vec2(u, v), vec2(0.0), vec2(1.0));

//     // Baked lighting (unchanged)
//     vec3 baked = texture(lightGridTex, lmUV).rgb;
//     baked = baked * bakedStrength + ambientBoost;

//     // Base lit by the bake
//     vec3 lit = base * baked;

//     // Dynamic overlay as a small emissive add (not a multiplier)
//     vec3 dyn = texture(dynamicGridTex, lmUV).rgb;

//     // Optional: reduce washout in already-bright baked areas
//     // weight is ~1 in the dark, ~0.6 in bright areas
//     float weight = 0.6 + 0.4 * (1.0 - clamp(max(max(baked.r,baked.g),baked.b), 0.0, 1.0));

//     vec3 outRGB = clamp(lit + dyn * (dynGain * weight), 0.0, 1.0);
//     finalColor = vec4(outRGB, alpha);
// }


// #version 330

// in vec2 vUV;
// in vec3 vWorldPos;
// in vec4 vColor;

// out vec4 finalColor;

// uniform sampler2D texture0;    // base/albedo
// uniform vec4      colDiffuse;  // material tint

// uniform sampler2D lightGridTex;    // baked (same as before)
// uniform sampler2D dynamicGridTex;  // NEW dynamic overlay (optional)

// uniform vec4  gridBounds;      // { minX, minZ, invSizeX, invSizeZ }
// uniform float bakedStrength;   // same
// uniform float ambientBoost;    // same
// uniform float dynStrength;     // NEW (0.0 → off)

// void main() {
//     // EXACTLY as before (keep vColor here to match prior visuals)
//     //vec4 base = texture(texture0, vUV) * colDiffuse * vColor;

//     vec4 baseS = texture(texture0, vUV);
//     vec4 base  = baseS * colDiffuse;   // <- no * vColor here

//     // World→lightmap UV (same as before)
//     float u = (vWorldPos.x - gridBounds.x) * gridBounds.z;
//     float v = (vWorldPos.z - gridBounds.y) * gridBounds.w;
//     vec2  lmUV = clamp(vec2(u, v), vec2(0.0), vec2(1.0));

//     vec3 baked = texture(lightGridTex, lmUV).rgb;
//     baked = baked * bakedStrength + ambientBoost;
//     baked = min(baked, vec3(0.8));   // hard cap: guarantees room for dynamics

//     // Dynamic overlay (neutral if dynStrength == 0)
//     vec3 dyn = texture(dynamicGridTex, lmUV).rgb;

//     // Combine in lighting domain; when dynStrength = 0, L == baked
//     //vec3 L = baked + (vec3(1.0) - baked) * (dynStrength * dyn);

//     //finalColor = vec4(base.rgb * L, base.a);
//     finalColor = vec4(base.rgb * (dyn), base.a);
// }



// #version 330
// in vec2 vUV;
// in vec3 vWorldPos;
// in vec4 vColor;
// out vec4 finalColor;

// uniform sampler2D texture0;
// uniform vec4 colDiffuse;

// uniform sampler2D lightGridTex;     // baked (as before)
// uniform sampler2D dynamicGridTex;   // NEW

// uniform vec4  gridBounds;    // { minX, minZ, invSizeX, invSizeZ }
// uniform float bakedStrength; // as before
// uniform float ambientBoost;  // as before
// uniform float dynStrength;   // NEW (0.0 == off)

// void main() {
//     vec4 base = texture(texture0, vUV) * colDiffuse * vColor;

//     float u = (vWorldPos.x - gridBounds.x) * gridBounds.z;
//     float v = (vWorldPos.z - gridBounds.y) * gridBounds.w;
//     vec2  lmUV = clamp(vec2(u, v), vec2(0.0), vec2(1.0));

//     vec3 baked = texture(lightGridTex,   lmUV).rgb;
//     vec3 dyn   = texture(dynamicGridTex, lmUV).rgb;

//     baked = baked * bakedStrength + ambientBoost;

//     // Combine; when dynStrength = 0.0, L == baked (identical to old shader)
//     vec3 L = baked + (vec3(1.0) - baked) * (dynStrength * dyn);
//     //finalColor = vec4(texture(dynamicGridTex, lmUV).rgb, 1.0);
//     finalColor = vec4(base.rgb * L, base.a);
// }






// #version 330

// in vec2 vUV;
// in vec3 vWorldPos;
// in vec4 vColor;


// out vec4 finalColor;

// // Raylib default samplers/uniforms
// uniform sampler2D texture0;    // base/albedo
// uniform vec4 colDiffuse;       // material tint

// // Baked lightmap (your 128x128 texture)
// uniform sampler2D lightGridTex;

// // Mapping from world XZ -> [0..1] UV for the baked map
// // gridBounds = { minX, minZ, invSizeX, invSizeZ }
// uniform vec4 gridBounds;

// // Optional tweak knobs
// uniform float bakedStrength;  // 1.0 = full baked, 0.0 = off
// uniform float ambientBoost;   // e.g. 0.0..1.0, add small base light if you baked very dark

// void main() {

//     vec4 base = texture(texture0, vUV) * colDiffuse * vColor;

//     // Map world XZ into baked lightmap UVs
//     float u = (vWorldPos.x - gridBounds.x) * gridBounds.z;
//     float v = (vWorldPos.z - gridBounds.y) * gridBounds.w;

//     // Clamp to [0,1] to avoid edge sampling artifacts
//     vec2 lmUV = clamp(vec2(u, v), vec2(0.0), vec2(1.0));

//     vec3 baked = texture(lightGridTex, lmUV).rgb;

//     // Optional small ambient boost (prevents pitch-black)
//     baked = baked * bakedStrength + ambientBoost;

//     finalColor = vec4(base.rgb * baked, base.a);
    
//     //finalColor = vec4(texture(lightGridTex, clamp(vec2(u, v), vec2(0.0), vec2(1.0))).rgb, 1.0);
// }
