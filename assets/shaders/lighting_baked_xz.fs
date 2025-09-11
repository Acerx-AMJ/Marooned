#version 330

in vec2 vUV;
in vec3 vWorldPos;
in vec4 vColor;          // For floors, set tint to WHITE and ignore it here

out vec4 finalColor;

uniform sampler2D texture0;        // floor albedo
uniform vec4      colDiffuse;      // set to WHITE for floors

uniform sampler2D dynamicGridTex;  // the single XZ lightmap (RGB = lights, A = lava mask)

// gridBounds = { minX, minZ, invSizeX, invSizeZ }
uniform vec4  gridBounds;

// Lighting knobs
uniform float dynStrength;     // 0..2 (default 1.0)
uniform float ambientBoost;    // e.g., 0.02–0.08

// New lava/ceiling knobs
uniform int   isCeiling;       // 0 = floor, 1 = ceiling
uniform float lavaCeilStrength; // how much lava boosts ceilings
uniform float ceilHeight;      // world-space Y where ceilings live (for attenuation)
uniform float lavaFalloff;     // how fast lava glow fades with height
                               // (e.g., 200.0 means at 200 units above floor, glow ≈0)

void main() {
    // Albedo (ignore vColor tint for floors if colDiffuse = WHITE)
    vec4 baseS = texture(texture0, vUV);
    vec3 base  = baseS.rgb * colDiffuse.rgb;
    float alpha = baseS.a * colDiffuse.a;

    // World XZ -> dynamic lightmap UV
    float u = (vWorldPos.x - gridBounds.x) * gridBounds.z;
    float v = (vWorldPos.z - gridBounds.y) * gridBounds.w;
    vec2  lmUV = clamp(vec2(u, v), vec2(0.0), vec2(1.0));
    //vec2 lmUV = clamp(vec2(u, 1.0 - v), vec2(0.0), vec2(1.0));

    // Sample lightmap once: RGB = dynamic light, A = lava mask
    vec4 lm = texture(dynamicGridTex, lmUV);
    vec3 dyn = lm.rgb;
    float lavaMask = lm.a; // 0..1

    // Base lighting (same as before)
    vec3 L = clamp(dyn * dynStrength + vec3(ambientBoost), 0.0, 1.0);

    // If drawing ceilings, add lava glow based on alpha mask
    if (isCeiling == 1) {
        // Vertical attenuation so glow fades the higher the ceiling
        float distY = max(0.0, vWorldPos.y - ceilHeight);
        float atten = exp(-distY / lavaFalloff); // exponential falloff

        // Add emissive red/orange glow
        vec3 lavaGlow = vec3(1.0, 0.25, 0.0) * lavaMask * lavaCeilStrength * atten;
        L = clamp(L + lavaGlow, 0.0, 1.0);
    }

    // Shade final
    finalColor = vec4(base * L, alpha);
}


// #version 330

// in vec2 vUV;
// in vec3 vWorldPos;
// in vec4 vColor;         // For floors, set tint to WHITE and ignore it here

// out vec4 finalColor;

// uniform sampler2D texture0;        // floor albedo
// uniform vec4      colDiffuse;      // set to WHITE for floors

// uniform sampler2D dynamicGridTex;  // the single XZ lightmap you rebuild each frame

// // gridBounds = { minX, minZ, invSizeX, invSizeZ } for the *same* mapping used to stamp
// uniform vec4  gridBounds;

// // Simple knobs
// uniform float dynStrength;   // 0..2 (start 1.0)
// uniform float ambientBoost;  // small base so pure dark isn’t ink (e.g., 0.02..0.08)

// void main() {
//     // Albedo only (no per-tile tint for floors)
//     vec4 baseS = texture(texture0, vUV);
//     vec3 base  = baseS.rgb * colDiffuse.rgb;
//     float alpha = baseS.a * colDiffuse.a;

//     // World XZ -> dynamic lightmap UV
//     float u = (vWorldPos.x - gridBounds.x) * gridBounds.z;
//     float v = (vWorldPos.z - gridBounds.y) * gridBounds.w;
//     vec2  lmUV = clamp(vec2(u, v), vec2(0.0), vec2(1.0));

//     // Dynamic light (the only light)
//     vec3 dyn = texture(dynamicGridTex, lmUV).rgb;

//     // Final lighting = scaled dynamic + tiny ambient floor
//     vec3 L = clamp(dyn * dynStrength + vec3(ambientBoost), 0.0, 1.0);

//     // Shade
//     finalColor = vec4(base * L, alpha);
//     //finalColor = vec4(texture(dynamicGridTex, lmUV).rgb, 1.0);
// }


// #version 330

// in vec2 vUV;
// in vec3 vWorldPos;
// in vec4 vColor;

// out vec4 finalColor;

// uniform sampler2D texture0;
// uniform vec4      colDiffuse;

// uniform sampler2D dynamicGridTex;
// uniform vec4  gridBounds;      // {minX, minZ, invSizeX, invSizeZ}
// uniform float dynStrength;
// uniform float ambientBoost;

// // NEW: baked lava glow, sampled top-down like dynamicGridTex
// uniform sampler2D lavaGlowTex;     // R channel footprint/glow
// uniform vec3  lavaGlowColor;       // e.g., vec3(1.0, 0.42, 0.12)
// uniform float lavaGlowIntensity;   // 0..2
// uniform int   isCeiling;           // 1 only when drawing ceiling tiles

// void main() {
//     vec4 baseS = texture(texture0, vUV);
//     vec3 base  = baseS.rgb * colDiffuse.rgb;
//     float alpha = baseS.a * colDiffuse.a;

//     // XZ -> [0..1]^2 (same mapping as your dynamic grid)
//     vec2 lmUV = clamp(vec2(
//         (vWorldPos.x - gridBounds.x) * gridBounds.z,
//         (vWorldPos.z - gridBounds.y) * gridBounds.w), 0.0, 1.0);

//     // Dynamic lighting you already have
//     vec3 dyn = texture(dynamicGridTex, lmUV).rgb;
//     vec3 L   = clamp(dyn * dynStrength + vec3(ambientBoost), 0.0, 1.0);

//     // Optional emissive from lava **only on ceilings**
//     if (isCeiling == 1) {

//         float glow = texture(lavaGlowTex, lmUV).r;   // [0..1]
//         vec3  E    = lavaGlowColor * (lavaGlowIntensity * glow);

//         // Headroom-style combine so colors don't go muddy:
//         L = 1.0 - (1.0 - L) * (1.0 - E);
//     }

//     finalColor = vec4(base * L, alpha);
    
// }