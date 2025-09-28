// ===== Shadows.h =====
#pragma once
#include <vector>
#include "raylib.h"
#include "vegetation.h"
#include "raymath.h"
#include "resourceManager.h"
#include "world.h"

//Holder for the rt
struct TreeShadowMask {
    RenderTexture2D rt = {0};
    int width = 0, height = 0;
    Rectangle worldXZBounds = {}; // {x,y,w,h} => (minX, minZ, sizeX, sizeZ)
};

inline Vector2 WorldXZToUV(const Rectangle& worldXZBounds, const Vector3& p) {
    float u = (p.x - worldXZBounds.x) / worldXZBounds.width;
    float v = (p.z - worldXZBounds.y) / worldXZBounds.height;
    return { u, v };
}

// Create or resize the mask RT
inline void InitOrResizeTreeShadowMask(TreeShadowMask& mask, int w, int h, const Rectangle& worldXZBounds) {
    if (mask.rt.id == 0 || mask.width != w || mask.height != h) {
        if (mask.rt.id != 0) UnloadRenderTexture(mask.rt);
        mask.rt    = LoadRenderTexture(w, h);
        mask.width = w;
        mask.height= h;
        mask.worldXZBounds = worldXZBounds;
    }
}

static inline void StampCanopy(Texture2D tex,
                               int cx, int cy,       // center in mask pixels
                               float diameterPx,     // dest diameter in pixels
                               float rotationDeg,
                               float strength01)
{
    Rectangle src = { 0, 0, (float)tex.width, (float)tex.height };

    // Pattern A: dest.x/y = CENTER, origin = half size
    Rectangle dst   = { (float)cx, (float)cy, diameterPx, diameterPx };
    Vector2   origin= { diameterPx*0.5f, diameterPx*0.5f };

    unsigned char a = (unsigned char)Clamp(strength01 * 255.0f, 0.0f, 255.0f);
    Color tint = { 0, 0, 0, a };

    DrawTexturePro(tex, src, dst, origin, rotationDeg, tint);
}

inline void BuildTreeShadowMask_Tex(TreeShadowMask& mask,
                                    const std::vector<TreeInstance>& trees,
                                    Texture2D canopyTex,
                                    float baseDiameterMeters = 88.0f, // tweak to match canopy
                                    float strength01         = 0.55f,// overall darkness
                                    float minDiameterPx      = 88.0f)
{
    BeginTextureMode(mask.rt);
    ClearBackground(WHITE);
    BeginBlendMode(BLEND_ALPHA);

    const float metersPerTexel = mask.worldXZBounds.width / (float)mask.width;

    for (const auto& t : trees) {
        // Use the EXACT draw position (incl. your jitter offsets)
        Vector3 wp = t.position;
        wp.x += t.xOffset;
        wp.z += t.zOffset;

        // Scale canopy size by your tree scale if useful
        float diameterM  = baseDiameterMeters * (t.scale / 25.0f);
        float diameterPx = diameterM / metersPerTexel;
        if (diameterPx < minDiameterPx) diameterPx = minDiameterPx;

        // World → mask pixels
        Vector2 uv = WorldXZToUV(mask.worldXZBounds, wp);
        int cx = (int)(uv.x * mask.width);
        int cy = (int)(uv.y * mask.height);

        // Stamp with rotation; flip sign if it appears reversed
        StampCanopy(canopyTex, cx, cy, diameterPx, t.rotationY, strength01);
        // If rotation looks mirrored, use -t.rotationY
    }

    EndBlendMode();
    EndTextureMode();
}

