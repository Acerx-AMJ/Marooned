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

static std::vector<Color> gStaticBase;   // same w*h as gDynamic



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



// Returns 0..1 visibility from light -> tileCenter using a small perpendicular ray fan.
// spreadMeters ~ 0.15f * tileSize; numRays 3..5; epsilonFrac ~ 0.02f
static float TileVisibilityWorldRay(const Vector3& lightPos,
                                    const Vector3& tileCenter,
                                    int numRays, float spreadMeters,
                                    float originYOffset, float targetYOffset,
                                    float epsilonFrac)
{
    // Direction in XZ, build a perpendicular (for fan offsets)
    Vector3 dirXZ = { tileCenter.x - lightPos.x, 0.0f, tileCenter.z - lightPos.z };
    float len = sqrtf(dirXZ.x*dirXZ.x + dirXZ.z*dirXZ.z);
    if (len < 1e-3f) return 1.0f; // same tile
    Vector3 perp = { -dirXZ.z/len, 0.0f, dirXZ.x/len };

    int visible = 0;
    int rays = (numRays < 1) ? 1 : numRays;
    float half = 0.5f * (float)(rays - 1);

    Vector3 from = lightPos; from.y += originYOffset;

    for (int i = 0; i < rays; ++i) {
        float t = ((float)i - half);                 // … -2,-1,0,1,2 …
        Vector3 to = tileCenter;
        to.x += perp.x * t * spreadMeters;
        to.z += perp.z * t * spreadMeters;
        to.y  = tileCenter.y + targetYOffset;

        if (HasWorldLineOfSight(from, to, epsilonFrac)) ++visible;
    }
    return (float)visible / (float)rays;
}


// Generic: stamp into ANY buffer (e.g., gStaticBase or gDynamic.pixels)
// Requires: TileVisibilityWorldRay(...) and SmoothFalloff(...) already defined.
static void StampLightWithWorldLOS_TileFirst_ToBuffer(std::vector<Color>& outBuf, int bufW, int bufH,
                                                      const Vector3& lightPos, float radius, Color color,
                                                      int tilesX, int tilesZ, float tileSize, float floorY)
{
    if ((int)outBuf.size() != bufW*bufH) {
        // safety: resize/clear if caller forgot to size it
        outBuf.assign((size_t)bufW*bufH, (Color){0,0,0,255});
    }

    // Texels-per-tile (assumes bufW/H are multiples of tilesX/Z)
    const int tppX = bufW / tilesX;
    const int tppZ = bufH / tilesZ;

    // Light tile and radius in tiles
    const int lx = (int)floorf((lightPos.x - gDynamic.minX) / tileSize);
    const int lz = (int)floorf((lightPos.z - gDynamic.minZ) / tileSize);
    const int R  = (int)ceilf(radius / tileSize);

    const float radius2 = radius * radius;

    // Ray fan params (tweak to taste)
    const int   numRays       = 5;
    const float spreadMeters  = 0.05f * tileSize;
    const float originYOffset = 2.0f;
    const float targetYOffset = 2.0f;
    const float epsilonFrac   = 0.2f;

    const int tx0 = std::max(0,        lx - R);
    const int tx1 = std::min(tilesX-1, lx + R);
    const int tz0 = std::max(0,        lz - R);
    const int tz1 = std::min(tilesZ-1, lz + R);

    for (int tz = tz0; tz <= tz1; ++tz) {
        for (int tx = tx0; tx <= tx1; ++tx) {
            // Tile center in world
            const float cx = gDynamic.minX + (tx + 0.5f) * tileSize;
            const float cz = gDynamic.minZ + (tz + 0.5f) * tileSize;
            const Vector3 tileCenter = { cx, floorY, cz };

            // Quick radial cull (center-based)
            const float dx = cx - lightPos.x;
            const float dz = cz - lightPos.z;
            if (dx*dx + dz*dz > (radius + 0.75f*tileSize)*(radius + 0.75f*tileSize)) continue;

            // Per-tile world LOS (small fan => soft edges)
            const float vis = TileVisibilityWorldRay(lightPos, tileCenter,
                                                     numRays, spreadMeters,
                                                     originYOffset, targetYOffset,
                                                     epsilonFrac);
            if (vis <= 0.0f) continue;

            // Texel range for this tile in OUT buffer
            const int x0 = tx * tppX;
            const int x1 = x0 + tppX - 1;
            const int y0 = tz * tppZ;
            const int y1 = y0 + tppZ - 1;

            // Per-texel falloff (round pools), scaled by vis
            for (int y = y0; y <= y1; ++y) {
                const float texV = (y + 0.5f) / bufH;
                const float wz   = gDynamic.minZ + texV * gDynamic.sizeZ;

                for (int x = x0; x <= x1; ++x) {
                    const float texU = (x + 0.5f) / bufW;
                    const float wx   = gDynamic.minX + texU * gDynamic.sizeX;

                    const float ddx = wx - lightPos.x;
                    const float ddz = wz - lightPos.z;
                    const float d2  = ddx*ddx + ddz*ddz;
                    if (d2 > radius2) continue;

                    const float d = sqrtf(d2);
                    float w = SmoothFalloff(d, radius) * vis;
                    if (w <= 0.0f) continue;

                    Color& p = outBuf[y * bufW + x];
                    int r = p.r + (int)(color.r * w);
                    int g = p.g + (int)(color.g * w);
                    int b = p.b + (int)(color.b * w);
                    p.r = (unsigned char)std::min(r, 255);
                    p.g = (unsigned char)std::min(g, 255);
                    p.b = (unsigned char)std::min(b, 255);
                }
            }
        }
    }
}

static void StampLight_StaticBase_WorldLOS_TileFirst(const Vector3& lightPos, float radius, Color color,
                                                     int tilesX, int tilesZ, float tileSize, float floorY)
{
    // Ensure base is the right size
    const int W = gDynamic.w, H = gDynamic.h;
    if ((int)gStaticBase.size() != W*H) {
        gStaticBase.assign((size_t)W*H, (Color){0,0,0,255});
    }

    StampLightWithWorldLOS_TileFirst_ToBuffer(gStaticBase, W, H,
                                              lightPos, radius, color,
                                              tilesX, tilesZ, tileSize, floorY);
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

void BuildStaticLightmapOnce(const std::vector<LightSource>& dungeonLights,
                             int tilesX, int tilesZ, float tileSize, float floorY)
{
        gStaticBase.assign((size_t)gDynamic.w * gDynamic.h, (Color){0,0,0,255});
        for (const auto& L : dungeonLights) {
            Color c = {
                (unsigned char)Clamp(L.colorTint.x * 255.0f * L.intensity, 0.0f, 255.0f),
                (unsigned char)Clamp(L.colorTint.y * 255.0f * L.intensity, 0.0f, 255.0f),
                (unsigned char)Clamp(L.colorTint.z * 255.0f * L.intensity, 0.0f, 255.0f),
                255
            };
            StampLight_StaticBase_WorldLOS_TileFirst(L.position, L.range, c, dungeonWidth, dungeonHeight, tileSize, floorHeight);
        }

        // Optional headroom so fireballs add nicely:
        for (Color& p : gStaticBase) {
            p.r = (unsigned char)(p.r * 0.65f);
            p.g = (unsigned char)(p.g * 0.65f);
            p.b = (unsigned char)(p.b * 0.65f);
        }
    
}

void BuildDynamicLightmapFromFrameLights(const std::vector<LightSample>& frameLights)
{
    // Start from the static base (cheap: ~65–150KB copy depending on resolution)
    gDynamic.pixels = gStaticBase;

    LightSample ls =  {
        player.position,
        Vector3 {1.0f, 1.0f, 1.0f},   // 0..1
        500,
        1.0f
    };

    Color c = {
        (unsigned char)Clamp(ls.color.x * 255.0f * ls.intensity, 0.0f, 255.0f),
        (unsigned char)Clamp(ls.color.y * 255.0f * ls.intensity, 0.0f, 255.0f),
        (unsigned char)Clamp(ls.color.z * 255.0f * ls.intensity, 0.0f, 255.0f),
        255
    };


    StampDynamicLight(ls.pos, ls.range, c);



    // Stamp dynamic movers (fireballs). You usually don't need LOS for these.
    for (const LightSample& L : frameLights) {
        Color c = {
            (unsigned char)Clamp(L.color.x * 255.0f * L.intensity, 0.0f, 255.0f),
            (unsigned char)Clamp(L.color.y * 255.0f * L.intensity, 0.0f, 255.0f),
            (unsigned char)Clamp(L.color.z * 255.0f * L.intensity, 0.0f, 255.0f),
            255
        };
        StampDynamicLight(L.pos, L.range, c);
        // No collusion for fireballs, too expensive. 
    }

    UpdateTexture(gDynamic.tex, gDynamic.pixels.data());
}




