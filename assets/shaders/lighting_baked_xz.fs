
#version 330

in vec2 vUV;
in vec3 vWorldPos;
in vec4 vColor;


out vec4 finalColor;

// Raylib default samplers/uniforms
uniform sampler2D texture0;    // base/albedo
uniform vec4 colDiffuse;       // material tint

// Baked lightmap (your 128x128 texture)
uniform sampler2D lightGridTex;

// Mapping from world XZ -> [0..1] UV for the baked map
// gridBounds = { minX, minZ, invSizeX, invSizeZ }
uniform vec4 gridBounds;

// Optional tweak knobs
uniform float bakedStrength;  // 1.0 = full baked, 0.0 = off
uniform float ambientBoost;   // e.g. 0.0..1.0, add small base light if you baked very dark

void main() {

    vec4 base = texture(texture0, vUV) * colDiffuse * vColor;

    // Map world XZ into baked lightmap UVs
    float u = (vWorldPos.x - gridBounds.x) * gridBounds.z;
    float v = (vWorldPos.z - gridBounds.y) * gridBounds.w;

    // Clamp to [0,1] to avoid edge sampling artifacts
    vec2 lmUV = clamp(vec2(u, v), vec2(0.0), vec2(1.0));

    vec3 baked = texture(lightGridTex, lmUV).rgb;

    // Optional small ambient boost (prevents pitch-black)
    baked = baked * bakedStrength + ambientBoost;

    finalColor = vec4(base.rgb * baked, base.a);
    
    //finalColor = vec4(texture(lightGridTex, clamp(vec2(u, v), vec2(0.0), vec2(1.0))).rgb, 1.0);
}
