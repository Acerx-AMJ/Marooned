#include "lighting.h"
#include "raylib.h"
#include "pathfinding.h"
#include "dungeonGeneration.h"
#include "raymath.h"
#include "cfloat"

BakedLightmap gBaked;
// Compute world-space bounds from your dungeon indices & tile size.
// Assumes GetDungeonWorldPos(x,y, tileSize, baseY) returns the *center* of the tile.
static void ComputeDungeonXZBounds(int dungeonWidth, int dungeonHeight, float tileSize,
                                   float baseY, float& outMinX, float& outMinZ,
                                   float& outSizeX, float& outSizeZ)
{
    Vector3 p00 = GetDungeonWorldPos(0, 0, dungeonWidth > 0 ? tileSize : 1.0f, baseY);
    Vector3 p11 = GetDungeonWorldPos(dungeonWidth - 1, dungeonHeight - 1, tileSize, baseY);

    float half = tileSize * 0.5f;

    float minX = std::min(p00.x, p11.x) - half;
    float maxX = std::max(p00.x, p11.x) + half;
    float minZ = std::min(p00.z, p11.z) - half;
    float maxZ = std::max(p00.z, p11.z) + half;

    outMinX = minX;
    outMinZ = minZ;
    outSizeX = (maxX - minX);
    outSizeZ = (maxZ - minZ);
}

void InitBakedLightmap128(int dungeonWidth, int dungeonHeight, float tileSize, float baseY)
{
    gBaked.w = 128;
    gBaked.h = 128;
    gBaked.pixels.assign(gBaked.w * gBaked.h, WHITE);

    // Compute world-space mapping once
    ComputeDungeonXZBounds(dungeonWidth, dungeonHeight, tileSize, baseY,
                           gBaked.minX, gBaked.minZ, gBaked.sizeX, gBaked.sizeZ);

    // Create the GPU texture
    Image img = GenImageColor(gBaked.w, gBaked.h, WHITE);
    gBaked.tex = LoadTextureFromImage(img);
    UnloadImage(img);

    SetTextureFilter(gBaked.tex, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(gBaked.tex, TEXTURE_WRAP_CLAMP);
}

// Simple smooth falloff (1 at center -> 0 at radius)
static inline float SmoothFalloff(float d, float radius)
{
    if (d >= radius) return 0.0f;
    float t = 1.0f - (d / radius);   // 1..0
    // smootherstep style
    return t * t * (3.0f - 2.0f * t);
}

// Stamp a single light as a soft colored blob into the CPU lightmap buffer.
static void StampLightAdditive(const Vector3& lightPos, float radius, Color color)
{
    // Convert light world XZ to grid indices
    float u = (lightPos.x - gBaked.minX) / gBaked.sizeX; // 0..1
    float v = (lightPos.z - gBaked.minZ) / gBaked.sizeZ; // 0..1

    int cx = (int)(u * gBaked.w);
    int cy = (int)(v * gBaked.h);

    // Project radius (world) into texel radius along X
    int rx = (int)ceilf((radius / gBaked.sizeX) * gBaked.w);
    int ry = (int)ceilf((radius / gBaked.sizeZ) * gBaked.h);

    int x0 = std::max(0, cx - rx), x1 = std::min(gBaked.w - 1, cx + rx);
    int y0 = std::max(0, cy - ry), y1 = std::min(gBaked.h - 1, cy + ry);

    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            // texel center in world, distance in world units
            float texU = (x + 0.5f) / gBaked.w;
            float texV = (y + 0.5f) / gBaked.h;
            float wx = gBaked.minX + texU * gBaked.sizeX;
            float wz = gBaked.minZ + texV * gBaked.sizeZ;

            float dx = wx - lightPos.x;
            float dz = wz - lightPos.z;
            float d  = sqrtf(dx*dx + dz*dz);

            float w = SmoothFalloff(d, radius);
            if (w <= 0.0f) continue;

            // Additive brighten (clamped)
            Color& p = gBaked.pixels[y * gBaked.w + x];
            int r = p.r + (int)(color.r * w);
            int g = p.g + (int)(color.g * w);
            int b = p.b + (int)(color.b * w);
            p.r = (unsigned char)std::min(r, 255);
            p.g = (unsigned char)std::min(g, 255);
            p.b = (unsigned char)std::min(b, 255);

            // --- If you prefer multiplicative darkening, comment the 3 lines above and use:
            // float m = Clamp(1.0f - w, 0.0f, 1.0f);
            // p.r = (unsigned char)(p.r * m);
            // p.g = (unsigned char)(p.g * m);
            // p.b = (unsigned char)(p.b * m);
        }
    }
}

void BakeStaticLightmapFromLights(const std::vector<LightSource>& lights)
{
    // super dark baseline
    std::fill(gBaked.pixels.begin(), gBaked.pixels.end(), (Color){ 0,0,0,255 });

    for (const LightSource& L : lights) {
        // scale tint by intensity (and clamp 0..255 later in StampLightAdditive)
        Color c = {
            (unsigned char)(Clamp(L.colorTint.x * L.intensity * 255.0f, 0.0f, 255.0f)),
            (unsigned char)(Clamp(L.colorTint.y * L.intensity * 255.0f, 0.0f, 255.0f)),
            (unsigned char)(Clamp(L.colorTint.z * L.intensity * 255.0f, 0.0f, 255.0f)),
            255
        };

        StampLightAdditive(L.position, L.range, c);
    }

    UpdateTexture(gBaked.tex, gBaked.pixels.data());
}




XZBounds ComputeXZFromTiles(const std::vector<FloorTile>& tiles, float tileSize) {
    XZBounds b{ FLT_MAX, -FLT_MAX, +FLT_MAX, -FLT_MAX };
    for (const auto& t : tiles) {
        b.minX = std::min(b.minX, t.position.x);
        b.maxX = std::max(b.maxX, t.position.x);
        b.minZ = std::min(b.minZ, t.position.z);
        b.maxZ = std::max(b.maxZ, t.position.z);
    }
    const float half = tileSize * 0.5f;
    b.minX -= half; b.maxX += half;
    b.minZ -= half; b.maxZ += half;
    return b;
}
