#version 330

in vec3 fragNormal;
in vec3 fragPosition;

out vec4 finalColor;

uniform vec3 cameraPos;

uniform vec2 u_WorldMinXZ;   // (minX, minZ)
uniform vec2 u_WorldSizeXZ;  // (sizeX, sizeZ)

// Textures bound via material slots
uniform sampler2D texGrass;
uniform sampler2D texSand;
uniform sampler2D textureOcclusion;   // palm shadow mask (RenderTexture)

// Tiling controls
uniform float grassTiling;  // e.g. 60.0
uniform float sandTiling;   // e.g. 20.0

// Fog / sky
uniform vec3  u_SkyColorTop;      // e.g. vec3(0.55, 0.75, 1.00)
uniform vec3  u_SkyColorHorizon;  // e.g. vec3(0.72, 0.82, 0.95)
uniform float u_FogStart;         // e.g. 4000.0
uniform float u_FogEnd;           // e.g. 18000.0
uniform float u_SeaLevel;         // e.g. 60.0
uniform float u_FogHeightFalloff; // e.g. 0.002

void main()
{
    float height = fragPosition.y;

    // ---- World-space UVs (0..1 across island), then tile
    vec2 uvIsland = (fragPosition.xz - u_WorldMinXZ) / u_WorldSizeXZ;
    uvIsland = clamp(uvIsland, 0.0, 1.0);

    vec2 uvGrass = uvIsland * grassTiling;
    vec2 uvSand  = uvIsland * sandTiling;

    // ---- Sample base textures
    vec3 grassTex = texture(texGrass, uvGrass).rgb;
    vec3 sandTex  = texture(texSand,  uvSand ).rgb;

    // ---- Height-based blend: water -> sand -> grass
    vec3 waterColor = vec3(0.10, 0.60, 1.00);

    float tWaterSand = smoothstep(30.0, 80.0, height);
    vec3 baseWS = mix(waterColor, sandTex, tWaterSand);

    float tSandGrass = smoothstep(100.0, 130.0, height);
    vec3 color = mix(baseWS, grassTex, tSandGrass);

    // Optional overall brightness (feel free to tweak/remove)
    color *= 0.85;

    // ---- Palm shadow mask (note Y flip for RenderTexture)
    vec2 uvMask = vec2(uvIsland.x, 1.0 - uvIsland.y);
    float mask = texture(textureOcclusion, uvMask).r; // 1=no shadow, 0=dark
    color *= mask;

    // ---- Sky-colored fog (no grayscale desat)
    float dist      = length(fragPosition - cameraPos);
    float fogDist   = smoothstep(u_FogStart, u_FogEnd, dist);

    float h         = max(fragPosition.y - u_SeaLevel, 0.0);
    float fogHeight = exp(-h * u_FogHeightFalloff);   // heavier near sea level
    float fog       = clamp(fogDist * fogHeight, 0.0, 1.0);

    // Simple sky gradient: bias toward horizon near sea level
    float skyT     = clamp((cameraPos.y - u_SeaLevel) / 600.0, 0.0, 1.0);
    vec3  skyColor = mix(u_SkyColorHorizon, u_SkyColorTop, skyT);

    color = mix(color, skyColor, fog);

    finalColor = vec4(color, 1.0);
}


// #version 330

// in vec3 fragNormal;
// in vec3 fragPosition;
// out vec4 finalColor;

// uniform vec3 cameraPos;

// // already used for the shadow mask + world-space mapping
// uniform vec2 u_WorldMinXZ;   // (minX, minZ)
// uniform vec2 u_WorldSizeXZ;  // (sizeX, sizeZ)

// // textures
// uniform sampler2D textureOcclusion;  // your palm shadow mask (bound to MATERIAL_MAP_OCCLUSION)
// uniform sampler2D texGrass;          // tileable grass
// uniform sampler2D texSand;           // tileable sand

// // tiling controls (how many repeats across the island width)
// uniform float grassTiling;  // e.g. 60.0
// uniform float sandTiling;   // e.g. 20.0



// void main()
// {
//     float height = fragPosition.y;

//     // === World-space UVs (0..1 across island), then tile ===
//     vec2 uvIsland = (fragPosition.xz - u_WorldMinXZ) / u_WorldSizeXZ;
//     uvIsland = clamp(uvIsland, 0.0, 1.0);

//     vec2 uvGrass = uvIsland * grassTiling;
//     vec2 uvSand  = uvIsland * sandTiling;

//     // Sample textures
//     vec3 grassTex = texture(texGrass, uvGrass).rgb;
//     vec3 sandTex  = texture(texSand,  uvSand ).rgb;

//     // === Height-based blending (tweak thresholds to match your old look) ===
//     // 1) water → sand
//     float tWaterSand = smoothstep(30.0, 80.0, height);  // 0=water, 1=sand+
//     vec3 baseWS = mix(vec3(0.1, 0.6, 1.0), sandTex, tWaterSand);

//     // 2) sand → grass
//     float tSandGrass = smoothstep(100.0, 130.0, height);
//     vec3 base = mix(baseWS, grassTex, tSandGrass);

//     // Global brightness like before
//     base *= 0.80;

//     // === Palm shadow mask ===
//     vec2 uvMask = uvIsland;
//     uvMask.y = 1.0 - uvMask.y;                // RenderTexture flip
//     float mask = texture(textureOcclusion, uvMask).r;  // 1=no shadow, 0=darkest
//     base *= mask;

//     // === Distance desaturation / fog tint 
//     float dist = length(fragPosition - cameraPos);
//     float desatFactor = clamp((dist - 5000.0) / 20000.0, 0.0, 1.0);
//     float grayscale = dot(base, vec3(0.299, 0.587, 0.114));
//     base = mix(base, vec3(grayscale), desatFactor);

//     finalColor = vec4(base, 1.0);
// }


// #version 330

// in vec3 fragNormal;
// in vec3 fragPosition;

// out vec4 finalColor;

// uniform vec3 cameraPos;

// // NEW: world bounds and shadow mask
// uniform vec2 u_WorldMinXZ;   // (minX, minZ)
// uniform vec2 u_WorldSizeXZ;  // (sizeX, sizeZ)
// uniform sampler2D u_ShadowMask; // R channel: 1=no shadow, 0=full shadow
// uniform sampler2D textureOcclusion;  // raylib will bind this for MATERIAL_MAP_OCCLUSION

// void main()
// {
//     float height = fragPosition.y;

//     vec3 waterColor = vec3(0.1, 0.6, 1.0);
//     vec3 sandColor  = vec3(0.9, 0.9, 0.01);
//     vec3 grassColor = vec3(0.2, 0.7, 0.2);

//     float brightness = 0.60;

//     // Base color by height
//     vec3 color = waterColor;
//     if (height > 60.0) {
//         float t = smoothstep(30.0, 80.0, height);
//         color = mix(waterColor, sandColor, t);

//         if (height > 120.0) {
//             float s = smoothstep(100.0, 130.0, height);
//             color = mix(sandColor, grassColor, s);
//         }
//     }
//     color *= brightness;

//     vec2 uv;
//     uv.x = (fragPosition.x - u_WorldMinXZ.x) / u_WorldSizeXZ.x;
//     uv.y = (fragPosition.z - u_WorldMinXZ.y) / u_WorldSizeXZ.y;
//     uv = clamp(uv, 0.0, 1.0);
//     uv.y = 1.0 - uv.y; // RenderTexture flip

//     float mask = texture(textureOcclusion, uv).r; // 1=no shadow, 0=dark
//     color *= mask;

//     // Distance-based desaturation & fog-ish tint (your existing bits)
//     float dist = length(fragPosition - cameraPos);
//     float desatFactor = clamp((dist - 5000.0) / 20000.0, 0.0, 1.0);

//     vec3 fogColor = vec3(0.6, 0.7, 0.9);
//     float grayscale = dot(color, vec3(0.299, 0.587, 0.114));
//     color = mix(color, vec3(grayscale), desatFactor);

//     finalColor = vec4(color, 1.0);

// }



// #version 330

// in vec3 fragNormal;
// in vec3 fragPosition;

// out vec4 finalColor;

// uniform vec3 cameraPos;

// void main()
// {
//     float height = fragPosition.y;

//     vec3 waterColor = vec3(0.1, .6, 1.0);//vec3(0.01, 0.3, 1.0); //vec3(0.0, 0.3, 1.0);
//     vec3 sandColor  = vec3(0.9, 0.9, 0.01); //vec3(0.9, 0.9, 0.2);
//     vec3 grassColor = vec3(0.2, 0.7, 0.2); 

//     float brightness = 0.60;

//     // Blend based on height
//     vec3 color = waterColor;

//     if (height > 60.0) {
//         float t = smoothstep(30.0, 80.0, height);
//         color = mix(waterColor, sandColor, t);

//         if (height > 120.0) {
//             float s = smoothstep(100.0, 130.0, height);
//             color = mix(sandColor, grassColor, s);
//         }
//     }

//     // Apply brightness
//     color *= brightness;

//     // Distance-based desaturation
//     float dist = length(fragPosition - cameraPos);
//     //float desatFactor = clamp((dist - 100.0) / 500.0, 0.0, 1.0); // fades out over distance
//     float desatFactor = clamp((dist - 5000.0) / 20000.0, 0.0, 1.0);


//     vec3 fogColor = vec3(0.6, 0.7, 0.9);
//     float grayscale = dot(color, vec3(0.299, 0.587, 0.114)); // Luminance formula
    
//     color = mix(color, vec3(grayscale), desatFactor); // blend toward grayscale

//     finalColor = vec4(color, 1.0);


// }


// #version 330

// in vec3 fragNormal;
// in vec3 fragPosition;

// out vec4 finalColor;

// uniform vec3 cameraPos;

// void main()
// {
//     float height = fragPosition.y;

//     // Colors
//     vec3 waterColor = vec3(0.9, 0.9, 0.01);//vec3(0.01, 0.3, 1.0); //vec3(0.0, 0.3, 1.0);
//     vec3 sandColor  = vec3(0.9, 0.9, 0.01); //vec3(0.9, 0.9, 0.2);
//     vec3 grassColor = vec3(0.01, 0.4, 0.01); 

//     // Thresholds
//     float waterLevel = 20.0;
//     float sandLevel  = 30.0;
//     float grassLevel = 150.0;

//     vec3 color;

//     if (height < waterLevel) {
//         color = waterColor;
//     } else if (height < sandLevel) {
//         float t = (height - waterLevel) / (sandLevel - waterLevel);
//         color = mix(waterColor, sandColor, t);
//     } else if (height < grassLevel) {
//         float t = (height - sandLevel) / (grassLevel - sandLevel);
//         color = mix(sandColor, grassColor, t);
//     } else {
//         color = grassColor;
//     }

//     float brightness = 0.35;
//     vec3 corrected = pow(color * brightness, vec3(1.0 / 2.2));
//     finalColor = vec4(corrected, 1.0);
// }
