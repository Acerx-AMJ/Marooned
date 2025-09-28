// ===== Shadows.h =====
#pragma once
#include <vector>
#include "raylib.h"
#include "vegetation.h"
#include "raymath.h"
#include "resourceManager.h"
#include "world.h"

// Simple holder
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

// Draw soft circles for each tree
inline void BuildTreeShadowMask(TreeShadowMask& mask,
                                const std::vector<TreeInstance>& trees,
                                float baseRadiusMeters = 4.0f,
                                float darknessCenter   = 0.25f,
                                int   rings            = 16)
{
    BeginTextureMode(mask.rt);
    ClearBackground(WHITE);
    BeginBlendMode(BLEND_ALPHA);

    // optional: test marker while tuning
    // DrawCircle(mask.width/2, mask.height/2, mask.height*0.25f, BLACK);

    const float metersPerTexel = mask.worldXZBounds.width / (float)mask.width;
    const float minPxRadius    = 5.0f; // keep or lower once you’re happy

    for (const auto& t : trees) {
        // *** Use the same world position you use when DRAWING the tree ***
        Vector3 wp = t.position;
        wp.x += t.xOffset;
        wp.z += t.zOffset;
        // (y doesn’t matter for the mask)

        float radiusMeters = baseRadiusMeters * (t.scale / 25.0f);
        float pxRadius     = radiusMeters / metersPerTexel;
        if (pxRadius < minPxRadius) pxRadius = minPxRadius;

        Vector2 uv = WorldXZToUV(mask.worldXZBounds, wp);
        int cx = (int)(uv.x * mask.width);
        int cy = (int)(uv.y * mask.height);

        for (int r = rings; r >= 1; --r) {
            float tstep = (float)r / (float)rings;
            float rr    = pxRadius * tstep;

            float gray  = darknessCenter + (1.0f - darknessCenter) * (tstep*tstep);
            unsigned char g = (unsigned char)Clamp(gray * 255.0f, 0.0f, 255.0f);
            DrawCircle(cx, cy, rr, { g, g, g, 255 });
        }
    }

    EndBlendMode();
    EndTextureMode();
}

