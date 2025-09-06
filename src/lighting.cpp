#include "lighting.h"
#include "raylib.h"
#include "pathfinding.h"
#include "dungeonGeneration.h"
#include "raymath.h"
#include "cfloat"
#include "stdint.h"
#include "world.h"
#include "utilities.h"


BakedLightmap gDynamic; 



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

void InitDynamicLightmap(int res)
{
    // If re-initting, free the old GPU texture to avoid leaks
    if (gDynamic.tex.id != 0){
        std::cout << gDynamic.tex.id << "previous texture found\n";
        UnloadTexture(gDynamic.tex);
    } 

    // Resolution
    gDynamic.w = res;
    gDynamic.h = res;

    // World-space mapping for this level (XZ bounds)
    ComputeDungeonXZBounds(dungeonWidth, dungeonHeight, tileSize, floorHeight,
                           gDynamic.minX, gDynamic.minZ, gDynamic.sizeX, gDynamic.sizeZ);

    // CPU buffer (black = no light)
    gDynamic.pixels.assign(gDynamic.w * gDynamic.h, (Color){0,0,0,255});

    // GPU texture
    Image img = GenImageColor(gDynamic.w, gDynamic.h, BLACK);
    gDynamic.tex = LoadTextureFromImage(img);
    UnloadImage(img);

    SetTextureFilter(gDynamic.tex, TEXTURE_FILTER_BILINEAR);
    SetTextureWrap(gDynamic.tex, TEXTURE_WRAP_CLAMP);
}


// Simple smooth falloff (1 at center -> 0 at radius)
float SmoothFalloff(float d, float radius)
{
    if (d >= radius) return 0.0f;
    float t = 1.0f - (d / radius);   // 1..0
    // smootherstep style
    return t * t * (3.0f - 2.0f * t);
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



static void StampDynamicLight(const Vector3& lightPos, float radius, Color color) {
    // Map world XZ -> texture space
    float u = (lightPos.x - gDynamic.minX) / gDynamic.sizeX; // 0..1
    float v = (lightPos.z - gDynamic.minZ) / gDynamic.sizeZ; // 0..1

    int cx = (int)(u * gDynamic.w);
    int cy = (int)(v * gDynamic.h);

    int rx = (int)ceilf((radius / gDynamic.sizeX) * gDynamic.w);
    int ry = (int)ceilf((radius / gDynamic.sizeZ) * gDynamic.h);

    int x0 = std::max(0, cx - rx), x1 = std::min(gDynamic.w - 1, cx + rx);
    int y0 = std::max(0, cy - ry), y1 = std::min(gDynamic.h - 1, cy + ry);

    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            float texU = (x + 0.5f) / gDynamic.w;
            float texV = (y + 0.5f) / gDynamic.h;
            float wx = gDynamic.minX + texU * gDynamic.sizeX;
            float wz = gDynamic.minZ + texV * gDynamic.sizeZ;

            float dx = wx - lightPos.x;
            float dz = wz - lightPos.z;
            float d  = sqrtf(dx*dx + dz*dz);

            float w = SmoothFalloff(d, radius);
            if (w <= 0.0f) continue;

            Color& p = gDynamic.pixels[y * gDynamic.w + x];

            // Additive (clamped); dynamic map stores 0..255 color
            int r = p.r + (int)(color.r * w);
            int g = p.g + (int)(color.g * w);
            int b = p.b + (int)(color.b * w);
            p.r = (unsigned char)std::min(r, 255);
            p.g = (unsigned char)std::min(g, 255);
            p.b = (unsigned char)std::min(b, 255);
            // alpha unused; keep 255
        }
    }
}


static size_t CountNonBlack(const std::vector<Color>& buf) {
    size_t n = 0;
    for (const Color& c : buf) if ( (c.r | c.g | c.b) != 0 ) ++n;
    return n;
}

// Call this right before/after UpdateTexture(...)
void LogDynamicLightmapNonBlack(const char* tag) {
    size_t nb = CountNonBlack(gDynamic.pixels);
    TraceLog(LOG_INFO, "[%s] nonBlack=%zu / %zu  texID=%d  res=%dx%d  bounds={minX=%.2f minZ=%.2f sizeX=%.2f sizeZ=%.2f}",
             tag, nb, gDynamic.pixels.size(),
             gDynamic.tex.id, gDynamic.w, gDynamic.h,
             gDynamic.minX, gDynamic.minZ, gDynamic.sizeX, gDynamic.sizeZ);
}

void BuildDynamicLightmapFromFrameLights(const std::vector<LightSample>& frameLights) {
    if (isLoadingLevel) return;
    // 1) Clear to black
    std::fill(gDynamic.pixels.begin(), gDynamic.pixels.end(), (Color){0,0,0,255});


    for (const LightSample& L : frameLights) {

        Color c = {
            (unsigned char)Clamp(L.color.x * 255.0f * L.intensity, 0.0f, 255.0f),
            (unsigned char)Clamp(L.color.y * 255.0f * L.intensity, 0.0f, 255.0f),
            (unsigned char)Clamp(L.color.z * 255.0f * L.intensity, 0.0f, 255.0f),
            255
        };
        StampDynamicLight(L.pos, L.range, c);

    }

    for (const LightSource& L : dungeonLights){

        Color c = {
            (unsigned char)Clamp(L.colorTint.x * 255.0f * L.intensity, 0.0f, 255.0f),
            (unsigned char)Clamp(L.colorTint.y * 255.0f * L.intensity, 0.0f, 255.0f),
            (unsigned char)Clamp(L.colorTint.z * 255.0f * L.intensity, 0.0f, 255.0f),
            255
        };
        StampDynamicLight(L.position, L.range, c);
        
    }

    // 3) Upload to GPU
   
    //LogDynamicLightmapNonBlack("build pre upload");
    UpdateTexture(gDynamic.tex, gDynamic.pixels.data());
    //LogDynamicLightmapNonBlack("build post upload");
    
}


